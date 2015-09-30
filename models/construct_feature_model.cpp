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
    kernel_name.push_back("rbf"+ std::to_string(pow(2, _exp - 1)));
    kernel_name.push_back("rbf_24h"+ std::to_string(pow(2, _exp - 1)));
    kernel_name.push_back("rbf_7d"+ std::to_string(pow(2, _exp - 1)));
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

//get feature in SparsesVector format for give (uid, session_id, _time)
SparseVector ConstructFeatureModel::getFeatureAtTime(long uid,
    int s_id, double _hours){
  
  vector<Feature> auxFeature = getAuxFeatureAtTime(uid, s_id, _hours);
  vector<Feature> hawkesFeature = getHawkesFeatureAtTime(uid, s_id, _hours);
  vector<Feature> jointFeature(auxFeature);
  
  //concatenate haekesFeature and auxFeature 
  //then return the SparseVector representation
  jointFeature.insert(jointFeature.end(),
      hawkesFeature.begin(), hawkesFeature.end());
  return SparseVector(jointFeature);
}
//get features other than Hawkes for given (uid, session_id, _time);
vector<Feature> ConstructFeatureModel::getAuxFeatureAtTime(long uid,
    int s_id, double _time){
  const UserContainer *data = _concat_data;
  const vector<Session> & sessions = data->at(uid).get_sessions();
  assert(s_id < (int)sessions.size());
  // copy session_feature and append day_feature
  vector<Feature> auxFeature(sessions[s_id].session_features);
  auxFeature.insert(auxFeature.end(), 
      sessions[s_id].day_features->begin(), sessions[s_id].day_features->end());
  return auxFeature;
}

//get features other than Hawkes for given (uid, session_id, _time);
vector<Feature> ConstructFeatureModel::getHawkesFeatureAtTime(long uid,
    int s_id, double _time){

  const UserContainer *data = _concat_data;
  assert(data->find(uid) != data->end()); // uid not in training data !?
  const vector<Session> &train_sessions = data->at(uid).get_sessions();
  vector<Feature> vec;
  for(int k = 0 ; k < num_kernel ; k++){
    int count_history = 0;
    double kernelValue = 0.0;
    for (int j = max(0, s_id - history_size); j < s_id ; j++){
      count_history ++;
      assert(train_sessions[j].start.hours() < _time);
      kernelValue += evalKernel(kernels[k].first, kernels[k].second,
          train_sessions[j].start.hours(), _time);
    }
    vec.push_back(Feature(getFeatureOffset(kernel_name[k]),
          kernelValue/(double)count_history));
  }
  return vec;
}

SparseVector ConstructFeatureModel::getIntegralFeatureAtTime(long uid,
    int s_id, double _hours){
  vector<Feature> auxFeature = getIntegralAuxFeatureAtTime(uid, s_id, _hours);
  vector<Feature> hawkesFeature = getIntegralHawkesFeatureAtTime(uid, s_id, _hours);
  vector<Feature> jointFeature(auxFeature);
  jointFeature.insert(jointFeature.end(),
      hawkesFeature.begin(), hawkesFeature.end());

  return SparseVector(jointFeature);
}

vector<Feature> ConstructFeatureModel::getIntegralAuxFeatureAtTime(long uid,
   int s_id, double _hours){
  assert(_hours >= 0);
  
  const UserContainer *data = _concat_data;
  const vector<Session> & sessions = data->at(uid).get_sessions();
  assert(s_id > 0);
  assert(s_id < (int)sessions.size());
  double prev_end = sessions[s_id - 1].end.hours();
  int target_bin = max(NUM_BIN - 1,(int)((_hours - prev_end)/(double)BIN_WIDTH));
  // copy session_feature and append day_feature
  vector<Feature> auxFeature(sessions[s_id].session_features);
  auxFeature.insert(auxFeature.end(), 
      sessions[s_id].day_features->begin(), sessions[s_id].day_features->end());

  //since session_feature and day_feature are the same within the same
  //session, we just scale by a factor of target_bin to get the desired number
  for(int i = 0 ; i < (int)auxFeature.size(); i++){
    auxFeature[i].second *= (target_bin + 1);
  }
  return auxFeature;
}

vector<Feature> ConstructFeatureModel::getIntegralHawkesFeatureAtTime(long uid,
    int s_id, double _hours){
  assert(_hours >= 0);
  const UserContainer *data = _concat_data;
  const vector<Session> & sessions = data->at(uid).get_sessions();
  assert(s_id > 0);
  assert(s_id < (int)sessions.size());
  
  double prev_end = sessions[s_id - 1].end.hours();
  assert(_hours >= prev_end);
  int target_bin = max(NUM_BIN - 1, (int)((_hours - prev_end)/(double)BIN_WIDTH));
  // get the feature at bin 0 then for b = 1 : target_bin
  // add them add
  vector<Feature> hawkesFeature = getHawkesFeatureAtTime(uid, 
      s_id, prev_end + BIN_WIDTH);

  for(int b = 1 ; b <= target_bin; b++){
    vector<Feature> tmpHawkes = getHawkesFeatureAtTime(uid, 
        s_id, prev_end + (b + 1) * BIN_WIDTH);
    for(int i = 0 ; i < (int)tmpHawkes.size(); i++){
      //for sanity check, 
      if(hawkesFeature[i].first != tmpHawkes[i].first){
        cerr<<hawkesFeature[i].first<<" "<<tmpHawkes[i].first<<endl;
      }
      assert(hawkesFeature[i].first == tmpHawkes[i].first);
        hawkesFeature[i].second += tmpHawkes[i].second;
    }
  }
  return hawkesFeature;
}



