#include "user_constant_model.h"
#include "data_io.h"
#include <iostream>
using namespace std;

int UserConstantModel::train(const UserContainer *data){
    _data = data;
    for(auto iter = data->begin();
            iter != data->end(); ++iter){
        long total_time = 0;
        double session_num = 0;
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
        double avg = session_num / total_time;
        lambda_u.insert(make_pair(iter->first, avg));
    }
    return 0;
}
ModelBase::PredictRes UserConstantModel::predict(const User &user){
    auto ite = _data->find(user.id());
    if(ite == _data->end()){
        return ModelBase::PredictRes(-1, 0, false);
    }else{
        return ModelBase::PredictRes(
            ite->second.get_sessions().back().end.seconds() + (1/(lambda_u[user.id()])),
            0,
            true);
    }
}
const char * UserConstantModel::modelName(){
    return "user_constant_model";    
}
