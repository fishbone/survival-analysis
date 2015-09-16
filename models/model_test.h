#include "model_base.h"
#include <iostream>
class ModelTest : public ModelBase {
  public:
    virtual const char *modelName(){
	return "model_test";
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
    const UserContainer *train_data;
};
