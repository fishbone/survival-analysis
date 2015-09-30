#ifndef __CONSTRUCT_FEATURE_MODEL_H__
#define __CONSTRUCT_FEATURE_MODEL_H__
#include "data_io.h"
#include <utility>
#include "model_base.h"
#include "sparse_vector.h"
#include <iostream>
#include "feature.h"
#include "util.h"
struct DataPoint{
  long uid; // start end in unix time (seconds)
  SparseVector x;
  SparseVector integral_x;
  double start, end, prev_end, y;
  int bin, s_id;
  DataPoint():uid(0),s_id(0),y(0),bin(-1),start(-1),end(0){}
  bool operator < (const DataPoint& rhs) const
  {
    if(this->uid < rhs.uid) return true;
    else if(this->uid > rhs.uid) return false;
    return (this->start <= rhs.start);
  }
};

class ConstructFeatureModel : public ModelBase{

  public:

    int train(const UserContainer *data);

    const char *modelName();

    PredictRes predict(const User &user);

    ConstructFeatureModel();

    void buildDataset();


    std::vector<DataPoint> & getTrainSet();

    std::vector<DataPoint> & getTestSet();

  private:

    typedef std::unordered_map<long, std::vector<SparseVector>> DatasetContainer;

    // concatenate train and test data so as to build Hawkes feature
    // for test data
    UserContainer * _concat_data;

    // is the (uid, session) pair from test set ? (we can't use for train))
    std::unordered_map<long, std::unordered_map<int, bool>> isTestSet;

    int num_kernel, history_size, num_feature;

    std::vector<std::pair<Kernels, double>> kernels; // string : type (exp, exp_7d, exp_24h), double: sigma^2
    std::vector<std::string> kernel_name;

    std::vector<DataPoint> train_data;

    std::vector<DataPoint> test_data;
    
    //getFeature in SparseFormat given (uid, session_id, time (in hours))
    SparseVector getFeatureAtTime(long, int, double);

    SparseVector getIntegralFeatureAtTime(long, int, double);

    std::vector<Feature> getHawkesFeatureAtTime(long, int, double );

    std::vector<Feature> getAuxFeatureAtTime(long, int, double );

    std::vector<Feature> getIntegralHawkesFeatureAtTime(long, int, double );

    std::vector<Feature> getIntegralAuxFeatureAtTime(long, int, double );

    void buildVectorizedDataset();

    void initParams();

    void writeToFile(std::string path);

};
#endif
