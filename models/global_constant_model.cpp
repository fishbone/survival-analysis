#include "data_io.h"
#include "global_constant_model.h"
#include <iostream>
using namespace std;

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
    return 0;
}

ModelBase::PredictRes GlobalConstantModel::predict(const User &user){
    // user here should contain test data sessions
    auto ite = _user_train->find(user.id());
    if(ite == _user_train->end() || ite->second.get_sessions().size() <= 1){
        return PredictRes(-1, 0.0, false);
    }else{
        const vector<Session> &train_sessions = ite->second.get_sessions();
        const vector<Session> &test_sessions = user.get_sessions();
        double loglik = 0.0;
        double prev_end = train_sessions.back().end.hours();
        int num_sessions = (int)test_sessions.size();
        for(int i = 0 ; i < num_sessions ; i++){
            // see Alex's note: log lambda_u(t) - \int_0^{cur_start - prev_end} lambda_u(t)
            double log_density = log(lambda);
            double normalized = lambda*(test_sessions[i].start.hours() - prev_end);
	    cout<<lambda<<" "<<log_density<<" "<<normalized<<" "<<test_sessions[i].start.hours()<<" "<<prev_end<<endl;
            prev_end = test_sessions[i].end.hours();
            loglik += log_density - normalized;
	    cout<<lambda<<" "<<log_density<<" "<<normalized<<" "<<test_sessions[i].start.hours()<<" "<<prev_end<<endl;
        }

        return PredictRes(0,
                          loglik/num_sessions,
                          true);
    }
}

const char *GlobalConstantModel::modelName(){
    return "global_constant_model";
}	

