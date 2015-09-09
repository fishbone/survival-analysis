#include "data_io.h"
#include "models.h"

class ModelTest : public ModelBase{
  public:
    virtual int train(UserArray &data){
        return 0;
    }
  private:
};

int main(){
    UserArray train_data;
    read_data("/home/yicheng1/survival-analysis/data/user_survive/daily/show_read_stay.%s",
              "20150703",
              "20150903",
              train_data);
    ModelBase *model = new ModelTest();
    model->train(train_data);
    return 0;
}
