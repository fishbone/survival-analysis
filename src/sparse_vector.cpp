#include "sparse_vector.h"
#include <vector>
using namespace std;
double SparseVector::dotProduct(SparseVector & vec1, SparseVector & vec2){

  SparseVector * vec_less = &vec1;    
  SparseVector * vec_more = &vec2;
  double res = 0.0;

  if ( vec1.nnz() > vec2.nnz() ) {
    SparseVector * tmp = vec_less;
    vec_less = vec_more;
    vec_more = tmp;
  }

  for(auto iter = vec_less->begin(); iter != vec_less->end(); ++iter){
    int ind = iter->first;
    res += vec_less->getVal(ind) * vec_more->getVal(ind);
  }

  return res;
}

int SparseVector::nnz() const{
  return (int)_feature.size();   
}

double SparseVector::getVal(int ind){
  return  _feature[ind];                                                       
}

void SparseVector::insert(int ind, double val){                                           
  _feature[ind] = val;                                                         
}

vector<int> SparseVector::getIndices(){
  vector<int> indices;
  for(auto iter = this->begin(); iter != this->end(); ++iter){
    indices.push_back(iter->first);
  }
  return indices;
}

SparseVector SparseVector::rand_init(int num_feature){                             
  std::default_random_engine generator;                                     
  std::uniform_real_distribution<double> distribution(0.0,1.0);             
  SparseVector vec;                                                         
  for(int i = 0 ; i < num_feature ; i++){                                   
    double val = distribution(generator);                                   
    vec.insert(i, val);                                                     
  }                                                                         
  return vec;                                                               
}                                                                           

SparseVector SparseVector::zero_init(int num_feature){                             
  SparseVector vec;                                                         
  for(int i = 0 ;i  < num_feature ; i++){                                   
    vec.insert(i,0);                                                
  }                                                                         
  return vec;                                                               
}

FLOAT & SparseVector::operator[](int ind){                                               
    return _feature[ind];                                                                                               
} 

void SparseVector::deleteKey(int key){
  _feature.erase(key);
}

void SparseVector::proxMap(double t, vector<int> *indices){
  if (indices != nullptr) {
    for(int ind : (*indices)){
      if(abs(_feature[ind]) <= t){
        deleteKey(ind);
      }else {
        if (_feature[ind] > 0) {
          _feature[ind] -= t;
        } else {
          _feature[ind] += t;
        }
      }
    }
  } else {
    vector<int> indices = this->getIndices();
    for(int ind : indices){
      if(abs(_feature[ind]) <= t){
        deleteKey(ind);
      }else {
        if (_feature[ind] > 0) {
          _feature[ind] -= t;
        } else {
          _feature[ind] += t;
        }
      }
    }
  }
}

SparseVector SparseVector::mul(double scale, vector<int> * indices){
  SparseVector vec(*this);
  if (indices != nullptr){
    for(int ind : (*indices)){
      vec[ind] *= scale;
    }
  } else {
    for(auto iter = this->begin(); iter != this->end(); ++iter){
      int ind = iter->first;
      vec[ind] *= scale;
    }
  }
  return vec;
}


SparseVector SparseVector::div(double scale, vector<int> * indices){
  assert(scale != 0.0);
  SparseVector vec(*this);
  if (indices != nullptr){
    for(int ind : (*indices)){
      vec[ind] /= scale;
    }
  } else {
    for(auto iter = this->begin(); iter != this->end(); ++iter){                
      int ind = iter->first;
      vec[ind] /= scale;
    }
  }
  return vec;
}


SparseVector SparseVector::add(double scale, vector<int> * indices){
  SparseVector vec(*this);
  assert(vec.nnz() == this->nnz());
  if (indices != nullptr){
    for(int ind : (*indices)){
      vec[ind] += scale;
    }
  } else {
    for(auto iter = this->begin(); iter != this->end(); ++iter){
      int ind = iter->first;
      vec[ind] += scale;
    }
  }
  return vec;
}

SparseVector SparseVector::add(SparseVector& rhs, vector<int> * indices){
  SparseVector vec(*this);
  if (indices != nullptr){
    for(int ind : (*indices)){
      vec[ind] += rhs.getVal(ind);
    }
  } else {
    for(auto iter = rhs.begin(); iter != rhs.end(); ++iter){
      int ind = iter->first;
      vec[ind] += rhs.getVal(ind);
    }
  }
  return vec;
}