void ConstructFeatureModel::buildVectorizedDataset(){
 int n_user = 0; 
 //tmpMap is for OpenMP acceleration
 unordered_map<long, vector<DataPoint>> tmpMap;
 vector<long> all_uids;
 for(auto iter = _concat_data->begin(); iter != _concat_data->end(); ++iter){ 
   tmpMap[iter->first] = vector<DataPoint>();
   all_uids.push_back(iter->first);
 }

#pragma omp parallel for
 for(int i = 0 ; i < (int)all_uids.size(); ++i){
   n_user ++;
   if(n_user % 10000 == 0){
     cerr <<"buildVectorizedDataset processed_user = "<<n_user<<endl;
   }
   long uid = all_uids[i];
   User &user = _concat_data->at(uid);
   const vector<Session> &all_sessions = user.get_sessions();
   for(int i = 1 ; i < (int)all_sessions.size(); i++){
     SparseVector x = getFeatureAtTime(uid, i, all_sessions[i].start.hours());
     double prev_end = all_sessions[i-1].end.hours();
     double start = all_sessions[i].start.hours();
     double end = all_sessions[i].end.hours();
     int bin = (start - prev_end)/(double)BIN_WIDTH;
     // ctr :(uid, s_id, y, bin, start, end)
     DataPoint data;
     // info about this data point
     data.uid = uid;
     data.start = start;
     data.end = end;
     data.prev_end = prev_end;
     data.bin = bin;
     data.y = start - prev_end;
     data.s_id = i;
     data.x = getFeatureAtTime(uid, i, start);
     data.integral_x = getIntegralFeatureAtTime(uid, i, start);
     tmpMap[uid].push_back(data);
   }
 }
 for(int i = 0 ; i < (int)all_uids.size(); ++i){
   long uid = all_uids[i];
   for(int j = 0 ; j < (int)tmpMap[uid].size() ; j++){
     if(isTestSet[uid][tmpMap[uid][j].s_id] == true){
       test_data.push_back(tmpMap[uid][j]);
     }else{
       train_data.push_back(tmpMap[uid][j]);
     }
   }
 }
 cerr <<"finished buildingVetorizedDatset ";
 cerr <<"#train session = "<< train_data.size()<<" # test_session = "
      <<test_data.size()<<endl;
}

void ConstructFeatureModel::buildDataset(){
  assert(_train_data != nullptr);
  assert(_test_data != nullptr);
  //concatenate train and test data to make our life easier.
  //as we need to iterate historical sessions for constructing hawkes features
  cerr <<"concatenate train and test data "
       <<"as we need to iterate historical sessions "
       <<"for constructing hawkes features" <<endl;
  _concat_data = new UserContainer(*_train_data); // copy 
  for(auto iter = _test_data->begin(); iter != _test_data->end(); ++iter){
    long uid = iter->first;
    if(_concat_data->find(uid) == _concat_data->end()){
      continue;
    }
    isTestSet[uid] = unordered_map<int, bool>();
    int train_session_size = (int)_concat_data->at(uid).get_sessions().size();
    int offset = 0;
    for(auto &test_session : _test_data->at(uid).get_sessions()){
      _concat_data->at(uid).append_session(test_session);
      isTestSet[uid][offset + train_session_size] = true;
      offset ++;
    }
  }
  cerr <<"buildVectorizedDataset"<<endl;
  buildVectorizedDataset();

  //it's more convenient to compute hawkes features
//  writeToFile(_config["construct_feature"]["output_path"].as<string>());
}

int ConstructFeatureModel::train(const UserContainer *data){
  this->buildDataset();
  cerr <<train_data[0].x<<endl;
  cerr << train_data[0].y<<endl;
  cerr <<train_data[0].integral_x<<endl;
  string A("__SIZE__");
  cout <<getFeatureOffset(A)<<endl;
  return 0;
}   

vector<DataPoint> & ConstructFeatureModel::getTrainSet(){
  return train_data;
}

vector<DataPoint> & ConstructFeatureModel::getTestSet(){
  return test_data;
}

void ConstructFeatureModel::writeToFile(string path){
  // sort the vector<DataPoint> in chronological order
  /*
  cerr <<"sort the data in chronological order"<<endl;
  sort(all_data.begin(), all_data.end());
  cerr <<"the data is sorted!!!"<<endl;
  ofstream outfile;
  outfile.open (path);
  for(auto data : all_data){
    outfile << data.uid<<"\t"<<data.s_id<<"\t"<<data.start<<"\t"<<data.end<<"\t"
      <<data.prev_end<<"\t"<<data.x.nnz();
    for(auto iter = data.x.begin(); iter != data.x.end(); ++iter){
      outfile<<"\t"<<iter->first <<"\t" << iter->second;
    }
    outfile << endl;
  }
  outfile.close();
  */
}


ModelBase::PredictRes ConstructFeatureModel::predict(const User &user){
  return PredictRes(0,0,false);
}

const char * ConstructFeatureModel::modelName(){
  return "construct_feature_model";
}
