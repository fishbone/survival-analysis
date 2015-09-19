#ifndef __HAWKES_PROCESS_MODEL_H__
#define __HAWKES_PROCESS_MODEL_H__
#include "data_io.h"
#include "model_base.h"
#include <iostream>

class HawkesProcessModel : public ModelBase{
  public:
    int train(const UserContainer *data);
    const char *modelName();
    PredictRes predict(const User &user);
    HawkesProcessModel(int, int );
    typedef std::unordered_map<long, double> LambdaU;
  private:
    const UserContainer *_user_train;
    const int history_size;
    const int num_kernel;
    double lr;
    std::vector<double> alpha;
    std::vector<double> sigma;
    std::vector<double> d_alpha; // store derivatives
    std::vector<double> d_sigma; // store derivatives
    double d_lambda_u; // no need to use LambdaU data structure because we will be using SGD -> consider 1 user at a time.
    LambdaU lambda_u;
    void getDerivative(const User &user, int session_index);
    void setLearningRate(double lr);
    void updateParameter(const long user_id);
};
#endif
