#include "feature.h"
#include <unordered_map>
#include <string>
typedef std::unordered_map<std::string, int> FeatureMap;
FeatureMap ffmap;
int getFeatureOffset(std::string &name){
    if(ffmap.count(name))
        return ffmap[name];
    ffmap[name] = ffmap.size();
    return ffmap[name];
}
