#ifndef __FEATURE_BASED_MODEL_H__
#define __FEATURE_BASED_MODEL_H__
#include "data_io.h"
#include "construct_feature_model.h"
#include <utility>
#include "model_base.h"
#include "sparse_vector.h"
#include <iostream>
#include "util.h"


class FeatureBasedModel : public ModelBase{

  public:
    
    int train(const UserContainer *data);
    
    const char *modelName();
    
    PredictRes predict(const User &user);
    
    FeatureBasedModel();

 
  private:

    ConstructFeatureModel ctrFeature;

    typedef std::unordered_map<long, std::vector<double>> LambdaU;
    
    std::vector<DataPoint> train_data, test_data;

    SparseVector W, dW, gW;

    LambdaU lambda_base, g_lambda_base, d_lambda_base;
    
    double lr_w, lr_lambda, momentum, l1_pen;
    
    int num_feature, max_iter;
    
    double evalLoglik(std::vector<DataPoint> &);

    void init();
    
    void initParams();    
};
#endif
