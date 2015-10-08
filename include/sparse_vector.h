#ifndef __SPARSE_VECTOR_H__
#define __SPARSE_VECTOR_H__
#include <vector>
#include <stdarg.h>
#include <fstream>
#include <unordered_map>
#include <google/dense_hash_map>
#include <google/sparse_hash_map>
#include <iterator>
#include <random>
#include <tuple>
#include "user.h"
#include <math.h>
#include "feature.h"
#include "jsoncons/json.hpp"

//typedef google::dense_hash_map<int, FLOAT>::const_iterator VecIterator;
typedef google::sparse_hash_map<int, FLOAT>::const_iterator VecIterator;

class SparseVector {
  public:

    SparseVector(){
//      _feature.set_empty_key(-1);
//      _feature.set_deleted_key(-2);
    }
    SparseVector(std::vector<Feature> &s){
//      _feature.set_empty_key(-1);
//      _feature.set_deleted_key(-2);
      for(auto &f : s){
        assert(f.first >= 0);
        _feature.insert(f);
      }
    }
    SparseVector(int n, std::vector<Feature> *s){
//      _feature.set_empty_key(-1);
//      _feature.set_deleted_key(-2);
      va_list arguments;
      va_start(arguments, s);
      for(int i = 0; i != n; ++i){
        for(auto &f : *s){
          assert(f.first >= 0);
          _feature.insert(f);
        }
        s = va_arg(arguments, std::vector<Feature> *);
      }
      va_end(arguments);
    }
    SparseVector(std::unordered_map<int, double> &keyVal){
//      _feature.set_empty_key(-1);
//      _feature.set_deleted_key(-2);
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

    std::vector<int> getIndices();

    int nnz() const;

    double getVal(int ind);

    void insert(int ind, double val);

    FLOAT & operator[](int ind);

    double norm2(){
      double sum_sq = 0;
      int count = 0;
      for(auto iter : _feature){
        count ++;
        sum_sq = iter.second * iter.second;
      }
      return sqrt(sum_sq/count);
    }
    
    FLOAT max(){
      FLOAT MAX = 0;
      for(auto iter = _feature.begin(); iter != _feature.end(); ++iter){
        MAX = std::max(MAX, iter->second);
      }
      return MAX;
    }
    
    void deleteKey(int key);

    void proxMap(double t, std::vector<int> *indices = nullptr);

    SparseVector mul(double, std::vector<int> *indices = nullptr);
    
    SparseVector& mulEq(double, std::vector<int> *indices = nullptr);

    SparseVector add(double, std::vector<int> *indices = nullptr);

    SparseVector sub(double, std::vector<int> *indices = nullptr);

    SparseVector add(SparseVector &, std::vector<int> *indices = nullptr);

    SparseVector sub(SparseVector &, std::vector<int> *indices = nullptr);

    SparseVector& addEq(SparseVector &, std::vector<int> *indices = nullptr);

    SparseVector& subEq(SparseVector &, std::vector<int> *indices = nullptr);
    
    SparseVector& subEq(SparseVector , std::vector<int> *indices = nullptr);
    
    SparseVector& subEq(double , std::vector<int> *indices = nullptr);

    SparseVector div(double, std::vector<int> *indices = nullptr);

    friend SparseVector operator*(const SparseVector &lhs, double scale);

    friend SparseVector operator/(const SparseVector &lhs, double scale);

    friend SparseVector operator-(const SparseVector &lhs, const SparseVector& rhs);

    SparseVector& operator+=(const SparseVector& vec);

    SparseVector& operator-=(const SparseVector& vec);

    friend std::ostream & operator<<(std::ostream &os, const SparseVector &vec);

    void threshold(FLOAT T, std::vector<int> *indices = nullptr){
      if(indices != nullptr){
        for(int ind : (*indices)){
            _feature[ind] = std::max(_feature[ind], T);
        }
      }else {
        for(auto iter : _feature){
          _feature[iter.first] = std::max(iter.second, T);
        }
      }
    }

    static double dotProduct(SparseVector &, SparseVector &) ;

  private:
    //google::dense_hash_map<int, FLOAT> _feature; // ind:val map
    google::sparse_hash_map<int, FLOAT> _feature; // ind:val map
};
#endif
