#include "model_base.h"
#include "model_test.h"
#include "eval_loglik.h"
#include "global_constant_model.h"
#include "construct_feature_model.h"
#include "user_constant_model.h"
#include "piecewise_constant_model.h"
#include "feature_based_model.h"
#include <utility>
#include <string>
#include <fstream>
jsoncons::json ModelBase::_config;
using namespace std;
void ModelBase::printStratifiedExpectedReturn(std::string fname){
  vector<pair<double, double>> segments;                                           
  int n_segments =  _config["segments"]["n_segments"].as<int>();                   
  for(int i = 1 ; i <= n_segments ; i++){                                           
    double s = _config["segments"]["s" + to_string(i)].as<double>();       
    double e = _config["segments"]["e" + to_string(i)].as<double>();       
    segments.push_back(make_pair(s,e));                                            
  }
  ofstream fout(fname);
  assert(train_data.size() > 0); 
  assert(test_data.size() > 0); 
  unordered_map<long, int> userCount;                                              
  vector<pair<int, double> > countPerp;                                            
  map<long, vector<DataPoint> > userSession;                             
  unordered_map<long, int> userToBin;
  vector<pair<int, long>> countUid;
  // count the # sessions for each user in training data                           
  for(auto data : train_data){                                                     
    userCount[data.uid] ++;                                               
  }
  for(auto iter = userCount.begin(); iter != userCount.end(); ++ iter){
    countUid.push_back(make_pair(iter->second, iter->first));
  }
  // sort based on frequency default cmp for pair is by first
  sort(countUid.begin(), countUid.end());
  int n_user = countUid.size();
  for(int i = 0 ; i < (int)countUid.size() ; i++){
    userToBin[countUid[i].second] = 0;
  }
  for(auto data : test_data){
    userSession[userToBin[data.uid]].push_back(data);
  }
  fout <<"segments:";
  for(auto p : segments){ 
    double start = p.first;
    double end = p.second;
    fout << "\t"<<start<<"-"<<end;
  }
  fout << endl;
  for(auto iter = userSession.begin(); iter != userSession.end(); ++ iter){
    fout << iter->first;
    for(auto p : segments){
      double start = p.first;
      double end = p.second;
      int countReturn = 0;
      int countNotReturn = 0;
      double expectedReturn =0.0;
      assert(end > start); // end time should > start time of segment
      for(int i = 0 ; i < iter->second.size() ; i++){
        DataPoint &data =  iter->second[i];
        double t = data.y;
        if(t <= end && t > start){
          countReturn++;
        } else {
          countNotReturn ++;
        }
        double GofStart = predictGofT(data, start);
        double GofEnd = predictGofT(data, end);
        double probReturn = GofStart - GofEnd;
        expectedReturn += probReturn;
      }
      cerr <<start<<"-"<<end
           <<"\tcountNotReturn = " << countNotReturn
           <<"\tcountReturn = "<<countReturn
           <<"\texpected return = "<<expectedReturn<<endl;
      fout << "\t" << (expectedReturn - countReturn)/(double)countReturn;
    }
    fout << endl;
  }
}
void ModelBase::printStratifiedPerp(std::string fname){
  //stratify by frequency of use in training data
  // 20 bins 0~5%, 5~10%,...,95~100% of heavy users
  ofstream fout(fname);
  assert(train_data.size() > 0); 
  assert(test_data.size() > 0); 
  unordered_map<long, int> userCount;                                              
  vector<pair<int, double> > countPerp;                                            
  unordered_map<long, vector<DataPoint> > userSession;                             
  unordered_map<long, int> userToBin;
  vector<pair<int, long>> countUid;
  // count the # sessions for each user in training data                           
  for(auto data : train_data){                                                     
    userCount[data.uid] ++;                                               
  }
  for(auto iter = userCount.begin(); iter != userCount.end(); ++ iter){
    countUid.push_back(make_pair(iter->second, iter->first));
  }
  // sort based on frequency default cmp for pair is by first
  sort(countUid.begin(), countUid.end());
  int n_user = countUid.size();
  for(int i = 0 ; i < (int)countUid.size() ; i++){
    userToBin[countUid[i].second] = i/(n_user / 20.0);
  }
  for(auto data : test_data){
    userSession[userToBin[data.uid]].push_back(data);
  }
  for(auto iter = userSession.begin(); iter != userSession.end(); ++ iter){        
    double avgPerp =  evalPerp(iter->second);
    fout << iter->first <<"\t" << avgPerp<<endl;
  }                                                                                
}

ModelBase *ModelBase::makeModel(const char *model_name){
  std::string name = model_name;
  if(name == "model_test"){
    return new ModelTest();
  }else if(name == "global_constant_model"){
    return new GlobalConstantModel();
  }else if(name == "user_constant_model"){
    return new UserConstantModel();
  }else if(name == "piecewise_constant_model"){
    return new PiecewiseConstantModel();
  }else if(name == "feature_based_model"){
    return new FeatureBasedModel();
  }else {
    cerr <<"Model with name = "<<name  <<" not makable.. abort !"<<endl;
    exit(1);
  }
  return nullptr;
}

EvaluationBase *EvaluationBase::makeEval(const char *eval_name){
  std::string name = eval_name;
  if(name == "eval_loglik"){
    return new EvalLoglik();
  }
  return nullptr;
}
