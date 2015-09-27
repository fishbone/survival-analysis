#ifndef __HAWKES_PROCESS_MODEL_H__
#define __HAWKES_PROCESS_MODEL_H__
#include "data_io.h"
#include "util.h"
#include <utility>
#include "model_base.h"
#include <iostream>

class HawkesProcessModel : public ModelBase{
  public:
    int train(const UserContainer *data);
    const char *modelName();
    PredictRes predict(const User &user);
    HawkesProcessModel();
    typedef std::unordered_map<long, std::vector<double>> LambdaU;
    typedef std::unordered_map<long, std::vector<double>> AlphaU;
  private:
    const UserContainer *_user_train;
    int history_size;
    int num_kernel;
    double lr_alpha, lr_lambda, momentum;
    LambdaU lambda_base;
    std::vector<std::pair<Kernels, double>> kernels; // string : type (exp, exp_7d, exp_24h), double: sigma^2
    std::vector<double> alpha;
    std::vector<double> d_alpha; // store derivatives
    LambdaU d_lambda_base; // no need to use LambdaU data structure because we will be using SGD -> consider 1 user at a time.
    std::vector<double> g_alpha; // for momentum
    LambdaU g_lambda_base; // for momentum
    void getDerivative(const User &user, int session_index);
    void updateParameter(const long user_id, double);
    double evaluateLoglik();
};
#endif
