#ifndef __USER_CONSTANT_MODEL_H__
#define __USER_CONSTANT_MODEL_H__
#include "data_io.h"
#include "model_base.h"
#include "user_constant_model.h"
#include <unordered_map>
#include "construct_feature_model.h"
#include <iostream>

class UserConstantModel : public ModelBase{
  friend class HawkesProcessModel;
  friend class FeatureBasedModel;
  friend class PiecewiseConstantModel;
  public:
    double predictGofT(DataPoint & , double);
    int train(const UserContainer *data);
    const char *modelName();
    PredictRes predict(const User &user);
    double predictRateValue(long, int, double);
    double evalPerp(std::vector<DataPoint> &);
    void initParams();
    double evalTrainPerp(const UserContainer *);
    typedef std::unordered_map<long, double> LambdaU;
    typedef double Lambda;
  private:
    int max_iter;
    const UserContainer *_user_train;
    double lr_lambda, lr_lambda_u, momentum;
    LambdaU lambda_u, d_lambda_u;
    Lambda lambda, d_lambda;
};
#endif
