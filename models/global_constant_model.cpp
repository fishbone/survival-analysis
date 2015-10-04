#include "data_io.h"
#include "global_constant_model.h"
#include <iostream>
using namespace std;
double GlobalConstantModel::evalTrainPerp(const UserContainer *data){
    double loglik = 0.0;
    int n_session = 0;
  for(auto iter = data->begin();
      iter != data->end(); ++iter){
    int index = 0;
    double prev_end = -1;

    for (auto j = iter->second.get_sessions().begin();
        j!= iter->second.get_sessions().end();
        ++j){
      if (index == 0){
        index++;
        prev_end = j->end.hours();
      } else{
        double log_density = log(lambda);
        double integral_lambda = lambda*(j->start.hours() - prev_end);
        prev_end = j->end.hours();
        loglik += log_density - integral_lambda;
        n_session ++;
      }
    }
  }
  cerr <<" train session = "<<n_session<<" avg perp = "<<exp(-loglik/(double)n_session)<<endl;
}
int GlobalConstantModel::train(const UserContainer *data){
  double total_time = 0;
  double session_num = 0;
  _user_train = data;
  for(auto iter = data->begin();
      iter != data->end(); ++iter){
    int index = 0;
    double prev_time = -1;

    for (auto j = iter->second.get_sessions().begin();
        j!= iter->second.get_sessions().end();
        ++j){
      if (index == 0){
        index++;
        prev_time = j->end.hours();
      } else{
        total_time += (j->start.hours() - prev_time);
        prev_time = j->end.hours();
        session_num ++;
      }
    }
  }
  lambda = session_num / total_time;
  cerr <<"finished training "<< string(modelName());                            
   evalTrainPerp(data);
  return 0;
}

ModelBase::PredictRes GlobalConstantModel::predict(const User &user){
  // user here should contain test data sessions
  auto ite = _user_train->find(user.id());
  if(ite == _user_train->end()){
    return PredictRes(-1, 0.0, 0.0, false);
  }else{
    const vector<Session> &train_sessions = ite->second.get_sessions();
    const vector<Session> &test_sessions = user.get_sessions();
    double loglik = 0.0;
    double prev_end = train_sessions.back().end.hours();
    int num_sessions = (int)test_sessions.size();
    for(int i = 0 ; i < num_sessions ; i++){
      // see Alex's note: log lambda_u(t) - \int_0^{cur_start - prev_end} lambda_u(t)
      double log_density = log(lambda);
      double integral_lambda = lambda*(test_sessions[i].start.hours() - prev_end);
      prev_end = test_sessions[i].end.hours();
      loglik += log_density - integral_lambda;
    }

    return PredictRes(0,
        loglik,
        num_sessions,
        true);
  }
}

const char *GlobalConstantModel::modelName(){
  return "global_constant_model";
}	

