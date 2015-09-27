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
        for(auto &i : predict_res){
            assert(std::get<1>(i).valid);
            sum_loglik += std::get<1>(i).loglikelihood;
            ++count;
        }
        if(count == 0)
            return "??";

        std::stringstream ss;
        ss<<"Test ="<<count<<" users\tAverage log-likelihood="<<(sum_loglik / count);
        return ss.str();
    }
};
#endif
