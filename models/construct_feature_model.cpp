#include "construct_feature_model.h"
#include "piecewise_constant_model.h"
#include "model_base.h"
#include "sparse_vector.h"
#include <fstream>
#include <algorithm> 
#include <unordered_map>
#include <utility>
#include <math.h>
#include <iostream>
using namespace std;
const static int LABEL_INDEX = 0;
void ConstructFeatureModel::initParams(){

  // hawkes parameters
  num_kernel   = _config["hawkes"]["num_kernel"].as<int>();
  history_size = _config["hawkes"]["history_size"].as<int>();
  num_feature  = _config["hawkes"]["history_size"].as<int>();
  
  // kernel functions
  int _exp = 0;                                                                    
  for(int i = 0 ; i < num_kernel ; i+= NUM_KERNEL_TYPE){                           
    kernels.push_back(make_pair(rbf, pow(2, _exp - 1)));
    kernels.push_back(make_pair(rbf_24h, pow(2, _exp - 1)));                       
    kernels.push_back(make_pair(rbf_7d, pow(2, _exp - 1)));                        
    _exp++;                                                                        
  } 
  num_kernel = (int)kernels.size();
}

ConstructFeatureModel::ConstructFeatureModel() {
  // default parameters
  num_kernel = 0;
  kernels = vector<pair<Kernels, double>>();                                       

  //initialize the model
  initParams();
}


void ConstructFeatureModel::insertEntry(DatasetContainer & dataset, 
    long uid, int session_index, int fea_ind, double fea_val){
  assert(dataset.find(uid) != dataset.end());
  dataset[uid][session_index].insert(fea_ind, fea_val);
    return ;
} 

void ConstructFeatureModel::insertEntry(DatasetContainer & dataset, 
    long uid, int session_index, SparseVector &vec){
  assert(dataset.find(uid) != dataset.end());
  dataset[uid][session_index] = vec;
  return ;
} 

void ConstructFeatureModel::buildIntegralHawkesFeature(DatasetContainer & dataset){
  //
  const UserContainer *data = _train_data;
  int n_user = 0;
  vector<long> all_uids; // for openMP use
  for(auto iter = data->begin(); iter != data->end(); ++iter){
    long id = iter->first;
    all_uids.push_back(id);
    dataset[id] = vector<SparseVector>(iter->second.get_sessions().size());
  }
  random_shuffle ( all_uids.begin(), all_uids.end());

#pragma omp parallel for
  for(int i = 0 ; i < (int)all_uids.size(); i++){
    n_user ++;
    if(n_user % 10000 == 0){
      cerr << n_user <<endl;
    }
    long id = all_uids[i];
    double prev_time = 0;
    int session_index = 0;
    for (auto j = data->at(id).get_sessions().begin();                         
        j!= data->at(id).get_sessions().end(); ++j){
      if (session_index == 0){                                                       
        prev_time = j->end.hours();                                        
      } else{ 
        int target_bin = min(NUM_BIN - 1, (int)((j->start.hours() - prev_time)/(double)BIN_WIDTH));
        double integral_value = 0.0;
        SparseVector vec;
        for(int b = 0 ; b < target_bin ; b++){
          SparseVector _vec = getHawkesFeatureAtTime(id, session_index, prev_time + BIN_WIDTH * (b + 1));
          vec += _vec;
        }
        insertEntry(dataset, id, session_index, vec);
        prev_time = j->end.hours();
      }
      session_index ++;
    }
  }
}

void ConstructFeatureModel::buildHawkesFeature(DatasetContainer & dataset){
  //
  const UserContainer *data = _train_data;
  vector<long> all_uids; // for openMP use
  for(auto iter = data->begin(); iter != data->end(); ++iter){
    long id = iter->first;
    all_uids.push_back(id);
    dataset[id] = vector<SparseVector>(iter->second.get_sessions().size() );
  }

  random_shuffle(all_uids.begin(), all_uids.end()); // for load-balancing
#pragma omp parallel for
  for(int i = 0 ; i < (int)all_uids.size(); i++){
    double prev_time = 0;
    int session_index = 0;
    long id = all_uids[i];
    for (auto j = data->at(id).get_sessions().begin();                         
        j!= data->at(id).get_sessions().end(); ++j){                                                  
      if (session_index == 0){                                                       
        prev_time = j->end.hours();                                        
      } else{ 
        SparseVector vec = getHawkesFeatureAtTime(id, session_index, j->start.hours());
        insertEntry(dataset, id, session_index, vec);
      }
      session_index ++;
    }
  } 
}


