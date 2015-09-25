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
