#ifndef __ADHOC_STATISTICS_MODEL_H__
#define __ADHOC_STATISTICS_MODEL_H__
#include "data_io.h"
#include "model_base.h"
#include "feature_based_model.h"
#include <iostream>

class AdhocStatisticsModel : public ModelBase{
  public:
    int train(const UserContainer *data);
    const char *modelName();
    double predictRateValue(DataPoint &, double){return 0.0;}
    double predictGofT(DataPoint &, double){return 0.0;}
    PredictRes predict(const User &user){return ModelBase::PredictRes(0,0,0,0);}
    double evalPerp(std::vector<DataPoint> & ){return 0.0;}
  private:
    //keep an access to training data because we need this during testing
    const UserContainer *_user_train;
    std::vector<DataPoint> train_data;
    std::vector<DataPoint> test_data;
    void nextVisitTimeHist(std::string fname);
    void sessionLengthHist(std::string fname);
    void userSessionHist(std::string fname);
    void startHourOfDayHist(std::string fname);
	void writeToFile(std::string f_tr, std::string f_te);
};
#endif
