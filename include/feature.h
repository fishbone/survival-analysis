#ifndef __FEATURE_H__
#define __FEATURE_H__
#include <utility>
#include <string>
int getFeatureOffset(std::string &name);
int getFeatureNumber();
typedef std::pair<int, double> Feature;
#endif
