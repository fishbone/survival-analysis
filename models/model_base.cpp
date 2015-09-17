#include "model_base.h"
#include "model_test.h"
#include "eval_loglik.h"
#include "global_constant_model.h"
#include "user_constant_model.h"
#include <string>

ModelBase *ModelBase::makeModel(const char *model_name){
    std::string name = model_name;
    if(name == "model_test"){
        return new ModelTest();
    }else if(name == "global_constant_model"){
        return new GlobalConstantModel();
    }else if(name == "user_constant_model"){
        return new UserConstantModel();
    }
    return nullptr;
}

EvaluationBase *EvaluationBase::makeEval(const char *eval_name){
    std::string name = eval_name;
    if(name == "eval_loglik"){
        return new EvalLoglik();
    }
    return nullptr;
}
