#include "feature.h"
#include <unordered_map>
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <string>
#include <iostream>
boost::shared_mutex rw_lock;
FeatureMap ffmap;
MapFeature mmap;
std::unordered_map<long, std::vector<int> > art_cat;

std::vector<int> &getArticleCat(long aid){
    return art_cat[aid];
}

void setArtCat(long aid, const std::string& cat){
    art_cat[aid].push_back(getFeatureOffset(cat));
}

int getFeatureOffset(const std::string &name){
    //    {
    //        boost::shared_lock<boost::shared_mutex> lock(rw_lock);
        if(ffmap.count(name))
            return ffmap[name];
        //    }
        //    {
        //        boost::lock_guard<boost::shared_mutex> lock(rw_lock);
        return ffmap[name] = ffmap.size();
        //    }
}

int getNumberOfFeature(){
        return ffmap.size();
}
