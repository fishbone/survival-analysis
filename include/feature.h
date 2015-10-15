#ifndef __FEATURE_H__
#define __FEATURE_H__
#include <utility>
#include <string>
#include "comm.h"
int getFeatureOffset(std::string &name);
int getNumberOfFeature();

typedef std::unordered_map<std::string, int> FeatureMap;
typedef std::unordered_map<int, std::string> MapFeature;

extern FeatureMap ffmap;
extern MapFeature mmap;

typedef std::pair<int, FLOAT> Feature;
#endif
