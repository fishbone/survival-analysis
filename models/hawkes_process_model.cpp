#include "hawkes_process_model.h"
#include "user_constant_model.h"
#include "piecewise_constant_model.h"
#include "data_io.h"
#include <stdio.h>
#include <iostream>
#include <random>
#include <math.h> 
using namespace std;

void HawkesProcessModel::updateParameter(const long user_id, double scale){
  for(int k = 0 ; k < num_kernel; k++){
    g_alpha[k] = this->momentum * g_alpha[k] + scale * lr_alpha * d_alpha[k];
    alpha[k] -= g_alpha[k];
    alpha[k] = max(alpha[k], 0.0);
  }
  for(int b = 0 ; b < NUM_BIN ; b++){
    g_lambda_base[user_id][b] = this->momentum * g_lambda_base[user_id][b] +
      scale * lr_lambda * d_lambda_base[user_id][b];
    lambda_base[user_id][b] -= g_lambda_base[user_id][b];
    lambda_base[user_id][b] = max(lambda_base[user_id][b], 0.0);
  }
}
double HawkesProcessModel::evaluateLoglik(){
  const UserContainer * data  = this->_test_data;
  double loglik =0.0;
  int n_user = 0;
  for(auto iter = data->begin();
      iter != data->end(); ++iter){
    PredictRes res = this->predict(iter->second);
    if(res.valid == true){
      loglik += res.loglikelihood;
      n_user ++;
    }
  }
  return loglik/n_user;

}
double HawkesProcessModel::evalKernel(Kernels &kernel, double sigma, double t1, double t2){
  double tmp = 0.0;
  switch (kernel){
    case rbf:
      tmp = abs(t1 - t2);
      return exp(-0.5 / sigma * tmp * tmp);
      break;
    case rbf_24h:
      tmp = fmod(abs(t1 - t2), 24.0); // mod 1d, 24h
      if(tmp >= 12.0){
        tmp = 24.0 - tmp;
      }
      return exp(-0.5 / sigma * tmp * tmp);
      break;
    case rbf_7d:
      tmp = fmod(abs(t1 - t2), 168.0); // mod 168h (7d)
      if(tmp >= 84){
        tmp = 168 - tmp;
      }
      return exp(-0.5 / sigma * tmp * tmp);
      break;
  }
  assert(false); // should not reach here !!!
  return -0.0;
}

HawkesProcessModel::HawkesProcessModel(){

  num_kernel = _config["hawkes"]["num_kernel"].as<int>();;
  num_kernel += NUM_KERNEL_TYPE - num_kernel % NUM_KERNEL_TYPE; // this is to make each kernel type having the same numbers
  history_size = _config["hawkes"]["history_size"].as<int>();
  kernels = vector<pair<Kernels, double>>();
  lr_lambda = _config["hawkes"]["lr_lambda"].as<double>();;
  lr_alpha = _config["hawkes"]["lr_alpha"].as<double>();;
  momentum = _config["hawkes"]["momentum"].as<double>();;
  cout <<"hawkes_process\nlr_alpha = "<<lr_alpha<< " lr_lambda = "<<lr_lambda
    <<" num_kernel = " << num_kernel 
    <<" history_size = "<<history_size
    <<" momentum = "<<momentum<<endl;
  //initialize sigma and alpha 
  int _exp = 0;
  for(int i = 0 ; i < num_kernel ; i+= NUM_KERNEL_TYPE){
    kernels.push_back(make_pair(rbf, pow(2, _exp - 2)));
    kernels.push_back(make_pair(rbf_7d, pow(2, _exp - 2)));
    kernels.push_back(make_pair(rbf_24h, pow(2, _exp - 2)));
    _exp++;
  }

}
int HawkesProcessModel::train(const UserContainer *data){

  // the base rate for each (user,bin) pair is initialized using 
  // user_constant model
  //UserConstantModel base;
  PiecewiseConstantModel base;
  cout <<"======training piecewise_constant to initialize base rate function...."<<endl;
  base.train(data);
  this->lambda_base = base.lambda_u;
  this->d_alpha = vector<double>(num_kernel, 0.0);
  this->g_alpha = vector<double>(num_kernel, 0.0);
  this->alpha = vector<double>(num_kernel, 1/(double)num_kernel);
  for(auto iter = base.lambda_u.begin(); iter != base.lambda_u.end(); ++iter){
    this->lambda_base[iter->first] = vector<double>(NUM_BIN,1/(double)NUM_BIN);
    this->d_lambda_base[iter->first] = vector<double>(NUM_BIN,0.0);
    this->g_lambda_base[iter->first] = vector<double>(NUM_BIN,0.0);
  }
  cout <<"======training hawkes process using stochastic gradient...."<<endl;
  _user_train = data;
  for(int i = 0 ; i < 40; i++){
    int trained_user = 0;
    int n_user = data->size();
    double loglik = 0.0;
    loglik = this->evaluateLoglik(); 
    cout <<"iteration: "<<i + 1 <<" loglik = "<<loglik<<endl;
    // can be done in parallel.......
    for(auto iter = data->begin();
        iter != data->end(); ++iter){
      trained_user ++;
      int n_session = iter->second.get_sessions().size();
      if(trained_user % 10000 == 0){
        cerr <<"trained_user = "<<trained_user <<" out of "<< n_user <<endl;    
      }
      for(int session_index = 0; session_index != (int)iter->second.get_sessions().size(); 
          session_index++){
        getDerivative(iter->second, session_index);
        updateParameter(iter->first, 1.0/n_session);
      }
    }
    for(int k = 0 ; k < num_kernel ; k++){
      cout <<alpha[k]<<" ";
    }
    cout <<endl;
  }

  return 0;
}


