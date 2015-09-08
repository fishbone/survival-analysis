#ifndef __MODELS_H__
#define __MODELS_H__

class ModelBase {
  public:
    virtual int train(UserArray &data) = 0;
    // Add more predict to it;
  private:
    ModelBase();
};

#endif
