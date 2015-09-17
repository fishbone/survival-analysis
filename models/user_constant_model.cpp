#include "user_constant_model.h"
#include "data_io.h"
#include <iostream>
using namespace std;

int UserConstantModel::train(const UserContainer *data){
    _user_train = data;
    for(auto iter = data->begin();
            iter != data->end(); ++iter){
        double total_time = 0;
        double session_num = 0;
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
        if(total_time != 0){
            double avg = session_num / total_time;
            lambda_u.insert(make_pair(iter->first, avg));
        }
    }
    return 0;
}
ModelBase::PredictRes UserConstantModel::predict(const User &user){
    auto ite = _user_train->find(user.id());
    if(ite == _user_train->end() || !lambda_u.count(user.id())){
        return PredictRes(-1, 0.0, false);
    }else{
        const vector<Session> &train_sessions = ite->second.get_sessions();
        const vector<Session> &test_sessions = user.get_sessions();
        double loglik = 0.0;
        double prev_end = train_sessions.back().end.hours();
        double lambda = lambda_u[user.id()];
        int num_sessions = (int)test_sessions.size();
        for(int i = 0 ; i < num_sessions ; i++){
            double log_density = log(lambda);
            double normalized = lambda*(test_sessions[i].start.hours() - prev_end);
            prev_end = test_sessions[i].end.hours();
            loglik += log_density - normalized;
        }   

        return PredictRes(0,
                loglik/num_sessions,
                true);
    } 
}
const char * UserConstantModel::modelName(){
    return "user_constant_model";    
}