void HawkesProcessModel::getDerivative(const User &user, int session_index){
  auto &sessions = user.get_sessions();
  assert(session_index < (int)sessions.size());
  long id = user.id();
  if(session_index == 0){
    // first session, we can't do anything
    return ;
  }else{
    int target_bin = sessions[session_index].binFromLastSession();
//    assert(target_bin >= 0);
    assert(lambda_base.find(id) != lambda_base.end());
    double prev_end = sessions[session_index - 1].end.hours();
    double start = sessions[session_index].start.hours();
    vector<double> & base_u = lambda_base[id];
    vector<double> & d_base_u = d_lambda_base[id];
    // get derivative for all base lambda
    for(int b = 0; b <= target_bin ; b++){
      d_base_u[b] = BIN_WIDTH; 
    }
    // get more derivative for base lambda in target_bin
    double deno = base_u[target_bin];
    for(int h = max(0, session_index -  history_size); h < session_index ; h++){
      for(int k = 0 ; k < num_kernel; k++){
        double kernelValue = 
          evalKernel(kernels[k].first, kernels[k].second, start , sessions[h].start.hours());
        deno += alpha[k] *  kernelValue;
      }
    }
    d_base_u[target_bin] -=  1.0f/deno;
    // get derivative for alpha_k
    for(int k = 0 ; k < num_kernel ; k++){
      double log_numerator =0.0;
      double A = 0.0;
      // d_alpha[k] = A - B:
      // A = \sum_{b} tau * [\sum_{t_i < t_b}exp(-1/(2.0sigma_k)(t_b - t_i)^2)]
      for(int b = 0 ; b <= target_bin ; b++){
        for(int h = max(0, session_index - history_size); h < session_index ;h++){
          double kernelValue = 
            evalKernel(kernels[k].first, kernels[k].second, prev_end + BIN_WIDTH*(b+1) , sessions[h].start.hours());
          A += BIN_WIDTH * kernelValue; 
        }
      }
      //B =  \frac{\sum_{t_i < t}exp(-1/(2*sigma_k)*(t-t_i)^2}
      //{\lambda_u^0 + \sum_{t_i < t}[\sum_{k=1}^K alpha_k exp(-1/(2sigma_k)*(t-t_i)^2]}
      for(int h = max(0, session_index - history_size); h < session_index ;h++){
        double kernelValue = 
          evalKernel(kernels[k].first, kernels[k].second, start , sessions[h].start.hours());
        log_numerator += kernelValue;
      }
      d_alpha[k] = A - log_numerator/deno;
    }
  }
}




ModelBase::PredictRes HawkesProcessModel::predict(const User &user){
  auto ite = _user_train->find(user.id());
  long id = user.id();
  if(ite == _user_train->end() || ite->second.get_sessions().size() == 0){
    return PredictRes(0, 0, false);
  }else{
    const vector<Session> &train_sessions = ite->second.get_sessions();
    const vector<Session> &test_sessions = user.get_sessions();
    vector<Session> history;
    if((int)train_sessions.size() <= history_size){
      history = vector<Session>(train_sessions);    
    }else{
      for(auto i = train_sessions.size() - history_size; i < train_sessions.size(); i++){
        history.push_back(train_sessions[i]);    
      }   
    }
    int H = history.size();
    double loglik = 0.0;
    double prev_end = train_sessions.back().end.hours();
    int num_sessions = (int)test_sessions.size();
    for(int i = 0 ; i < num_sessions ; i++){
      double log_density = 0.0;
      double t = test_sessions[i].start.hours();
      double normalized = 0.0;
      double log_density_in_log = 0.0;
      int target_bin = (test_sessions[i].start.hours() - prev_end)/(BIN_WIDTH);
      //int target_bin = test_sessions[i].binFromLastSession();
      assert(target_bin >= 0);
      if (target_bin >= NUM_BIN){
        target_bin = NUM_BIN - 1;     
      }

      for(int b = 0 ; b <= target_bin; b++){
        double t_b = prev_end + (b+1) * BIN_WIDTH;
        normalized += BIN_WIDTH *  lambda_base[id][b];
        // computing kernels using history sessions
        
        
        for(int h = max(0, H - history_size); h < H; h++){
          for(int k = 0 ; k < num_kernel; k++){
            double kernelValue = 
              evalKernel(kernels[k].first, kernels[k].second, t_b , history[h].start.hours());
            normalized +=  BIN_WIDTH * alpha[k] * kernelValue;
          }
        }
        
      }
      
      for(int h = max(0, H - history_size); h < H; h++){
        for(int k = 0 ; k < num_kernel; k++){
          double kernelValue = 
            evalKernel(kernels[k].first, kernels[k].second, t , history[h].start.hours());
          //cerr <<setprecision(10) << "kernel value = "<<kernelValue<<" "<<t<<" "<<history[h].start.hours()<<endl;
          log_density_in_log +=  alpha[k] * kernelValue;
        }
      }
      
      if(lambda_base[id][target_bin] + log_density_in_log <= 0 ){
        for(int i = 0 ; i <= target_bin ; i++){
          cerr << "id = "<<id<<" "<<lambda_base[id][target_bin]<<" i = "<<i<<endl;
        }
        cerr << lambda_base[id][target_bin] <<" "<< log_density_in_log<<endl;
      }
      assert(lambda_base[id][target_bin] + log_density_in_log > 0);
      log_density = log(lambda_base[id][target_bin] + log_density_in_log);
      prev_end = test_sessions[i].end.hours();
      loglik += log_density - normalized;
      // add this test session to the history so that we are using the correct history
      history.push_back(test_sessions[i]);
      H++;
    }   

    return PredictRes(0,
        loglik/num_sessions,
        true);
  }
}
const char * HawkesProcessModel::modelName(){
  return "hawkes_process_model";    
}
