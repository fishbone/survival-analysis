#ifndef __MODELS_H__
#define __MODELS_H__
#include "user.h"
class ModelBase {
  public:
    virtual const char *modelName() = 0;
    virtual int train(const UserContainer *data) = 0;
    virtual long predict(long uid) = 0;
};

#endif
