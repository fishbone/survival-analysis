#include "feature.h"
#include <unordered_map>
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <string>
#include <iostream>
boost::shared_mutex rw_lock;
typedef std::unordered_map<std::string, int> FeatureMap;
typedef std::unordered_map<int, std::string> MapFeature;
FeatureMap ffmap;
MapFeature mmap;
int getFeatureOffset(std::string &name){
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

int getFeatureNumber(){
    //    {
    //        boost::shared_lock<boost::shared_mutex> lock(rw_lock);
        return ffmap.size();
        //    }
}
