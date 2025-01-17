#ifndef __GLOBAL_CONSTANT_MODEL_H__
#define __GLOBAL_CONSTANT_MODEL_H__
#include "data_io.h"
#include "model_base.h"
#include "feature_based_model.h"
#include <iostream>

class GlobalConstantModel : public ModelBase{
  friend class FeatureBasedModel;
  public:
    int train(const UserContainer *data);
    const char *modelName();
    double predictRateValue(DataPoint &, double);
    double predictGofT(DataPoint &, double);
    PredictRes predict(const User &user);
    double evalPerp(std::vector<DataPoint> & );
    double lambda;
  private:
    //keep an access to training data because we need this during testing
    const UserContainer *_user_train;
    std::vector<DataPoint> train_data;
    std::vector<DataPoint> test_data;
};
#endif
