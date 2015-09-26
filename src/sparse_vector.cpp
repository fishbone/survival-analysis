#include "sparse_vector.h"

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
    vec.insert(i,(double)i);                                                
  }                                                                         
  return vec;                                                               
}

double & SparseVector::operator[](int ind){                                               
  return _feature[ind];                                                                                               
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
  for(auto iter = lhs.begin(); iter != vec.end(); ++iter){                
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

