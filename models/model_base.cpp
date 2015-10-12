#include "model_base.h"
#include "model_test.h"
#include "eval_loglik.h"
#include "global_constant_model.h"
#include "adhoc_statistics_model.h"
#include "construct_feature_model.h"
#include "user_constant_model.h"
#include "piecewise_constant_model.h"
#include "feature_based_model.h"
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/date_time/posix_time/posix_time.hpp> 
#include <utility>
#include <string>
#include <fstream>
using namespace boost;
using namespace boost::posix_time;
using namespace std;
jsoncons::json ModelBase::_config;
void ModelBase::printExpectedReturnUser(string fname){
  cerr <<"printExpectedReturnUser..."<<endl;

  ofstream fout(fname);

}
void ModelBase::printRandomSampledRateFunction(string fname){
  cerr <<"printRandomSampledRateFunction..."<<endl;
  ofstream fout(fname);
  assert(train_data.size() > 0); 
  assert(test_data.size() > 0); 
  unordered_map<long, int> userCount;                                              
  vector<pair<int, double> > countPerp;                                            
  map<long, vector<DataPoint> > userSession;
  map<int, int> hist_session_user;
  unordered_map<long, int> userToBin;
  vector<pair<int, long>> countUid;
  // count the # sessions for each user in testing data                           
  for(auto data : test_data){
    userCount[data.uid] ++;                                               
  }
  for(auto iter = userCount.begin(); iter != userCount.end(); ++ iter){
    hist_session_user[iter->second]++;
    countUid.push_back(make_pair(iter->second, iter->first));
  }
  // sort based on frequency default cmp for pair is by first
  sort(countUid.begin(), countUid.end());
  int num_session_in_bin = test_data.size()/20.0;
  int remain = num_session_in_bin;
  int n_user = countUid.size();
  int bin = 0;
  for(int i = 0 ; i < (int)countUid.size() ; i++){
    userToBin[countUid[i].second] = bin;
    remain -= countUid[i].first;
    if(remain < 0){
      remain = num_session_in_bin;
      bin++;
    }
  }

  for(auto data : test_data){
    userSession[userToBin[data.uid]].push_back(data);
  }

  for(auto iter = userSession.begin(); iter != userSession.end(); ++ iter){
    int plot = 0;
    for(int i = 0 ; i < iter->second.size() ; i++){
      // find a instance whose return time < 24 (just for good plot)
      double y = iter->second[i].y;
      if(y < 24){

        fout <<y;

        for(double t = 0.1 ; t <= 25 ; t += 0.05){
          fout << " "<< t <<" "<<predictRateValue(iter->second[i],t);
        }
        fout << endl;
        plot++;
      }
      if(plot > 20)break;
    }
  } 


}
void ModelBase::printStratifiedExpectedReturn(std::string fname){
  cerr <<"printStratifiedExpectedReturn..."<<endl;
  ofstream fout(fname);
  vector<pair<string, string>> segments;                                           
  assert(train_data.size() > 0);
  assert(test_data.size() > 0);
  int n_segments =  _config["segments"]["n_segments"].as<int>();                   
  for(int i = 1 ; i <= n_segments ; i++){                                           
    string s = _config["segments"]["s" + to_string(i)].as<string>();       
    string e = _config["segments"]["e" + to_string(i)].as<string>();       
    segments.push_back(make_pair(s,e));
  }
  unordered_map<long, vector<DataPoint>> uidData;
  for(auto data : train_data){
    uidData[data.uid].push_back(data);
  }
  for(auto data : test_data){
    uidData[data.uid].push_back(data);
  }
  for(auto iter = uidData.begin(); iter != uidData.end(); ++iter){
    sort(iter->second.begin(), iter->second.end());
  }
  ptime _unix_start(boost::gregorian::date(1970,1,1)); 
  for(auto p : segments){
    ptime p1(time_from_string(p.first));
    ptime p2(time_from_string(p.second));
    double start_hours = (p1 - _unix_start).total_seconds()/3600.00;
    double end_hours = (p2 - _unix_start).total_seconds()/3600.00;
    double expected = 0;
    int truth = 0;
    int notReturn =0;
    for(auto iter = uidData.begin(); iter != uidData.end(); ++iter){
      int found = -1;
      for(int j = 0 ; j < iter->second.size() ; j++){
        if(iter->second[j].start > start_hours){
          if(iter->second[j].start < end_hours){
            truth ++;
          }else {
            notReturn ++;
          }
          break;
        }
        found = j;
      }
      if(found == iter->second.size() - 1){
        notReturn ++;
        continue;
      }
      if(found != -1){
        double prev_end = iter->second[found].end;
        DataPoint data;
        data.uid = iter->first;
        data.s_id = found + 1;
        data.prev_end = prev_end;
        double GofEnd = predictGofT(data, end_hours - prev_end); 
        double GofStart = predictGofT(data, start_hours - prev_end); 
        expected += (1 - GofEnd) - (1 - GofStart);
        bool yes = iter->second[found+1].start < end_hours;
         
        cerr <<start_hours<< " " <<end_hours<<" "<< iter->second[found].start<<" "<<iter->second[found+1].start<< " "<<start_hours - prev_end<<" true = "<<yes<<" prob = "<<GofStart- GofEnd<<endl;
      }
    }
    cerr <<p.first<<" - "<<p.second<<endl;
    cerr <<"expected active users = " <<expected <<" true active users = " << truth<<endl; 
  }
}
void ModelBase::printStratifiedPerp(std::string fname){
  cerr <<"printStratifiedPerp..."<<endl;
  //stratify by frequency of use in training data
  // 20 bins 0~5%, 5~10%,...,95~100% of heavy users
  ofstream fout(fname);
  assert(train_data.size() > 0); 
  assert(test_data.size() > 0); 
  unordered_map<long, int> userCount;                                              
  vector<pair<int, double> > countPerp;                                            
  map<long, vector<DataPoint> > userSession;
  map<int, int> hist_session_user;
  unordered_map<long, int> userToBin;
  vector<pair<int, long>> countUid;
  // count the # sessions for each user in testing data                           
  for(auto data : test_data){
    userCount[data.uid] ++;                                               
  }
  for(auto iter = userCount.begin(); iter != userCount.end(); ++ iter){
    hist_session_user[iter->second]++;
    countUid.push_back(make_pair(iter->second, iter->first));
  }
  // sort based on frequency default cmp for pair is by first
  sort(countUid.begin(), countUid.end());
  int num_session_in_bin = test_data.size()/20.0;
  int remain = num_session_in_bin;
  int n_user = countUid.size();
  int bin = 0;
  for(int i = 0 ; i < (int)countUid.size() ; i++){
    userToBin[countUid[i].second] = bin;
    remain -= countUid[i].first;
    if(remain < 0){
      remain = num_session_in_bin;
      bin++;
    }
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
  }else if (name == "adhoc_statistics_model"){
    return new AdhocStatisticsModel();
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
