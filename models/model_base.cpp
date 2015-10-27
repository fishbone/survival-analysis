#include "model_base.h"
#include <random>
#include "model_test.h"
#include "eval_loglik.h"
#include "global_constant_model.h"
#include "adhoc_statistics_model.h"
#include <vector>
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
    sort(iter->second.begin(), iter->second.end());
    int plot = 0;
    for(int i = 0 ; i < iter->second.size() ; i++){
      // find a instance whose return time < 48 (just for good plot)
      double y = iter->second[i].y;
      if(y < 49){
        fout <<y<<" "<<iter->second[i].prev_end;
        for(double t = 0.1 ; t <= 49 ; t += 0.05){
          fout << " "<< t <<" "<<predictRateValue(iter->second[i],t);
        }
        fout << endl;
        plot++;
      }
      if(plot > 30)break;
    }
  } 

}

void ModelBase::printExpectedReturnUser(std::string fname){
  cerr <<"================ printExpectedReturnUser !!!" << endl;
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
    if(data.isCensored == true){
      data.end = data.start;
      continue;
    }
    uidData[data.uid].push_back(data);
  }
  for(auto data : test_data){
    if(data.isCensored == true){
      data.end = data.start;
      continue;
    }
    uidData[data.uid].push_back(data);
  }

  for(auto iter = uidData.begin(); iter != uidData.end(); ++iter){
    sort(iter->second.begin(), iter->second.end());
  }
  ptime _unix_start(boost::gregorian::date(1970,1,1)); 
  vector<int> seg_ind;
  vector<pair<double, double>> expected_true;

#pragma omp parallel for
  for(int i = 0 ; i < segments.size() ; i++ ){
    pair<string, string> p = segments[i];
    ptime p1(time_from_string(p.first));
    ptime p2(time_from_string(p.second));
    double start_hours = (p1 - _unix_start).total_seconds()/3600.00;
    double end_hours = (p2 - _unix_start).total_seconds()/3600.00;
    double expected = 0;
    double cond_expected = 0;
    int truth = 0;
    int notReturn =0;
    for(auto iter = uidData.begin(); iter != uidData.end(); ++iter){
      int found = -1;
      for(int j = 0 ; j < iter->second.size() ; j++){
        if(iter->second[j].start > start_hours 
            && iter->second[j].start < end_hours){
          if(j > 0 && iter->second[j-1].end < start_hours && iter->second[j-1].end > 0){
            truth ++;
          }
          break;
        }
      }
      for(int j = 0 ; j < iter->second.size() ; j++){
        if(iter->second[j].end > start_hours){
          break;
        }
        found = j;
      }
      if(found != -1){
        double prev_end = iter->second[found].end;
        //DataPoint data(iter->second[found+1]);
        DataPoint data;
        data.uid = iter->first;
        data.s_id = iter->second[found].s_id + 1;
        data.prev_end = prev_end;
        double GofEnd = predictGofT(data, end_hours - data.prev_end); 
        double GofStart = predictGofT(data, start_hours - data.prev_end); 
        //expected += (GofStart - GofEnd)/(1 - GofStart);
        expected += (GofStart - GofEnd);
        cond_expected += (GofStart - GofEnd)/(GofStart+1e-8); // 1e-8 just to avoid numerical issue
//        cerr << "GStart - GofEnd = " << (GofStart - GofEnd) <<"ration = " << (GofStart - GofEnd)/(GofStart) <<endl;
      }
    }
#pragma omp critical
    {
     cerr <<p.first<<" - "<<p.second<<endl;
     cerr <<"expected active users = " <<cond_expected <<" true active users = " << truth<<endl; 
     seg_ind.push_back(i);
     expected_true.push_back(make_pair(cond_expected, truth));
    }
  }
  for(int i = 0 ; i < seg_ind.size() ; i++){
    cerr << segments[seg_ind[i]].first<<"-"<<segments[seg_ind[i]].second<<" ";
    cerr << expected_true[i].first<<" "<<expected_true[i].second<<" "<<endl;
  }

}
void ModelBase::printStratifiedExpectedReturn(std::string fname){
  cerr <<"printStratifiedExpectedReturn..."<<endl;
  ofstream fout(fname);
  vector<double> segments;                                           
  assert(train_data.size() > 0);
  assert(test_data.size() > 0);
  int n_segments =  _config["segments"]["n_segments"].as<int>();                   
  for(int i = 1 ; i <= n_segments ; i++){                                           
    double s = _config["segments"]["s" + to_string(i)].as<double>();       
    segments.push_back(s);
  }

  double correct = 0.0;
  double total =0.0;
  double all_zero =0.0;
  default_random_engine generator;
#pragma omp parallel for reduction(+:correct) reduction(+:total)
  for(int i = 0 ; i < test_data.size() ; i++){
    DataPoint & _data = test_data[i];
    vector<double> prob;
    double G= 0.0;
    double sum_p = 0.0;
    total ++;
    for(int j = 0 ; j < segments.size() ; j++){
      double s = segments[j];
      G = predictGofT(_data, segments[j]);
      double P = 1 - G;
      prob.push_back(P - sum_p);
      sum_p += P -sum_p;
      //      double e = p.second;
    }
    prob.push_back(1 - sum_p);
    discrete_distribution<int> distribution(prob.begin(), prob.end());
    int sampled = distribution(generator);
    if(i % 50000 == 0 ){
      cerr << "acc now = "<<correct/total<<endl;
    }
    if (sampled == prob.size() - 1){
      if(_data.y > segments.back()){
        correct ++;
      }
    }else if (sampled == 0){
      if(_data.y < segments[0]){
        correct ++;
      }
    } else {
      if(_data.y < segments[sampled] && _data.y > segments[sampled-1]){
        correct ++;
      }
    }
  }
  cerr << "Accuracy of Prediction = " << correct / test_data.size()<<endl;
}
void ModelBase::printStratifiedPerpUser(std::string fname){
  ofstream fout(fname);
  assert(train_data.size() > 0); 
  assert(test_data.size() > 0); 
  unordered_map<long, int> userCount;
  map<long, vector<DataPoint> > userSession;
  vector<pair<int, long>> countUid;
  for(auto data : test_data){
    userCount[data.uid] ++;
    userSession[data.uid].push_back(data);
  }
  for(auto data : train_data){
    userCount[data.uid] ++;
    userSession[data.uid].push_back(data);
  }
  for(auto iter = userCount.begin(); iter != userCount.end(); ++ iter){
    countUid.push_back(make_pair(iter->second, iter->first));
  }
  sort(countUid.begin(), countUid.end());
  vector<DataPoint> V;
  for(int i = 0 ; i < countUid.size() ; i++){
    V.insert(V.end(),userSession[countUid[i].second].begin(), userSession[countUid[i].second].end());
    if( (i+ 1) % 500 == 0){
    //double avgPerp =  evalPerp(userSession[countUid[i].second]);
      double avgPerp = evalPerp(V);
      fout << avgPerp<<endl;  
      V.clear();
    }
//    fout << countUid[i].second <<" "<<countUid[i].first<<" " << avgPerp<<endl; 
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
  int num_session_in_bin = test_data.size()/50.0;
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
    fout << iter->first<<" "<<avgPerp<< endl;
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
