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
                prev_time = j->end;
            } else{
                total_time += (j->start - prev_time);
                prev_time = j->end;
                session_num ++;
            }
        }
    }
    lambda = session_num / total_time;
    return 0;
}

long GlobalConstantModel::predict(long uid){
    auto ite = _data->find(uid);
    if(ite == _data->end()){
        return -1;
    }else{
        return ite->second.get_sessions().back().end + (1/lambda);
    }
}

const char *GlobalConstantModel::modelName(){
    return "global_constant_model";
}	

