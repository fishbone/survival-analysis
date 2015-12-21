#ifndef __FEATURE_H__
#define __FEATURE_H__
#include <utility>
#include <string>
#include <vector>
#include <unordered_map>
#include "comm.h"

std::vector<int> &getArticleCat(long aid);
int getFeatureOffset(const std::string &name);
int getNumberOfFeature();
void setArtCat(long aid, const std::string &cat);

typedef std::unordered_map<std::string, int> FeatureMap;
typedef std::unordered_map<int, std::string> MapFeature;

extern FeatureMap ffmap;
extern MapFeature mmap;

typedef std::pair<int, FLOAT> Feature;

#endif
