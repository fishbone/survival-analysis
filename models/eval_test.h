#include "model_base.h"
#include <cmath>
#include <sstream>
class EvalTest : public EvaluationBase {
  public:
    const char *evalName(){
        return "eval_test";
    }
    std::string doEval(const UserContainer *data,
                const std::vector<std::tuple<long, ModelBase::PredictRes> > &predict_res){
        double dis = 0.0;
        int count = 0;
        for(auto i : predict_res){
            long uid = std::get<0>(i);
            auto iter = data->find(uid);
            if(iter == data->end())
                continue;
            const std::vector<Session> &sessions = iter->second.get_sessions();
            if(sessions.size() == 0){
                continue;
            }
            dis += abs(sessions[0].start.hours() - std::get<1>(i).next_visit);
            ++count;
        }
        if(count == 0)
            return "??";

        std::stringstream ss;
        ss<<"TestNum="<<count<<"\tAverDis="<<(dis / count);
        return ss.str();
    }
};
