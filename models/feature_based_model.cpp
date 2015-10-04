#include "feature_based_model.h"
#include "construct_feature_model.h"
#include "piecewise_constant_model.h"
#include "global_piecewise_constant_model.h"
#include <algorithm> 
#include <unordered_map>
#include <utility>
#include <math.h>
#include "model_base.h"
#include "sparse_vector.h"
#include <iostream>
using namespace std;
const static int LABEL_INDEX = 0;
void FeatureBasedModel::initParams(){

  // feature-based parameters
  lr_w      = _config["feature_based"]["lr_w"].as<double>();
  lr_lambda = _config["feature_based"]["lr_lambda"].as<double>();
  lr_lambda_u = _config["feature_based"]["lr_lambda_u"].as<double>();
  momentum  = _config["feature_based"]["momentum"].as<double>();
  max_iter  = _config["feature_based"]["max_iter"].as<int>();
  l1_pen    = _config["feature_based"]["l1_pen"].as<double>();
  l2_pen    = _config["feature_based"]["l2_pen"].as<double>();
  // weights for features
  //W  = SparseVector::rand_init(num_feature);
  W  = SparseVector::zero_init(1);
  gW = SparseVector::zero_init(1);
  dW = SparseVector::zero_init(1);
}

FeatureBasedModel::FeatureBasedModel() {
  // default parameters
  max_iter = 20;
  momentum = 0.0;
  num_feature = getNumberOfFeature();
  //initialize the model
}

void FeatureBasedModel::init(){
  initParams();
  cerr <<"=====lr_lambda = "<<lr_lambda<<endl
    <<"=====lr_lambda_u= "<<lr_lambda_u<<endl
    <<"=====momentum = "<<momentum<<endl
    <<"=====lr_w = "<<lr_w<<endl
    <<"=====l2_pen = "<<l2_pen<<endl
    <<"=====l1_pen = "<<l1_pen<<endl
    <<"=====kernel = "<< _config["hawkes"]["num_kernel"].as<int>()
    <<"=====history= "<<_config["hawkes"]["history_size"].as<int>()<<endl;
  ctrFeature.setData(_train_data, _test_data);
  assert(_train_data != nullptr);
  assert(_test_data != nullptr);
  ctrFeature.train(_train_data);
  train_data = ctrFeature.getTrainSet();
  test_data = ctrFeature.getTestSet();
  assert(train_data.size() != 0);
  assert(test_data.size() != 0);
}

double FeatureBasedModel::evalLoglik(vector<DataPoint> & data){

  double loglik = 0.0;
  unordered_map<long, int> perUserCount;
  unordered_map<long, double> perUserLik;
  long n_session = 0;
  for(int i = 0 ; i < (int)data.size(); i++){
    DataPoint &_point = data[i];

    int target_bin = min(NUM_BIN - 1,_point.bin);
    long uid = _point.uid;
    double y = _point.y;
    double _loglik = 0.0;
    SparseVector &x = _point.x;
    SparseVector &int_x = _point.integral_x;
    assert(_point.start - _point.prev_end >= 0);
    assert(target_bin >= 0);

    _loglik += log(lambda[target_bin] + lambda_u[uid] + SparseVector::dotProduct(W, x));
    for(int b = 0 ; b < target_bin; b++){
      _loglik -= lambda[b] * BIN_WIDTH;
    }
    _loglik -= SparseVector::dotProduct(W, int_x) +  
      (_point.start - (_point.prev_end + target_bin * BIN_WIDTH)) * 
      lambda[target_bin] + 
      (_point.start - _point.prev_end) * lambda_u[uid];

    perUserCount[uid] ++;
    perUserLik[uid] += _loglik;
    loglik += _loglik;
  }

    return exp(-loglik/(double)data.size());

    for(auto iter : perUserCount){
    loglik += perUserLik[iter.first]/iter.second;
  }
  cout << "evaluated_user = "<< perUserCount.size()<<endl;
  return exp(-loglik/(double)perUserCount.size());
  //return loglik/(double)perUserCount.size();

}

