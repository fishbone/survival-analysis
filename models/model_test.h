#include "model_base.h"
#include <iostream>
#include <sstream>
class ModelTest : public ModelBase {
  public:
    virtual const char *modelName(){
        ss.clear();
        double v = _config["test"]["a"].as<double>();;
        ss<<"model_test[v="<<v<<"]";
        auto x = _config["test"]["x"].as<std::vector<int> >();
        for(int i = 0; i < x.size(); ++i){
            ss<<" "<<x[i];
        }
        ss<<"\n";
	return ss.str().c_str();
    }

    virtual PredictRes predict(const User &user){
        return PredictRes(0, 0, 0.0, false);
    }
    virtual double predictRateValue(long uid, int s_id, double _time){
      std::cerr <<"shouldn't call predictValueRate for model_test !!" << std::endl;
      assert(false);
      return 0.0;
    }
    virtual double evalPerp(std::vector<DataPoint> & ){
      return 0.0;
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
