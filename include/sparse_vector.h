#ifndef __SPARSE_VECTOR_H__
#define __SPARSE_VECTOR_H__
#include <vector>
#include <fstream>
#include <unordered_map>
#include <iterator>
#include <tuple>
#include "user.h"
#include "jsoncons/json.hpp"

typedef std::unordered_map<int, double>::const_iterator VecIterator;

class SparseVector {
  public:

    SparseVector(){ }

    SparseVector(std::unordered_map<int, double> keyVal){
      for(auto iter = keyVal.begin(); iter != keyVal.end(); ++iter){

        _feature[iter->first] = iter->second;

      }
    }

    int nnz() const{
      return (int)_feature.size();
    }

    double getVal(int ind){
      return  _feature[ind];
    }

    void insert(int ind, double val){
      _feature[ind] = val;
    }

    VecIterator begin() const{
      return _feature.begin();
    }

    VecIterator end() const{
      return _feature.end();
    }


    static double dotProduct(SparseVector &, SparseVector &) ;

  private:
    std::unordered_map<int, double> _feature; // ind:val map
};
#endif
