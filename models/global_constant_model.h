#ifndef __GLOBAL_CONSTANT_MODEL_H__
#define __GLOBAL_CONSTANT_MODEL_H__
#include "data_io.h"
#include "model_base.h"
#include <iostream>

class GlobalConstantModel : public ModelBase{
  public:
    int train(const UserContainer *data);
    const char *modelName();
    PredictRes predict(const User &user);
    double lambda;
  private:
    //keep an access to training data because we need this during testing
    const UserContainer *_user_train;
};
#endif
