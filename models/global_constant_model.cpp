#include "data_io.h"
#include "global_constant_model.h"
#include <iostream>
using namespace std;

int GlobalConstantModel::train(const UserContainer *data){
    long total_time = 0;
    double session_num = 0;
    _data = data;
    for(auto iter = data->begin();
        iter != data->end(); ++iter){
        int index = 0;
        int prev_time = -1;

        for (auto j = iter->second.get_sessions().begin();
             j!= iter->second.get_sessions().end();
             ++j){
            if (index == 0){
                index++;
                prev_time = j->end.seconds();
            } else{
                total_time += (j->start.seconds() - prev_time);
                prev_time = j->end.seconds();
                session_num ++;
            }
        }
    }
    lambda = session_num / total_time;
    return 0;
}

ModelBase::PredictRes GlobalConstantModel::predict(const User &user){
    auto ite = _data->find(user.id());
    if(ite == _data->end()){
        return PredictRes(-1, 0.0, false);
    }else{
        return PredictRes(ite->second.get_sessions().back().end.seconds() + (1/lambda),
                          0.0,
                          true);
    }
}

const char *GlobalConstantModel::modelName(){
    return "global_constant_model";
}	

