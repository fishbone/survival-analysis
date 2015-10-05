#ifndef __GLOBAL_PIECEWISE_CONSTANT_MODEL_H__
#define __GLOBAL_PIECEWISE_CONSTANT_MODEL_H__
#include "data_io.h"
#include "model_base.h"
#include "feature_based_model.h"
#include <iostream>

class GlobalPiecewiseConstantModel : public ModelBase{
  friend class FeatureBasedModel;
  public:
    int train(const UserContainer *data);
    const char *modelName();
    PredictRes predict(const User &user);
    typedef std::vector<double> Lambda;
    double evalTrainPerp(const UserContainer *data);
  private:
    //keep an access to training data because we need this during testing
    const UserContainer *_user_train;
    Lambda lambda_all;  
};
#endif
