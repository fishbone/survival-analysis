#ifndef __SPARSE_VECTOR_H__
#define __SPARSE_VECTOR_H__
#include <vector>
#include <stdarg.h>
#include <fstream>
#include <google/dense_hash_map>
#include <iterator>
#include <random>
#include <tuple>
#include "user.h"
#include <math.h>
#include "feature.h"
#include "jsoncons/json.hpp"

typedef google::dense_hash_map<int, double>::const_iterator VecIterator;

class SparseVector {
  public:

    SparseVector(){
        _feature.set_empty_key(-1);
        
    }
    SparseVector(std::vector<Feature> &s){
        _feature.set_empty_key(-1);
      for(auto &f : s){
        _feature.insert(f);
      }
    }
    SparseVector(int n, std::vector<Feature> *s){
        _feature.set_empty_key(-1);
        va_list arguments;
        va_start(arguments, s);
        for(int i = 0; i != n; ++i){
            for(auto &f : *s){
                _feature.insert(f);
            }
            s = va_arg(arguments, std::vector<Feature> *);
        }
        va_end(arguments);
    }
    SparseVector(std::unordered_map<int, double> keyVal){
        _feature.set_empty_key(-1);
        
      for(auto iter = keyVal.begin(); iter != keyVal.end(); ++iter){

        _feature[iter->first] = iter->second;

      }
    }


    VecIterator begin() const{
      return _feature.begin();
    }

    VecIterator end() const{
      return _feature.end();
    }

    static SparseVector rand_init(int num_feature);

    static SparseVector zero_init(int num_feature);

    int nnz() const;

    double getVal(int ind);

    void insert(int ind, double val);

    double & operator[](int ind);

    friend SparseVector operator*(const SparseVector &lhs, double scale);

    friend SparseVector operator/(const SparseVector &lhs, double scale);


    friend SparseVector operator-(const SparseVector &lhs, const SparseVector& rhs);

    SparseVector& operator+=(const SparseVector& vec);

    SparseVector& operator-=(const SparseVector& vec);

    friend std::ostream & operator<<(std::ostream &os, const SparseVector &vec);

    void threshold(double T){
      for(auto iter : _feature){
        _feature[iter.first] = std::max(iter.second, T);
      }
    }

    static double dotProduct(SparseVector &, SparseVector &) ;

  private:
    google::dense_hash_map<int, double> _feature; // ind:val map
};
#endif
