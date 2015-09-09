#include "data_io.h"
#include "models.h"
#include <iostream>
using namespace std;

class RateFunctionModel : public ModelBase{
    public:
        virtual int train(UserContainer &data){
	    for(auto iter = data.begin();
		iter != data.end(); ++iter){
		long total_time = 0;
		double session_num = 0;
		int index = 0;
		int prev_time = -1;

		for (auto j = iter->second.get_sessions().begin();
		    j!= iter->second.get_sessions().end();
		    ++j){
		    if (index == 0){
			index++;
			prev_time = j->end;
		    } else{
	 	    	total_time += (j->start - prev_time);
			prev_time = j->end;
		    	session_num ++;
		    }
		}
		double avg = session_num / total_time;
                lambda_u.insert(make_pair(iter->first, avg));
	    }
            return 0;
	}
	typedef std::unordered_map<long, double> LambdaU;
        LambdaU lambda_u;
};

int main(){
    UserContainer train_data;
    train_data.reserve(10000000);
    read_data("/home/yicheng1/survival-analysis/data/user_survive/daily/show_read_stay.%s",
              "20150703",
              "20150705",
              train_data);
    RateFunctionModel *model = new RateFunctionModel();
    model->train(train_data);
    RateFunctionModel::LambdaU &lambda_u = model->lambda_u;
    for(auto iter = lambda_u.begin();
            iter != lambda_u.end(); ++iter){
	cout<<endl<<"User_id="<<iter->first;
	cout<<"\tlambda_u:"<<iter->second<<endl;
    }
    return 0;
}
