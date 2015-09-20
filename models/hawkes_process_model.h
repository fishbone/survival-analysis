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
    HawkesProcessModel();
    typedef std::unordered_map<long, std::vector<double>> LambdaU;
  private:
    const UserContainer *_user_train;
    int history_size;
    int num_kernel;
    double lr;
    LambdaU lambda_base;
    std::vector<double> alpha;
    std::vector<double> sigma;
    std::vector<double> d_alpha; // store derivatives
    std::vector<double> d_sigma; // store derivatives
    LambdaU d_lambda_base; // no need to use LambdaU data structure because we will be using SGD -> consider 1 user at a time.
    void getDerivative(const User &user, int session_index);
    void setLearningRate(double lr);
    void updateParameter(const long user_id);
    double computeSessionLoglik(const User &user, int session_index);
};
#endif
