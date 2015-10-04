#ifndef __MODELS_H__
#define __MODELS_H__
#include <vector>
#include <fstream>
#include <tuple>
#include "user.h"
#include "jsoncons/json.hpp"
class ModelBase {
  public:
    struct PredictRes{
        PredictRes(long n, double p, int s, bool v):next_visit(n),
                                             loglikelihood(p),
                                             n_session(s),
                                             valid(v){}
        long next_visit;
        double loglikelihood;
        bool valid;
        int n_session;
    };
    static ModelBase *makeModel(const char* model_name);
  public:
    ModelBase():_train_data(nullptr), _test_data(nullptr){}
    void setData(const UserContainer *train, const UserContainer *test){
        _train_data = train;
        _test_data = test;
    }
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
            if(iter->second.get_sessions().size() == 0)
                continue;
            PredictRes res = predict(iter->second);
            if(res.valid){
                result.push_back(std::make_tuple(
                    iter->first,
                    res));
            }
        }
    }
  public:
    static bool loadConfig(const char *file){
        _config = jsoncons::json::parse_file(file);
        return true;
    }
    static const jsoncons::json &getConfig(){
        return _config;
    }
  protected:
    static jsoncons::json _config;
    const UserContainer *_train_data;
    const UserContainer *_test_data;
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