SparseVector SparseVector::sub(SparseVector& rhs, vector<int> * indices){
  SparseVector vec(*this);
  if (indices != nullptr){
    for(int ind : (*indices)){
      vec[ind] -= rhs.getVal(ind);
    }
  } else {
    for(auto iter = rhs.begin(); iter != rhs.end(); ++iter){
      int ind = iter->first;
      vec[ind] -= rhs.getVal(ind);
    }
  }
  return vec;
}

SparseVector SparseVector::sub(double scale, vector<int> * indices){
  SparseVector vec(*this);
  if (indices != nullptr){
    for(int ind : (*indices)){
      vec[ind] -= scale;
    }
  } else {
    for(auto iter = this->begin(); iter != this->end(); ++iter){                
      int ind = iter->first;
      vec[ind] -= scale;
    }
  }
  return vec;
}

SparseVector& SparseVector::subEq(SparseVector & rhs, vector<int> * indices){
  if (indices != nullptr){
    for(int ind : (*indices)){
      _feature[ind] -= rhs.getVal(ind);
    }
  } else {
    for(auto iter = rhs.begin(); iter != rhs.end(); ++iter){
      int ind = iter->first;
      _feature[ind] -= rhs.getVal(ind);
    }
  }
  return (*this);
}

SparseVector& SparseVector::subEq(SparseVector  rhs, vector<int> * indices){
  if (indices != nullptr){
    for(int ind : (*indices)){
      _feature[ind] -= rhs.getVal(ind);
    }
  } else {
    for(auto iter = rhs.begin(); iter != rhs.end(); ++iter){
      int ind = iter->first;
      _feature[ind] -= rhs.getVal(ind);
    }
  }
  return (*this);
}

SparseVector& SparseVector::addEq(SparseVector & rhs, vector<int> * indices){
  if (indices != nullptr){
    for(int ind : (*indices)){
      _feature[ind] += rhs.getVal(ind);
    }
  } else {
    for(auto iter = rhs.begin(); iter != rhs.end(); ++iter){
      int ind = iter->first;
      _feature[ind] += rhs.getVal(ind);
    }
  }
  return (*this);
}

SparseVector& SparseVector::mulEq(double scale, vector<int> * indices){
  if (indices != nullptr){
    for(int ind : (*indices)){
      _feature[ind] *= scale;
    }
  } else {
    for(auto iter = this->begin(); iter != this->end(); ++iter){
      int ind = iter->first;
      _feature[ind] *= scale;
    }
  }
  return (*this);
}

SparseVector operator*(const SparseVector &lhs, double scale){      
  SparseVector vec(lhs);
  for(auto iter = lhs.begin(); iter != lhs.end(); ++iter){                
    vec[iter->first] *= scale;                                
  }                                                                         
  return vec;
}  

SparseVector operator/(const SparseVector& lhs, double scale){      
  SparseVector vec(lhs);                                                         
  assert(scale != 0.0);
  for(auto iter = lhs.begin(); iter != lhs.end(); ++iter){                
    vec[iter->first] /= scale;                                
  }
  return vec;
}                                                                           


SparseVector operator-(const SparseVector & lhs, const SparseVector& rhs){
  SparseVector vec(lhs);                                                  
  for(auto iter = rhs.begin(); iter != rhs.end(); ++iter){              
    vec[iter->first] -= iter->second;                                       
  }                                                                         
  return vec;
}                                                                           

std::ostream & operator<<(std::ostream &os, const SparseVector &vec){
  for(auto iter = vec.begin(); iter != vec.end(); ++iter){                  
    os <<"("<<iter->first<<","<<iter->second<<")"<<" ";                      
  }                                                                         
  os << std::endl;                                                          
}  

SparseVector & SparseVector::operator+=(const SparseVector & rhs){
  for(auto iter = rhs.begin(); iter != rhs.end(); ++iter){
    (*this)[iter->first] += iter->second;
  }
  return *this;
}

SparseVector & SparseVector::operator-=(const SparseVector & rhs){
  for(auto iter = rhs.begin(); iter != rhs.end(); ++iter){
    (*this)[iter->first] -= iter->second;
  }
  return *this;
}

