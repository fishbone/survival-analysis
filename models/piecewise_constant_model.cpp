#include "piecewise_constant_model.h"
#include "data_io.h"
#include <iostream>
using namespace std;

int PiecewiseConstantModel::train(const UserContainer *data){
    _user_train = data;
    //some bins will have no data, which will result in very poor estimation
    //here we simply fill them by the global_constant estimate...
    double total_time = 0;
    double session_num = 0;
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
    double global_lambda = session_num / total_time;
    for(auto iter = data->begin();
            iter != data->end(); ++iter){
        vector<double> userLambda(NUM_BIN,global_lambda);
        vector<int> countInBin(NUM_BIN,0);
        vector<int> countBeforeBin(NUM_BIN,0);
        const vector<Session> &sessions = iter->second.get_sessions();
        for(int i = 1 ; i < (int)sessions.size() ; i++){
            int target_bin = sessions[i].binFromLastSession();
            countInBin[target_bin]++;
            for(int b = 0 ; b <= target_bin; b++){
                countBeforeBin[b] ++;    
            }
        }
        for (int i = 0 ; i < NUM_BIN ; i++){
            if(countBeforeBin[i] > 0 && countInBin[i] > 0)
                userLambda[i] = countInBin[i]/(double)(countBeforeBin[i] * BIN_WIDTH);
        }
        lambda_u.insert(make_pair(iter->first, userLambda));
    }
    return 0;
}
ModelBase::PredictRes PiecewiseConstantModel::predict(const User &user){
    auto ite = _user_train->find(user.id());
    if(ite == _user_train->end() || ite->second.get_sessions().size() != 0){
        return PredictRes(-1, 0.0, false);
    }else{
        const vector<Session> &train_sessions = ite->second.get_sessions();
        const vector<Session> &test_sessions = user.get_sessions();
        double loglik = 0.0;
        double prev_end = train_sessions.back().end.hours();
        int num_sessions = (int)test_sessions.size();
        for(int i = 0 ; i < num_sessions ; i++){
            int target_bin = (test_sessions[i].start.hours() - prev_end)/(double)BIN_WIDTH;
            if(target_bin < 0){
              cerr << user.id()<<" "<<test_sessions[i].start.hours() <<" "<<prev_end<<" "<<target_bin<<endl;
            }
            assert(target_bin >= 0);
            if(target_bin >= NUM_BIN)target_bin = NUM_BIN - 1;
            prev_end = test_sessions[i].end.hours();
                double lambda = lambda_u[user.id()][target_bin];
                loglik += log(lambda);
                for (int j = 0; j <= target_bin; j++) {
                    double lambda_j = lambda_u[user.id()][j];
                    if (lambda_j != 0.0){
                        double normalized = lambda_j*BIN_WIDTH;
                        loglik += - normalized;
                    }
                }
        }   

        return PredictRes(0,
                loglik/num_sessions,
                true);
    } 
}
const char * PiecewiseConstantModel::modelName(){
    return "piecewise_constant_model";    
}
