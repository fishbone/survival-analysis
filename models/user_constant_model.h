#ifndef __RATE_FUNC_MODEL_H
#define __RATE_FUNC_MODEL_H
#include "data_io.h"
#include "model_base.h"
#include <iostream>

class UserConstantModel : public ModelBase{
    public:
        int train(const UserContainer *data);
        long predict(long uid);
        typedef std::unordered_map<long, double> LambdaU;
        LambdaU lambda_u;
};
#endif
