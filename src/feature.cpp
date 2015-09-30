#include "feature.h"
#include <unordered_map>
#include <string>
#include <iostream>
typedef std::unordered_map<std::string, int> FeatureMap;
typedef std::unordered_map<int, std::string> MapFeature;
FeatureMap ffmap;
MapFeature mmap;
int getFeatureOffset(std::string &name){
    int offset = 0;
#pragma omp critical
    {
        if(ffmap.count(name))
            offset = ffmap[name];
        else
            offset = ffmap[name] = ffmap.size();
    }
    return offset;
}
