#include "user_constant_model.h"
#include "global_constant_model.h"
#include "piecewise_constant_model.h"
#include "construct_feature_model.h"
#include "data_io.h"
#include <iostream>
using namespace std;
double FeatureBasedModel::predictGofT(DataPoint & data, double t){
  int bin = min((int)(t/(double)BIN_WIDTH), NUM_BIN - 1);
  long uid = data.uid;
  int s_id = data.s_id;
  double eps = 1e-8; 
  double integral_lambda = t * (lambda_u[uid] + lambda);
  for(int b = 0 ; b < bin ; b++){ 
    integral_lambda += BIN_WIDTH * lambda_bin[b];    
  }                                  
  integral_lambda += (t - bin * BIN_WIDTH) * lambda_bin[bin];
  SparseVector int_x = this->ctrFeature->getIntegralFeatureAtTime(uid, s_id, data.prev_end + t);;
  integral_lambda += SparseVector::dotProduct(W,int_x);
  return exp(-integral_lambda); 
}
double FeatureBasedModel::predictRateValue(DataPoint & data, double t){
  long uid = data.uid;
  int s_id = data.s_id;
  int bin = min((int)(t/(double)BIN_WIDTH), NUM_BIN - 1);
  double _hours = data.prev_end + t;
  SparseVector x = this->ctrFeature->getFeatureAtTime(uid, s_id, _hours);
  return lambda_u[uid] + lambda + lambda_bin[bin]+ SparseVector::dotProduct(W,x);
}
void FeatureBasedModel::initParams(){                                              

  // feature-based parameters
  string _type = _config["feature_based"]["feature_type"].as<string>();
  if (_type == "NO_FEATURE"){
    feature_type = NO_FEATURE;
  }else if(_type == "HAWKES_FEATURE"){
    feature_type = HAWKES_FEATURE;
  } else if(_type == "AUX_FEATURE"){
    feature_type = AUX_FEATURE;
  } else if(_type == "BOTH_FEATURE") {
    feature_type = BOTH_FEATURE;
  } else {
    cerr <<"specified type "<< _type <<" not valid..."<<endl;
    assert(false);
    // should not get here
  }
  lr_lambda = _config["lr_lambda"].as<double>();
  lr_lambda_u = _config["lr_lambda_u"].as<double>();
  lr_w = _config["lr_w"].as<double>();
  momentum  = _config["momentum"].as<double>();                   
  max_iter  = _config["max_iter"].as<int>();
  cerr <<"=====lr_lambda = "<<lr_lambda<<endl                                      
    <<"=====lr_lambda_u= "<<lr_lambda_u<<endl                                      
    <<"=====lr_w= "<<lr_w<<endl
    <<"=====momentum = "<<momentum<<endl;
  // weights for features                                                          
  //W  = SparseVector::rand_init(num_feature);                                     
}  
double FeatureBasedModel::evalPerp(vector<DataPoint> & data){
  double loglik = 0.0;                                                          
  unordered_map<long, int> perUserCount;                                        
  unordered_map<long, double> perUserLik;                                       
  long n_session =  data.size();
  double sum_loglik =0.0;
  for(int i = 0 ; i < (int)data.size(); i++){                                   
    DataPoint &_point = data[i];                                                
    long uid = _point.uid;
    assert(lambda_u.find(uid) != lambda_u.end()); 
    double y = _point.y;
    SparseVector x = _point.x;
    SparseVector int_x = _point.integral_x;
    int bin = min(NUM_BIN-1, (int)(y/(double)BIN_WIDTH));
    double _loglik = log(lambda_u[uid] + lambda + lambda_bin[bin]+ SparseVector::dotProduct(W,x))
      - y * (lambda + lambda_u[uid]);
    for(int b = 0 ; b < bin ; b++){
      _loglik -= lambda_bin[b] * BIN_WIDTH;
    }
    assert(y >= bin * BIN_WIDTH);
    _loglik -= (y - bin * BIN_WIDTH) * lambda_bin[bin];
    _loglik -= SparseVector::dotProduct(W,int_x);
    perUserCount[uid] ++;                                                       
    perUserLik[uid] += _loglik;                                                 
    //    loglik += _loglik;                                                    
  }                                                                             
  for(auto iter : perUserCount){                                                
    loglik += perUserLik[iter.first]/iter.second;
    sum_loglik += perUserLik[iter.first];
  }                                                                             
  cout << "evaluated_user = "<< perUserCount.size()<<endl;
  //  cerr <<"=========Avg Perp = "<<exp(-sum_loglik/n_session);
  //  cerr <<"=========User-Avg Perp = "<<exp(-loglik/(double)perUserCount.size());
  //return -loglik/(double)perUserCount.size(); 
  return exp(-sum_loglik/n_session);       
}

