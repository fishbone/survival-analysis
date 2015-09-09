#ifndef __MODELS_H__
#define __MODELS_H__
#include "user.h"
class ModelBase {
  public:
    virtual int train(UserContainer &data) = 0;
};

#endif