int FeatureBasedModel::train(const UserContainer *data){

  init();
  cout <<"initializing base rate using piecewise constant..."<<endl;
  GlobalPiecewiseConstantModel global;
  UserConstantModel baseu;
  global.train(data);
  baseu.train(data);
  for(auto iter = baseu.lambda_u.begin(); iter != baseu.lambda_u.end(); ++iter){
    this->lambda_u[iter->first] =0.0; 
    this->d_lambda_u[iter->first] = 0.0;
  } 
  //this->lambda =  global.lambda_all;
  this->d_lambda =  vector<double>(NUM_BIN, 0.0);
  this->lambda =  vector<double>(NUM_BIN, EPS_LAMBDA);
//  this->lambda_u = base.lambda_u;
  assert(train_data.size() != 0); // shoud call buildDataset before start training
  random_shuffle ( train_data.begin(), train_data.end() );
  double best_test = 2147483647.0;
  for(int iter = 0 ; iter < max_iter ; iter++){
    cerr <<"Iter: "<<iter+1<<" ------loglik(train_data) = "<<evalLoglik(train_data)<<endl;
    double test_log_lik = evalLoglik(test_data);
    if(test_log_lik < best_test){
      best_test = test_log_lik;
    }
    cerr <<"Iter: "<<iter+1<<" ------loglik(test_data)  = "<<test_log_lik<<endl;
    cerr <<"Iter: "<<iter+1<<" ------best test loglik   = "<<best_test<<endl;
    double scale = 0.0;
    int n_data = 0;
    for(int i = 0 ; i < (int)train_data.size() ; i++){
      n_data ++;
      if(n_data % 500000 == 0){
        cout <<"Training with SGD: " << n_data<<"/"<<train_data.size()<<endl;
        cerr <<"max(W) = "<<W.max()<<endl;
      }
      long uid = train_data[i].uid;
      int bin = min(NUM_BIN - 1,train_data[i].bin);
      SparseVector &x = train_data[i].x;
      SparseVector &int_x = train_data[i].integral_x;
      double t = train_data[i].y;
      double divider = lambda_u[uid] + lambda[bin] + SparseVector::dotProduct(W, x);
      assert(divider >= 0);
      scale = 1/(double)data->at(uid).get_sessions().size();
    
      for(int b = 0 ; b < bin ; b++){
       d_lambda[b] = 
         momentum * d_lambda[b] - lr_lambda * scale * BIN_WIDTH;
       lambda[b] += d_lambda[b];
       lambda[b] = max(EPS_LAMBDA, lambda[b]);
      }
      d_lambda[bin] = 
         momentum * d_lambda[bin] - lr_lambda * scale * 
         (t - bin * BIN_WIDTH - 1.0/(divider));
      lambda[bin] += d_lambda[bin];
      lambda[bin] = max(EPS_LAMBDA, lambda[bin]);

      d_lambda_u[uid] = 
        momentum * d_lambda_u[uid] - lr_lambda_u * scale * (t - 1.0/(divider));
      lambda_u[uid] += d_lambda_u[uid];
      lambda_u[uid] = max(EPS_LAMBDA, lambda_u[uid]);

      SparseVector gradW = int_x - x/divider;
      vector<int> indices = gradW.getIndices();
      dW.mulEq(momentum, &indices);
      dW.subEq(gradW * lr_w * scale, &indices);
      W.addEq(dW, &indices);
//      W.subEq(lr_w * l1_pen, &indices); // l1 regularization
      W.threshold(0, &indices);
      //      dW = dW *momentum - (int_x * BIN_WIDTH - x/divider) * lr_w * scale;
      //      W += dW;
      //      W.threshold(0, &indices);


      //      W.threshold(0);
      //      cerr << W.norm2()<<" "<<dW.norm2()<<endl;
      //      if((iter + 1) % 5 == 0){
      //        cout << W<<endl;
      //      }
      //cout << W<<endl;
    }
  }
  //cout <<"=========================== W ============================"<<endl;
  //cout << W<<endl;
  cerr <<"=====lr_lambda = "<<lr_lambda<<endl
    <<"=====lr_lambda_u= "<<lr_lambda_u<<endl
    <<"=====momentum = "<<momentum<<endl
    <<"=====lr_w = "<<lr_w<<endl
    <<"=====l2_pen = "<<l2_pen<<endl
    <<"=====l1_pen = "<<l1_pen<<endl
    <<"=====kernel = "<< _config["hawkes"]["num_kernel"].as<int>()
    <<"=====history= "<<_config["hawkes"]["history_size"].as<int>()<<endl
    <<"=====best test loglik = "<<best_test<<endl;

  return 0;
}   

ModelBase::PredictRes FeatureBasedModel::predict(const User &user){
  return PredictRes(0,0,0.0,false);
}

const char * FeatureBasedModel::modelName(){
  return "feature_based_model";
}