int FeatureBasedModel::train(const UserContainer *data){
  initParams();
  if(feature_type == NO_FEATURE){
    cerr <<"building no_feature only..."<<endl;
  }else if(feature_type == AUX_FEATURE){
    cerr <<"building aux_feature only..."<<endl;
  } else if(feature_type == HAWKES_FEATURE) {
    cerr <<"building hawkes_feature only..."<<endl;
  } else if(feature_type == BOTH_FEATURE){
    cerr <<"building both hawkes and aux feature..." <<endl;
  } else {
    cerr <<"specified type "<< feature_type <<" not valid..."<<endl;
    assert(false);
  }
  ctrFeature = new ConstructFeatureModel(feature_type);
  ctrFeature->setData(_train_data, _test_data);
  assert(_train_data != nullptr);                                                  
  assert(_test_data != nullptr);
  ctrFeature->train(_train_data);                                                   
  train_data = ctrFeature->getTrainSet();
  test_data = ctrFeature->getTestSet();
  cerr <<"=======# train_sessions = "<<train_data.size()<<endl
    <<"=======# test_sessions = "<<test_data.size()<<endl
    <<"====== feature dimension = " << getNumberOfFeature()<<endl;

  for(auto iter = data->begin();                                                   
      iter != data->end(); ++iter){   
    long uid = iter->first;
    lambda_u[uid] = EPS_LAMBDA;
    d_lambda_u[uid] = 0.0;
  }
  lambda = d_lambda = EPS_LAMBDA;
  lambda_bin = vector<double>(NUM_BIN, EPS_LAMBDA);
  d_lambda_bin = vector<double>(NUM_BIN, 0.0);

  double best_test = 2147483647.0;
  double scale = 1.0;
  for(int iter = 1; iter <= max_iter ; iter++){
    if(iter % 10 == 0){
      scale *= 0.95;
    }
    cerr <<"Iter: "<<iter<<" ------loglik(train_data) = "<<evalPerp(train_data)<<endl;
    double test_log_lik = evalPerp(test_data);                                                                   
    if(test_log_lik < best_test){                                               
      best_test = test_log_lik;                                                 
    }                                                                           
    cerr <<"Iter: "<<iter<<" ------loglik(test_data)  = "<<test_log_lik<<endl;
    cerr <<"Iter: "<<iter<<" ------best test loglik   = "<<best_test<<endl;  
    for(int i = 0 ; i < (int)train_data.size() ; i++){
      if(i % 200000 == 0){
        cout <<"Training with SGD: " << i<<"/"<<train_data.size()<<endl;
      }
      DataPoint & _point = train_data[i];
      long uid = _point.uid;                                                         
      double y = _point.y;
      SparseVector x = _point.x;
      SparseVector int_x = _point.integral_x;
      int bin = min(NUM_BIN-1,(int)(y/(double)BIN_WIDTH));
      double divider = 1.0/(lambda_bin[bin] + lambda_u[uid] + lambda + SparseVector::dotProduct(W,x));
      d_lambda = momentum * d_lambda - lr_lambda  * scale * (y - divider);
      d_lambda_u[uid] = momentum * d_lambda_u[uid] - lr_lambda_u * scale * (y - divider);
      for(int b = 0 ; b < bin ; b++){
        d_lambda_bin[b] = momentum * d_lambda_bin[b] - lr_lambda * scale * BIN_WIDTH;
        lambda_bin[b] += d_lambda_bin[b];
        lambda_bin[b] = max(lambda_bin[b], EPS_LAMBDA);
      }
      d_lambda_bin[bin] = momentum * d_lambda_bin[bin] 
        - lr_lambda * scale * ((y - bin*BIN_WIDTH) - divider);


      lambda_bin[bin] += d_lambda_bin[bin];
      lambda_bin[bin] = max(lambda_bin[bin], EPS_LAMBDA); 
      lambda += d_lambda;
      lambda_u[uid] += d_lambda_u[uid];
      lambda = max(lambda, EPS_LAMBDA);
      lambda_u[uid] = max(lambda_u[uid], EPS_LAMBDA);

      SparseVector gradW = int_x - x * divider;
      vector<int> indices = gradW.getIndices();
      dW.mulEq(momentum, &indices);
      dW.subEq(gradW * lr_w * scale, &indices);
      W.addEq(dW, &indices);
      W.threshold(0, &indices);
    }
  }


  cerr <<"finished training "<< string(modelName())<<endl;
  string stratified_out = _config["stratified_output"].as<string>();
  string expected_return_out = _config["expected_return_output"].as<string>();
  string rate_out = _config["rate_function"].as<string>();

  printStratifiedPerp(stratified_out);
  printStratifiedExpectedReturn(expected_return_out);
  printRandomSampledRateFunction(rate_out); 
  // evalTrainPerp(data);
  return 0;
}
ModelBase::PredictRes FeatureBasedModel::predict(const User &user){
  return PredictRes(-1, 0.0, 0.0, false);
  auto ite = _user_train->find(user.id());
  if(ite == _user_train->end() || ite->second.get_sessions().size() == 0){
    return PredictRes(-1, 0.0, 0.0, false);
  }else{
    // this compute 1/n_session * p(t' <= t), not the log-likelihood...
    const vector<Session> &train_sessions = ite->second.get_sessions();
    const vector<Session> &test_sessions = user.get_sessions();
    double loglik = 0.0;
    double prev_end = train_sessions.back().end.hours();
    double lambda = lambda_u[user.id()];
    int num_sessions = (int)test_sessions.size();
    for(int i = 0 ; i < num_sessions ; i++){
      double log_density = log(lambda);
      double integral_lambda = lambda*(test_sessions[i].start.hours() - prev_end);
      // 1 - G(t) = 1 - exp(-int_{0}^t lambda(t) dt)
      loglik += log_density - integral_lambda;
      prev_end = test_sessions[i].end.hours();
    }   

    return PredictRes(0,
        loglik,
        num_sessions,
        true);
  } 
}
const char * FeatureBasedModel::modelName(){
  return "feature_based_model";    
}
