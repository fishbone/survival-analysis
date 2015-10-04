#ifndef __EVAL_LOGLIK__
#define __EVAL_LOGLIK__
#include "model_base.h"
#include <cmath>
#include <sstream>
#include <iostream>
class EvalLoglik : public EvaluationBase {
  public:
    const char *evalName(){
        return "eval_loglik";
    }
    std::string doEval(const UserContainer *data,
                const std::vector<std::tuple<long, ModelBase::PredictRes> > &predict_res){
        double sum_loglik = 0.0;
        int count = 0;
        long n_session = 0;
        for(auto &i : predict_res){
            assert(std::get<1>(i).valid);
            n_session += std::get<1>(i).n_session;
            double perp = std::get<1>(i).loglikelihood;
            //double perp = std::get<1>(i).loglikelihood/(double)std::get<1>(i).n_session;
            sum_loglik += perp;
            ++count;
          //  std::cerr << std::get<1>(i).loglikelihood<<" "<< std::get<1>(i).n_session<<" "<<sum_loglik<<std::endl;
         //   getchar();
        }
        if(n_session == 0)
            return "??";

        std::stringstream ss;
        ss<<"Test ="<<count<<"\tuser\t"<<n_session<<"\tsessions, Average User log-likelihood="<<exp(-sum_loglik/(double)n_session);
        return ss.str();
    }
};
#endif
