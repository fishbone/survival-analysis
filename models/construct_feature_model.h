#ifndef __CONSTRUCT_FEATURE_MODEL_H__
#define __CONSTRUCT_FEATURE_MODEL_H__
#include "data_io.h"
#include <utility>
#include "model_base.h"
#include "sparse_vector.h"
#include <iostream>
#include "feature.h"
#include "util.h"
class ConstructFeatureModel : public ModelBase{

  public:

    int train(const UserContainer *data);

    const char *modelName();

    PredictRes predict(const User &user);

    double predictRateValue(long, int ,double);

    double evalPerp(std::vector<DataPoint> & );

    ConstructFeatureModel(int _type = 0) : feature_type(_type){
      num_kernel = 0;                                                                                                             
      kernels = std::vector<std::pair<Kernels, double>>();                                       
      //initialize the model                                                           
      //  initParams();   
    }

    void buildDataset();

    std::vector<DataPoint> & getTrainSet();

    std::vector<DataPoint> & getTestSet();

  private:

    typedef std::unordered_map<long, std::vector<SparseVector>> DatasetContainer;

    // concatenate train and test data so as to build Hawkes feature
    // for test data
    UserContainer * _concat_data;

    //type of feature we want to build:
    //in comm.h
    //#define NO_FEATURE 0  
    //#define HAWKES_FEATURE 1                                                           
    //#define AUX_FEATUURE 2                                                             
    //#define BOTH_FEATURE 3 
    int feature_type; 

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
