#ifndef __USER_CONSTANT_MODEL_H__
#define __USER_CONSTANT_MODEL_H__
#include "data_io.h"
#include "model_base.h"
#include <iostream>

class UserConstantModel : public ModelBase{
    public:
        int train(const UserContainer *data);
        const char *modelName();
        long predict(long uid);
        typedef std::unordered_map<long, double> LambdaU;
  private:
    const UserContainer *_data;
        LambdaU lambda_u;
};
#endif
