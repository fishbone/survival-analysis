#ifndef __FEATURE_H__
#define __FEATURE_H__
#include <utility>
#include <string>
#include "comm.h"
int getFeatureOffset(std::string &name);
int getNumberOfFeature();
typedef std::pair<int, FLOAT> Feature;
#endif
