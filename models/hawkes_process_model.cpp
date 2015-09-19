#include "hawkes_process_model.h"
#include "data_io.h"
#include <iostream>
#include <random>
using namespace std;

void HawkesProcessModel::setLearningRate(double lr){
  this->lr = lr;    
}

void HawkesProcessModel::updateParameter(const long user_id){
    for(int k = 0 ; k < num_kernel; k++){
        alpha[k] -= lr * d_alpha[k];
        sigma[k] -= lr * d_sigma[k];
    }  
    lambda_u[user_id] -= lr * d_lambda_u;
}

HawkesProcessModel::HawkesProcessModel(int _history_size, int _num_kernel):history_size(_history_size), num_kernel(_num_kernel){
    //initialize sigma and alpha randomly
    alpha = vector<double>(num_kernel,0.0);
    sigma = vector<double>(num_kernel,0.0);
    d_alpha = vector<double>(num_kernel,0.0);
    d_sigma = vector<double>(num_kernel,0.0);
    
    normal_distribution<double> distribution(0.0,3.0);
    default_random_engine generator;
    for(int i = 0 ; i < num_kernel; i++){
        alpha[i] =  distribution(generator);
        sigma[i] = distribution(generator);
        sigma[i] *= sigma[i]; // sigma should be > 0
    }
}
int HawkesProcessModel::train(const UserContainer *data){
    _user_train = data;
    for(auto iter = data->begin();
            iter != data->end(); ++iter){
        for(int session_index = 0; session_index != (int)iter->second.get_sessions().size(); 
             session_index++){
              getDerivative(iter->second, session_index);
              updateParameter(iter->first.id());
        }
    }
    return 0;
}
void HawkesProcessModel::getDerivative(const User &user, int session_index){
     auto &sessions = user.get_sessions();
     assert(session_index < (int)sessions.size());
     double start = sessions[session_index].start.hours();
     int id = user.id();
     if(session_index == 0){
      // first session, we can't do anything
      d_lambda_u = 0.0;
      fill(d_alpha.begin(), d_alpha.end(),0.0);
      fill(d_sigma.begin(), d_sigma.end(),0.0);
      return ;
     }else{
       
       int target_bin = sessions[session_index].binFromLastSession();
       double prev_end = sessions[session_index - 1].end.hours();
       double lambda = lambda_u[id];
       // get derivative for lambda_u
       double deno = lambda;
       for(int h = max(0, session_index -  history_size); h < session_index ; h++){
           for(int k = 0 ; k < num_kernel; k++){
               deno += alpha[k] * 
                 exp(-1.0/(2 * sigma[k]) *pow((start - sessions[h].start.hours()),2) ) ;   
           }
       }
       d_lambda_u = BIN_WIDTH * target_bin - 1.0f/deno;
       // get derivative for alpha_k
       for(int k = 0 ; k < num_kernel ; k++){
           double log_numerator =0.0;
           double A = 0.0;
           d_alpha[k] = 0.0;
           // d_alpha[k] = A - B:
           // A = \sum_{b} tau * [\sum_{t_i < t_b}exp(-1/(2.0sigma_k)(t_b - t_i)^2)]
           for(int b = 0 ; b <= target_bin ; b++){
               for(int h = max(0, session_index - history_size); h < session_index ;h++){
                   
                   A += BIN_WIDTH * 
                     exp(-1.0/(2*sigma[k])*
                     pow(prev_end + BIN_WIDTH*b - sessions[h].start.hours(),2));
               }
           }
           //B =  \frac{\sum_{t_i < t}exp(-1/(2*sigma_k)*(t-t_i)^2}
           //{\lambda_u^0 + \sum_{t_i < t}[\sum_{k=1}^K alpha_k exp(-1/(2sigma_k)*(t-t_i)^2]}
           for(int h = max(0, session_index - history_size); h < session_index ;h++){
               log_numerator += exp(-1.0/(2*sigma[k])*
                 pow(start - sessions[h].start.hours(),2));
           }
           d_alpha[k] = A - log_numerator/deno;
       }
     }
}


ModelBase::PredictRes HawkesProcessModel::predict(const User &user){
        return PredictRes(0,
                0,
                false);
}
const char * HawkesProcessModel::modelName(){
    return "hawkes_process_model";    
}