void ConstructFeatureModel::buildLabel(DatasetContainer & Y){
  double prev_time = 0.0;
  const UserContainer *data = _train_data;
  for(auto iter = data->begin(); iter != data->end(); ++iter){
    int session_index = 0;
    long id = iter->first;
    Y[id] = vector<SparseVector>(iter->second.get_sessions().size());
    for (auto j = iter->second.get_sessions().begin();                         
        j!= iter->second.get_sessions().end(); ++j){                                                  
      if (session_index == 0){                                                       
        prev_time = j->end.hours();                                        
      } else{ 
        assert(j->start.hours() >= prev_time);
        insertEntry(Y, id, session_index, LABEL_INDEX, j->start.hours() - prev_time);
        prev_time = j->end.hours();
      }
      session_index ++;
    }
  } 
}

vector<DataPoint> ConstructFeatureModel::buildVectorRepresentation(DatasetContainer &X,
    DatasetContainer &integral_X, DatasetContainer &Y){
  vector<DataPoint> dataset;
  const UserContainer * data = _train_data;
  for(auto iter : X){
    long uid = iter.first;
    const vector<Session> & sessions = data->at(uid).get_sessions();
    assert(Y.find(uid) != Y.end()); // any data in X should be also in Y...
    for(int session_index = 0; session_index < iter.second.size(); session_index++){
      if(session_index == 0)continue; // the first session is NOT used as training data...
      assert(session_index < (int)sessions.size());
      DataPoint _example;
      _example.x = X[uid][session_index];
      _example.integral_x = integral_X[uid][session_index];
      _example.uid = uid;
      _example.s_id = session_index;
      _example.start = sessions[session_index].start.seconds();
      _example.end = sessions[session_index].end.seconds();
      _example.prev_end = sessions[session_index - 1].end.seconds();
      _example.y = Y[uid][session_index].getVal(LABEL_INDEX); // index 0 is where y resides...
      _example.bin = min(NUM_BIN - 1, (int)(_example.y / (double)BIN_WIDTH));
      dataset.push_back(_example);
    }
  }
  return dataset;  
}

SparseVector ConstructFeatureModel::getHawkesFeatureAtTime(long uid, int s_id, double _time){

  const UserContainer *data = _train_data;
  assert(data->find(uid) != data->end()); // uid not in training data !?
  const vector<Session> &train_sessions = data->at(uid).get_sessions();
  SparseVector vec;
  for(int k = 0 ; k < num_kernel ; k++){
    int count_history = 0;
    double kernelValue = 0.0;
    for (int j = max(0, s_id - history_size); j < s_id ; j++){
      count_history ++;
      assert(train_sessions[j].start.hours() < _time);
      kernelValue += evalKernel(kernels[k].first, kernels[k].second,
          train_sessions[j].start.hours(), _time);
    }
    vec.insert(k, kernelValue/(double)count_history);
  }
  return vec;
}

int ConstructFeatureModel::train(const UserContainer *data){
  this->buildDataset();
  return 0;
}   


void ConstructFeatureModel::buildDataset(){
  assert(_train_data != nullptr);
  //it's more convenient to compute hawkes features
  DatasetContainer X;
  DatasetContainer integral_X;
  DatasetContainer Y;
  cout <<"==========building the hawkes feature======="<<endl;
  buildHawkesFeature(X);
  cout <<"==========building integral features========"<<endl;
  buildIntegralHawkesFeature(integral_X);
  cout <<"==========building labels==================="<<endl;
  buildLabel(Y);
  cout <<"==========covert to vector representation==="<<endl;
  all_data = buildVectorRepresentation(X,integral_X, Y);
  cout <<"number of sessions = "<<all_data.size()<<endl;
  writeToFile(_config["construct_feature"]["output_path"].as<string>());
}

void ConstructFeatureModel::writeToFile(string path){
  // sort the vector<DataPoint> in chronological order
  cerr <<"sort the data in chronological order"<<endl;
  sort(all_data.begin(), all_data.end());
  cerr <<"the data is sorted!!!"<<endl;
  ofstream outfile;
  outfile.open (path);
  for(auto data : all_data){
    outfile << data.uid<<"\t"<<data.start<<"\t"<<data.end<<"\t"
      <<data.prev_end<<"\t"<<data.x.nnz();
    for(auto iter = data.x.begin(); iter != data.x.end(); ++iter){
      outfile<<"\t"<<iter->first <<"\t" << iter->second;
    }
    outfile << "\t" << data.integral_x.nnz();
    for(auto iter = data.integral_x.begin(); 
        iter != data.integral_x.end(); ++iter){
      outfile<<"\t"<<iter->first <<"\t" << iter->second;
    }
    outfile << endl;
  }
  outfile.close();
}


ModelBase::PredictRes ConstructFeatureModel::predict(const User &user){
  return PredictRes(0,0,false);
}

const char * ConstructFeatureModel::modelName(){
  return "construct_feature_model";
}
