#ifndef __MODELS_H__
#define __MODELS_H__
#include <vector>
#include <tuple>
#include "user.h"
class ModelBase {
  public:
    struct PredictRes{
        PredictRes(long n, double p, bool v):next_visit(n),
                                             loglikelihood(p),
                                             valid(v){}
        long next_visit;
        double loglikelihood;
        bool valid;
    };
    static ModelBase *makeModel(const char* model_name);
  public:
    virtual const char *modelName() = 0;
    virtual int train(const UserContainer *data) = 0;
    virtual PredictRes predict(const User &user) = 0;
  public:
    void batchPredict(const UserContainer *data,
                      std::vector<std::tuple<long, ModelBase::PredictRes> > &result){
        result.clear();
        for(auto iter = data->begin();
            iter != data->end();
            ++iter){
            PredictRes res = predict(iter->second);
            if(res.valid){
                result.push_back(std::make_tuple(
                    iter->first,
                    res));
            }
        }
    }
};

class EvaluationBase {
  public:
    static EvaluationBase *makeEval(const char* eval_name);
  public:
    virtual const char *evalName() = 0;
    virtual std::string doEval(
        const UserContainer *data,
        const std::vector<std::tuple<long, ModelBase::PredictRes> > &predict_res) = 0;
};

#endif
