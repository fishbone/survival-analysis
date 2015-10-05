#include "user_constant_model.h"
#include "piecewise_constant_model.h"
#include "global_piecewise_constant_model.h"
#include "data_io.h"
#include <iostream>
using namespace std;
double GlobalPiecewiseConstantModel::evalTrainPerp(const UserContainer * data){       
  int n_session = 0;
  int n_user = 0;
  double sum_loglik = 0.0;                                                      
  for(auto iter = data->begin();                                                
      iter != data->end(); ++iter){                                             
    int index = 0;                                                                 
    double prev_end = -1;                                                          
    long uid = iter->first;
    if(iter->second.get_sessions().size() > 1){
      n_user ++;
    }    
    for (auto j = iter->second.get_sessions().begin();                             
        j!= iter->second.get_sessions().end();                                     
        ++j){
      if (index == 0){                                                          
        index++;                                                                
        prev_end = j->end.hours();                                              
      } else{                                                                   
        double loglik = 0.0;                                                    
        int target_bin = (j->start.hours() - prev_end)/(double)BIN_WIDTH;       
        assert(target_bin >= 0);                                                
        target_bin = min(target_bin, NUM_BIN - 1);                              
        double lambda = lambda_all[target_bin];
        loglik += log(lambda);                                                     
        for (int j = 0; j < target_bin; j++) {                                  
          double lambda_j = lambda_all[j];                                      
          if (lambda_j != 0.0){                                                    
            double normalized = lambda_j*BIN_WIDTH;                                
            loglik += - normalized;                                             
          }                                                                     
        }                                                                       
        double lambda_target = lambda_all[target_bin];                       
        double normalized =                                                     
          lambda_target * (j->start.hours() -                                   
              (prev_end + target_bin * BIN_WIDTH));                             
        loglik -= normalized;                                                   
        prev_end = j->end.hours();                                              
        n_session ++;                                                           
        sum_loglik += loglik/(double)(iter->second.get_sessions().size() - 1); 
      }
    }                                                                           
  }                                                                             
  //cerr <<" train session = "<<n_session<<" avg perp = "<<exp(-sum_loglik/(double)n_session)<<endl;
  cerr <<" train session = "<<n_session<<" avg perp = "<<exp(-sum_loglik/(double)n_user)<<endl;
}
int GlobalPiecewiseConstantModel::train(const UserContainer *data){
  _user_train = data;
  //some bins will have no data, which will result in very poor estimation
  //here we simply fill them by the global_constant estimate...
  this->lambda_all = vector<double>(NUM_BIN, EPS_LAMBDA);
  vector<int> countInBin(NUM_BIN,0);
  vector<double> timeBeforeBin(NUM_BIN,0.0);
  for(auto iter = data->begin();
      iter != data->end(); ++iter){
    const vector<Session> &sessions = iter->second.get_sessions();
    for(int i = 1 ; i < (int)sessions.size() ; i++){
      int target_bin = sessions[i].binFromLastSession();
      countInBin[target_bin]++;
      for(int b = 0 ; b < target_bin; b++){
        timeBeforeBin[b] += BIN_WIDTH;
      }
      timeBeforeBin[target_bin] += 
        (sessions[i].start.hours() - (target_bin * BIN_WIDTH + sessions[i-1].end.hours()));
    }
  }
  for (int i = 0 ; i < NUM_BIN ; i++){
    if(timeBeforeBin[i] > 0 && countInBin[i] > 0)
      lambda_all[i] = countInBin[i]/(double)timeBeforeBin[i];
  }
   cerr <<"finished training "<< string(modelName()); 
  evalTrainPerp(data);
  return 0;
}
ModelBase::PredictRes GlobalPiecewiseConstantModel::predict(const User &user){
  auto ite = _user_train->find(user.id());
  if(ite == _user_train->end()){
    return PredictRes(-1, 0.0, 0.0, false);
  }else{
    const vector<Session> &train_sessions = ite->second.get_sessions();
    const vector<Session> &test_sessions = user.get_sessions();
    double loglik = 0.0;
    double prev_end = train_sessions.back().end.hours();
    int small =0;
    int total = 0;
    int num_sessions = (int)test_sessions.size();
    for(int i = 0 ; i < num_sessions ; i++){
      int target_bin = (test_sessions[i].start.hours() - prev_end)/(double)BIN_WIDTH;
      if(target_bin < 0){
        cerr << user.id()<<" "<<test_sessions[i].start.hours() <<" "<<prev_end<<" "<<target_bin<<endl;
      }
      assert(target_bin >= 0);
      target_bin = min(target_bin, NUM_BIN - 1);
      double lambda = lambda_all[target_bin];
      loglik += log(lambda);
      for (int j = 0; j < target_bin; j++) {
        double lambda_j = lambda_all[j];
        if (lambda_j != 0.0){
          double normalized = lambda_j*BIN_WIDTH;
          loglik += - normalized;
        }
      }
      double lambda_target = lambda_all[target_bin];
      double normalized = 
        lambda_target * (test_sessions[i].start.hours() - 
            (prev_end + target_bin * BIN_WIDTH));
      loglik -= normalized;
      prev_end = test_sessions[i].end.hours();
    }
    return PredictRes(0,
        loglik,
        num_sessions,
        true);
  } 
}
const char * GlobalPiecewiseConstantModel::modelName(){
  return "global_piecewise_constant_model";    
}
