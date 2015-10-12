#include "data_io.h"
#include "model_base.h"
#include "construct_feature_model.h"
#include "adhoc_statistics_model.h"
#include <iostream>
using namespace std;

void AdhocStatisticsModel::startHourOfDayHist(string fname){
  ofstream fout(fname);                                                         
  assert(train_data.size() > 0);                                                
  assert(test_data.size() > 0);                                                 
  map<double, int> startHourOfDayCount;
  for(auto data : train_data){
    fout << fmod(data.start, 24.0) << " ";
  }
  fout << endl;
}

void AdhocStatisticsModel::nextVisitTimeHist(string fname){
  ofstream fout(fname);                                                         
  assert(train_data.size() > 0);                                                
  assert(test_data.size() > 0);                                                 
  map<double, int> nextVisitTimeCount;
  map<long, vector<double>> userNextVisit;
  map<long, int> userNextVisitCount;
  for(auto data : train_data){
    fout << data.y<<" ";
  }
    fout <<endl;
}
void AdhocStatisticsModel::sessionLengthHist(string fname){
  ofstream fout(fname);                                                         
  assert(train_data.size() > 0);                                                
  assert(test_data.size() > 0);                                                 
  map<double, int> lengthCount;
  for(auto data : train_data){
    fout << data.end - data.start << " ";
  }
  fout << endl;
}

void AdhocStatisticsModel::userSessionHist(string fname){
  ofstream fout(fname);                                                         
  assert(train_data.size() > 0);                                                
  assert(test_data.size() > 0);                                                 
  unordered_map<long, int> userCount;
  map<int, int> hist_session_user;
  // count the # sessions for each user in testing data                           
  for(auto data : train_data){                                                   
    userCount[data.uid] ++;                                                     
  }                    
  for(auto iter = userCount.begin(); iter != userCount.end(); ++ iter){         
    fout << iter->second <<" ";
  }
  fout << endl;
}
int AdhocStatisticsModel::train(const UserContainer *data){
  ConstructFeatureModel ctrFeature(NO_FEATURE);                                    
  ctrFeature.setData(_train_data, _test_data);                                     
  assert(_train_data != nullptr);                                                  
  assert(_test_data != nullptr);                                                   
  ctrFeature.train(_train_data);                                                   
  train_data = ctrFeature.getTrainSet();                                           
  test_data = ctrFeature.getTestSet(); 
  string user_session_hist = _config["adhoc"]["user_session_hist"].as<string>();
  string next_visit_time_hist = _config["adhoc"]["next_visit_time_hist"].as<string>();
  string session_length_hist = _config["adhoc"]["session_length_hist"].as<string>();
  string start_hour_of_day_hist = _config["adhoc"]["start_hour_of_day_hist"].as<string>();
  cerr <<"nextVisitTime Histogram..." << endl;
  nextVisitTimeHist(next_visit_time_hist);
  cerr <<"session Length Histogram..." << endl;
  sessionLengthHist(session_length_hist);
  cerr <<"user Session Histogram..." << endl;
  userSessionHist(user_session_hist);
  cerr <<"start hour of day..." << endl;
  startHourOfDayHist(start_hour_of_day_hist);
}

const char *AdhocStatisticsModel::modelName(){  
  return "adhoc_statistics_model";
}	

