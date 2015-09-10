#ifndef __MODELS_H__
#define __MODELS_H__
#include <vector>
#include <tuple>
#include "user.h"
class ModelBase {
  public:
    typedef long PredictRes;
    static ModelBase *makeModel(const char* model_name);
  public:
    virtual const char *modelName() = 0;
    virtual int train(const UserContainer *data) = 0;
    virtual PredictRes predict(long uid) = 0;

  public:
    void batchPredict(const UserContainer *data,
                      std::vector<std::tuple<long, ModelBase::PredictRes> > &result){
        result.clear();
        for(auto iter = data->begin();
            iter != data->end();
            ++iter){
            PredictRes res = predict(iter->first);
            if(res >= 0){
                result.push_back(std::make_tuple(
                    iter->first,
                    res));
            }
        }
    }
  private:
    bool valid(const PredictRes &res){
        return res >= 0;
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
