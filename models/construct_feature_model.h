#ifndef __CONSTRUCT_FEATURE_MODEL_H__
#define __CONSTRUCT_FEATURE_MODEL_H__
#include "data_io.h"
#include <utility>
#include "model_base.h"
#include "sparse_vector.h"
#include <iostream>
#include "util.h"
struct DataPoint{
  long uid, start, end, prev_end; // start end in unix time (seconds)
  SparseVector x;
  SparseVector integral_x;
  double y;
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


  private:

    typedef std::unordered_map<long, std::vector<SparseVector>> DatasetContainer;

    int num_kernel, history_size, num_feature;

    std::vector<std::pair<Kernels, double>> kernels; // string : type (exp, exp_7d, exp_24h), double: sigma^2

    std::vector<DataPoint> all_data;

    SparseVector getHawkesFeatureAtTime(long, int, double );

    void buildHawkesFeature(DatasetContainer &);

    void buildIntegralHawkesFeature(DatasetContainer &);

    std::vector<DataPoint> buildVectorRepresentation(DatasetContainer &, DatasetContainer &, DatasetContainer &);

    void buildLabel(DatasetContainer &);

    void insertEntry(DatasetContainer &, long uid, int session_index, int fea_ind, double fea_val);

    void insertEntry(DatasetContainer &, long uid, int session_index, SparseVector &vec);

    void initParams();

    void writeToFile(std::string path);

};
#endif
