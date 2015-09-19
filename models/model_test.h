#include "model_base.h"
#include <iostream>
#include <sstream>
class ModelTest : public ModelBase {
  public:
    virtual const char *modelName(){
        ss.clear();
        double v = _config.get<double>("test.a");
        ss<<"model_test[v="<<v<<"]";
	return ss.str().c_str();
    }

    virtual PredictRes predict(const User &user){
        return PredictRes(0, 0, false);
    }
    virtual int train(const UserContainer *data){
        train_data = data;
        for(auto iter = data->begin();
            iter != data->end(); ++iter){
            //std::cout<<"User_id="<<iter->first<<std::endl;
            //std::cout<<"Session:"<<std::endl;
            for(auto j = iter->second.get_sessions().begin();
                j != iter->second.get_sessions().end();
                ++j){
                //std::cout<<j->start<<"\t"<<j->end<<std::endl;
            }
        }
        return 0;
    }
  private:
    std::stringstream ss;
    const UserContainer *train_data;
};
