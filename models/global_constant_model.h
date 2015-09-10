#ifndef __RATE_FUNC_MODEL_H
#define __RATE_FUNC_MODEL_H
#include "data_io.h"
#include "model_base.h"
#include <iostream>

class GlobalConstantModel : public ModelBase{
  public:
    int train(const UserContainer *data);
    const char *modelName();
    long predict(long uid);
    double lambda;
  private:
    const UserContainer *_data;
};
#endif
