#ifndef __PIECEWISE_CONSTANT_MODEL_H__
#define __PIECEWISE_CONSTANT_MODEL_H__
#include "data_io.h"
#include "model_base.h"
#include "user_constant_model.h"
#include <unordered_map>
#include "construct_feature_model.h"
#include <iostream>

class PiecewiseConstantModel : public ModelBase{
  friend class HawkesProcessModel;
  friend class FeatureBasedModel;
  public:
    int train(const UserContainer *data);
    const char *modelName();
    PredictRes predict(const User &user);
    double predictRateValue(long, int, double); 
    double evalPerp(std::vector<DataPoint> &);
    void initParams();
    double evalTrainPerp(const UserContainer *);
    typedef std::unordered_map<long, double> LambdaU;
    typedef double Lambda;
    typedef std::vector<double> LambdaBin;
  private:
    int max_iter;
    const UserContainer *_user_train;
    double lr_lambda, lr_lambda_u, momentum;
    LambdaU lambda_u, d_lambda_u;
    Lambda lambda, d_lambda;
    LambdaBin lambda_bin, d_lambda_bin;
};
#endif
