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
  }else if(name == "construct_feature_model"){
    return new ConstructFeatureModel();
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
