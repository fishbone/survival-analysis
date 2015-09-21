#include "hawkes_process_model.h"
#include "piecewise_constant_model.h"
#include "data_io.h"
#include <iostream>
#include <random>
#include <math.h> 
using namespace std;

void HawkesProcessModel::setLearningRate(double lr){
    this->lr = lr;    
}

void HawkesProcessModel::updateParameter(const long user_id){
    for(int k = 0 ; k < num_kernel; k++){
        alpha[k] -= lr * d_alpha[k];
      //  sigma[k] -= lr * d_sigma[k];
    }
    for(int b = 0 ; b < NUM_BIN ; b++){
        lambda_base[user_id][b] -= lr  * d_lambda_base[user_id][b];
        lambda_base[user_id][b] = max(lambda_base[user_id][b], 1e-2);
//        cerr <<"d_lambda_base[user_id][b] = "<< d_lambda_base[user_id][b];
    }
}

//HawkesProcessModel::HawkesProcessModel(int _history_size, int _num_kernel):history_size(_history_size), num_kernel(_num_kernel){
HawkesProcessModel::HawkesProcessModel(){
    num_kernel = 5;
    history_size = 10;
    lr = 0.000002;
    //initialize sigma and alpha randomly
    alpha = vector<double>(num_kernel,0.0);
    sigma = vector<double>(num_kernel,0.0);
    d_alpha = vector<double>(num_kernel,0.0);
    d_sigma = vector<double>(num_kernel,0.0);

    normal_distribution<double> distribution(0.0,0.1);
    default_random_engine generator;
    for(int i = 0 ; i < num_kernel; i++){
        //alpha[i] =  distribution(generator);
        alpha[i] =  1;
        sigma[i] = pow(0.5,i-2);
    }
}
int HawkesProcessModel::train(const UserContainer *data){
    // the base rate for each (user,bin) pair is initialized using the piecewise-constant model
    PiecewiseConstantModel base;
    cout <<"======training piecewise_constant as base rate function...."<<endl;
    base.train(data);
    this->lambda_base = base.lambda_u;
    for(auto iter = this->lambda_base.begin(); iter != this->lambda_base.end(); ++iter){
        this->d_lambda_base[iter->first] = vector<double>(NUM_BIN,0.0);
        this->lambda_base[iter->first] = vector<double>(NUM_BIN,0.2);
    }
    cout <<"======training hawkes process using stochastic gradient...."<<endl;
    _user_train = data;
    for(int i = 0 ; i < 50; i++){
        int trained_user = 0;
        int n_user = data->size();
        // can be done in parallel.......
        for(auto iter = data->begin();
                iter != data->end(); ++iter){
            trained_user ++;
            int n_session = iter->second.get_sessions().size();
            if(trained_user % 10000 == 0){
              cerr <<"trained_user = "<<trained_user <<" out of "<< n_user <<endl;    
            }
            for(int session_index = 0; session_index != (int)iter->second.get_sessions().size(); 
                    session_index++){
                getDerivative(iter->second, session_index);
                lr = 0.000005 * 1.0/n_session;
                updateParameter(iter->first);
            }
        }
    }
    return 0;
}
void HawkesProcessModel::getDerivative(const User &user, int session_index){
    auto &sessions = user.get_sessions();
    assert(session_index < (int)sessions.size());
    double start = sessions[session_index].start.hours();
    long id = user.id();
    // reset gradient to zero
  //  fill(d_lambda_base[id].begin(), d_lambda_base[id].end(), 0.0);
    fill(d_alpha.begin(), d_alpha.end(),0.0);
    fill(d_sigma.begin(), d_sigma.end(),0.0);
    if(session_index == 0){
        // first session, we can't do anything
        return ;
    }else{

        int target_bin = sessions[session_index].binFromLastSession();
        assert(target_bin >= 0);
//        assert(lambda_base.find(id) != lambda_base.end());
//        assert(d_lambda_base.find(id) != d_lambda_base.end());
        double prev_end = sessions[session_index - 1].end.hours();
        vector<double> & base_u = lambda_base[id];
        vector<double> & d_base_u = d_lambda_base[id];
        // get derivative for all base lambda
        for(int b = 0; b <= target_bin ; b++){
            d_base_u[b] = BIN_WIDTH; 
        }
        // get more derivative for base lambda in target_bin
        double deno = base_u[target_bin];
        for(int h = max(0, session_index -  history_size); h < session_index ; h++){
            for(int k = 0 ; k < num_kernel; k++){
                deno += alpha[k] * 
                    exp(-1.0/(2 * sigma[k]) *
                      pow((start - sessions[h].start.hours()),2) ) ;   
            }
        }
        d_base_u[target_bin] -=  1.0f/deno;
        // get derivative for alpha_k
        for(int k = 0 ; k < num_kernel ; k++){
            double log_numerator =0.0;
            double A = 0.0;
            // d_alpha[k] = A - B:
            // A = \sum_{b} tau * [\sum_{t_i < t_b}exp(-1/(2.0sigma_k)(t_b - t_i)^2)]
            for(int b = 0 ; b <= target_bin ; b++){
                for(int h = max(0, session_index - history_size); h < session_index ;h++){

                    A += BIN_WIDTH * 
                        exp(-1.0/(2*sigma[k]) *
                                pow(prev_end + BIN_WIDTH*(b+1) - sessions[h].start.hours(),2));
                }
            }
            //B =  \frac{\sum_{t_i < t}exp(-1/(2*sigma_k)*(t-t_i)^2}
            //{\lambda_u^0 + \sum_{t_i < t}[\sum_{k=1}^K alpha_k exp(-1/(2sigma_k)*(t-t_i)^2]}
            for(int h = max(0, session_index - history_size); h < session_index ;h++){
                log_numerator += exp(-1.0/(2*sigma[k]) *
                        pow(start - sessions[h].start.hours(),2));
            }
            d_alpha[k] = A - log_numerator/deno;
        }
    }
}

