#include "piecewise_constant_model.h"
#include "data_io.h"
#include <iostream>
using namespace std;

int PiecewiseConstantModel::train(const UserContainer *data){
    _user_train = data;
    for(auto iter = data->begin();
            iter != data->end(); ++iter){
        int prev_bin = -1;
	int target_bin = 0;
	int count = 0;
	vector<double> singleUserBin(NUM_BIN,0.0);
        for (auto j = iter->second.get_sessions().begin();
                j!= iter->second.get_sessions().end();
                ++j){
		target_bin = j->binFromLastSession();
		if (target_bin < 0){
		    cout<<"target bin:"<<target_bin<<endl;
		    cout<<j->start.hours()<<" "<<(j-1)->end.hours()<<endl;
		}
		target_bin = target_bin % NUM_BIN;
		if (prev_bin == -1 || target_bin == prev_bin){
		    count++;
		    prev_bin = target_bin;
		}else{
		    singleUserBin[prev_bin] = count/BIN_WIDTH;
		    cout<<prev_bin<<" "<<singleUserBin[prev_bin]<<endl;
		    count = 1;
		    prev_bin = target_bin;
		}
        }
	if (prev_bin != -1)
	    singleUserBin[prev_bin] = count/BIN_WIDTH;
	cout<<prev_bin<<" "<<singleUserBin[prev_bin]<<endl;
        lambda_u.insert(make_pair(iter->first, singleUserBin));
    }
    return 0;
}
ModelBase::PredictRes PiecewiseConstantModel::predict(const User &user){
    auto ite = _user_train->find(user.id());
    if(ite == _user_train->end() || !lambda_u.count(user.id())){
        return PredictRes(-1, 0.0, false);
    }else{
        const vector<Session> &train_sessions = ite->second.get_sessions();
        const vector<Session> &test_sessions = user.get_sessions();
        double loglik = 0.0;
        double prev_end = train_sessions.back().end.hours();
        int num_sessions = (int)test_sessions.size();
        for(int i = 0 ; i < num_sessions ; i++){
	    int target_bin = test_sessions[i].binFromLastSession();
	    double lambda_j = lambda_u[user.id()][target_bin];
            double log_density = log(lambda_j);
            double normalized = lambda_j*(test_sessions[i].start.hours() - prev_end);
            prev_end = test_sessions[i].end.hours();
            loglik += log_density - normalized;
        }   

        return PredictRes(0,
                loglik/num_sessions,
                true);
    } 
}
const char * PiecewiseConstantModel::modelName(){
    return "piecewise_constant_model";    
}
