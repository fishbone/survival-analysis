#include "feature_based_model.h"
#include "construct_feature_model.h"
#include "piecewise_constant_model.h"
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
  momentum  = _config["feature_based"]["momentum"].as<double>();
  max_iter  = _config["feature_based"]["max_iter"].as<int>();
  l1_pen    = _config["feature_based"]["l1_pen"].as<double>();
  
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
  ctrFeature.setData(_train_data, _test_data);
  assert(_train_data != nullptr);
  assert(_test_data != nullptr);
  ctrFeature.train(_train_data);
  train_data = ctrFeature.getTrainSet();
  test_data = ctrFeature.getTestSet();
  assert(train_data.size() != 0);
  assert(test_data.size() != 0);
  initParams();
}

double FeatureBasedModel::evalLoglik(vector<DataPoint> & data){
  
  double loglik = 0.0;
  unordered_map<long, int> perUserCount;
  unordered_map<long, double> perUserLik;
  for(int i = 0 ; i < (int)data.size(); i++){
    DataPoint &_point = data[i];

    int bin = _point.bin;
    long uid = _point.uid;
    double y = _point.y;
    double _loglik = 0.0;
    SparseVector &x = _point.x;
    SparseVector &int_x = _point.integral_x;
    
    assert(bin >= 0);
    assert(bin < NUM_BIN);
    
    _loglik += log(this->lambda_base[uid][bin] + SparseVector::dotProduct(W, x));
    for(int b = 0 ; b <= bin; b++){
      _loglik -= BIN_WIDTH * (this->lambda_base[uid][b]);
    }
    _loglik -= BIN_WIDTH * SparseVector::dotProduct(W, int_x);
    
    perUserCount[uid] ++;
    perUserLik[uid] += _loglik;
  }

  for(auto iter : perUserCount){
    loglik += perUserLik[iter.first]/iter.second;
  }
  cout << "evaluated_user = "<< perUserCount.size()<<endl;
  return loglik/(double)perUserCount.size();

}

int FeatureBasedModel::train(const UserContainer *data){

  init();
  cout <<"initializing base rate using piecewise constant..."<<endl;
  PiecewiseConstantModel base;                                                  
  base.train(data);                                                             
  this->lambda_base = base.lambda_u;

  for(auto iter = base.lambda_u.begin(); iter != base.lambda_u.end(); ++iter){
    this->lambda_base[iter->first] = vector<double>(NUM_BIN,1/(double)NUM_BIN); 
    this->d_lambda_base[iter->first] = vector<double>(NUM_BIN,0.0); 
    this->g_lambda_base[iter->first] = vector<double>(NUM_BIN,0.0); 
  } 
  assert(train_data.size() != 0); // shoud call buildDataset before start training

  for(int iter = 0 ; iter < max_iter ; iter++){

    cerr <<"Iter: "<<iter+1<<" ------loglik(train_data) = "<<evalLoglik(train_data)<<endl;
    cerr <<"Iter: "<<iter+1<<" ------loglik(test_data)  = "<<evalLoglik(test_data)<<endl;
    random_shuffle ( train_data.begin(), train_data.end() );
    double scale = 0.0;
    int n_data = 0;
    for(int i = 0 ; i < (int)train_data.size() ; i++){
      n_data ++;
      if(n_data % 10000 == 0){
        cout <<"Training with SGD: " << n_data<<"/"<<train_data.size()<<endl;
      }
      long uid = train_data[i].uid;
      int bin = train_data[i].bin;
      SparseVector &x = train_data[i].x;
      SparseVector &int_x = train_data[i].integral_x;
      double y = train_data[i].y;
      double divider = lambda_base[uid][bin] + SparseVector::dotProduct(W, x);
      assert(lambda_base[uid][bin] + SparseVector::dotProduct(W, x) > 0 );
      scale = 1/(double)data->at(uid).get_sessions().size();
      for(int b = 0 ; b <= bin ; b++){
        if(b == bin){
          d_lambda_base[uid][b] = momentum * d_lambda_base[uid][b] 
            - lr_lambda *scale * (BIN_WIDTH - 1.0/divider);
        }else{
          d_lambda_base[uid][b] = momentum * d_lambda_base[uid][b] 
            - lr_lambda * scale * (BIN_WIDTH);
        }
        lambda_base[uid][b] += d_lambda_base[uid][b];
        lambda_base[uid][b]  = max(lambda_base[uid][b] ,1e-6);
      }
//      cout << int_x<<endl;      
      SparseVector gradW = int_x * BIN_WIDTH - x/divider;
      vector<int> indices = gradW.getIndices();
      dW.mulEq(momentum, &indices);
      dW.subEq(gradW * lr_w * scale, &indices);
      W.addEq(dW, &indices);
//      dW = dW *momentum - (int_x * BIN_WIDTH - x/divider) * lr_w * scale;
//      W += dW;
      W.threshold(0, &indices);

      W.proxMap(l1_pen, &indices);

      //W.threshold(0, &indices);
//      W.threshold(0);
//      cerr << W.norm2()<<" "<<dW.norm2()<<endl;
//      if((iter + 1) % 5 == 0){
//        cout << W<<endl;
//      }
      //cout << W<<endl;
    }
  }

  return 0;
}   

ModelBase::PredictRes FeatureBasedModel::predict(const User &user){
  return PredictRes(0,0,false);
}

const char * FeatureBasedModel::modelName(){
  return "feature_based_model";
}
