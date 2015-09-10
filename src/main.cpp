#include "data_io.h"
#include "model_base.h"
#include <iostream>
using namespace std;

class ModelTest : public ModelBase{
  public:
    virtual const char *modelName(){
	return "model_test";
    }

    virtual long predict(long uid){
        return 0;
    }
    virtual int train(const UserContainer *data){
        train_data = data;
        for(auto iter = data->begin();
            iter != data->end(); ++iter){
            cout<<"User_id="<<iter->first<<endl;
            cout<<"Session:"<<endl;
            for(auto j = iter->second.get_sessions().begin();
                j != iter->second.get_sessions().end();
                ++j){
                cout<<j->start<<"\t"<<j->end<<endl;
            }
        }
        return 0;
    }
  private:
    const UserContainer *train_data;
};

int main(){
    UserContainer train_data;
    train_data.reserve(10000000);
    read_data("/home/yicheng1/survival-analysis/data/user_survive/daily/show_read_stay.%s",
              "20150703",
              "20150705",
              train_data);
    ModelBase *model = new ModelTest();
    model->train(&train_data);
    return 0;
}