ModelBase::PredictRes HawkesProcessModel::predict(const User &user){
    auto ite = _user_train->find(user.id());
    long id = user.id();
    if(ite == _user_train->end() || ite->second.get_sessions().size() <= 1){
        return PredictRes(0, 0, false);
    }else{
        const vector<Session> &train_sessions = ite->second.get_sessions();
        const vector<Session> &test_sessions = user.get_sessions();
        vector<Session> history;
        if(train_sessions.size() <= history_size){
            history = vector<Session>(train_sessions);    
        }else{
            for(int i = train_sessions.size() - history_size; i < train_sessions.size(); i++){
                history.push_back(train_sessions[i]);    
            }   
        }
        int H = history.size();
        double loglik = 0.0;
        double prev_end = train_sessions.back().end.hours();
        int num_sessions = (int)test_sessions.size();
        for(int i = 0 ; i < num_sessions ; i++){
            double log_density = 0.0;
            double t = test_sessions[i].start.hours();
            double normalized = 0.0;
            double log_density_in_log = 0.0;
            int target_bin = (test_sessions[i].start.hours() - prev_end)/(BIN_WIDTH);
            assert(target_bin >= 0);
            if (target_bin >= NUM_BIN){
                target_bin = NUM_BIN - 1;     
            }
            for(int b = 0 ; b <= target_bin; b++){
                double t_b = prev_end + (b+1) * BIN_WIDTH;
                normalized += BIN_WIDTH *  lambda_base[id][b];
                // computing kernels using history sessions
                for(int h = max(0, H - history_size); h < H; h++){
                    for(int k = 0 ; k < num_kernel; k++){
                        normalized +=  BIN_WIDTH * alpha[k] * exp(-1.0/(2*sigma[k]) *pow(t_b - history[h].start.hours(),2));
                    }
                }
            }
            for(int h = max(0, H - history_size); h < H; h++){
                for(int k = 0 ; k < num_kernel; k++){
                    log_density_in_log +=  BIN_WIDTH * alpha[k] * exp(-1.0/(2*sigma[k]) *pow(t - history[h].start.hours(),2));
                }
            }
            if (lambda_base[id][target_bin] + log_density_in_log < 0){
                for(int k = 0 ; k < num_kernel ; k++){
                 cerr <<alpha[k]<<" @@@"<<endl;   
                 }
                log_density = -1234567;
            }else{
                log_density = log(lambda_base[id][target_bin] + log_density_in_log);
            }
            //log_density = log(lambda_base[id][target_bin] );
            prev_end = test_sessions[i].end.hours();
            loglik += log_density - normalized;
            // add this test session to the history so that we are using the correct history
            history.push_back(test_sessions[i]);
            H++;
        }   

        return PredictRes(0,
                loglik/num_sessions,
                true);
    }
}
const char * HawkesProcessModel::modelName(){
    return "hawkes_process_model";    
}
