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
	vector<double> singleUserBin(NUM_BIN,0.0);
	double T = 0.0;
	vector<int> count(NUM_BIN,0);
	const vector<Session> &sessions = iter->second.get_sessions();
	double prev_end = sessions[0].end.hours();
	for(int i = 1 ; i < sessions.size() ; i++){
	    int target_bin = (sessions[i].start.hours() - prev_end)/(double)BIN_WIDTH;
	    if(target_bin >= NUM_BIN)target_bin = NUM_BIN - 1;
	    T+= sessions[i].start.hours() - prev_end;
            prev_end = sessions[i].start.hours();
	    count[0]++;
	}
	for (int i = 0 ; i < NUM_BIN ; i++){
		singleUserBin[i] = count[i]/T;
	}
        double sum = 0.0;
        int C = 0;
        for(auto l : singleUserBin){
           if(l > 0.0){
               sum += l; C++;
    	   }
        }
        for(int i = 0 ; i < singleUserBin.size(); i++){
            if(singleUserBin[i] == 0.0){
                singleUserBin[i] = sum/C;
            }
        }
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
        if(train_sessions.size() == 1)return PredictRes(-1, 0.0, false);
        for(int i = 0 ; i < num_sessions ; i++){
	    int target_bin = (test_sessions[i].start.hours() - prev_end)/(double)BIN_WIDTH;
	    if(target_bin >= NUM_BIN)target_bin = NUM_BIN - 1;
            prev_end = test_sessions[i].start.hours();
//	    int target_bin = test_sessions[i].binFromLastSession();
	    cout <<"@@@ " << target_bin<<endl;
	    if (target_bin >= 0) {
		double lambda = lambda_u[user.id()][target_bin];
		if (lambda != 0.0)
		    loglik += log(lambda);
		for (int j = 0; j <= target_bin; j++) {
	    	    double lambda_j = lambda_u[user.id()][j];
		    if (lambda_j != 0.0){
            	    	double normalized = lambda_j*BIN_WIDTH;
            	    	loglik += - normalized;
		    }
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
