#include "data_io.h"
#include "feature.h"
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <string>
#include <utility>
#include <chrono>
#include <ctime>
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
typedef bool (*read_handle)(std::istream &is, UserContainer &data);

/*
  4612525700	20150628	1435644003	1435645012	4	133961	35	146017	528	306609	34	423517	81
  4612525700	20150628	1435670188	1435670188	0
*/
bool stay_handle(std::istream &is,
                 UserContainer &data){
    long uid, start_time, end_time;
    int arts;
    char read_date[128];
    is>>uid>>read_date>>start_time>>end_time>>arts;

    if(is.eof())
        return true;
    // UTC -> Chine time
    assert(start_time <= end_time);
    start_time += 8 * 60 * 60;
    end_time += 8 * 60 * 60;
    
    if(!data.count(uid)){
        data.insert(std::make_pair(uid, User(uid)));
    }
    
    Session &sess = data[uid].add_session(uid,
                                          start_time,
                                          end_time,
                                          read_date);
    int offset;
    for(int i = 0; i < arts; ++i){
        std::string str_id;
        int stay;
        std::string tmp = str_id + "_read";
        is>>str_id>>stay;
        offset = getFeatureOffset(tmp);
        sess.session_features.push_back({offset, 1});
        tmp = str_id + "_stay";
        offset = getFeatureOffset(tmp);
        sess.session_features.push_back({offset, stay});
    }
    return true;
}

/*
  2597120	20150601	37	66852	162	15392	98693	29	68	62360	23	22	20	55184	143	70540	76823	108	6	133	153	82281	118	36590	492	20457	69030	109797	96344	59602	336	16964	318	88884	50	19632	35741	39	44806	37
  8314842	20150601	55	18933	127988	467	54514	73362	100844	20457	16230	104550	108411	143	77658	89	96341	33876	83	71633	17101	75	458	713	109896	68	13579	109630	30002	19633	46	16685	43	37	59043	69666	287	29	28	11035	792	151	22	12334	18	76696	15	14	13	23	9	66952	75399	6	74373	84355	74491	250
*/
bool app_handle(std::istream &s,
                UserContainer &data){
    long uid;
    char date[128];
    int apps;
    s>>uid>>date>>apps;
    while(s.eof())
        return false;

    if(!data.count(uid)){
        data.insert(std::make_pair(uid, User(uid)));
    }

    std::string str_id;
    for(int i = 0; i < apps; ++i){
        s>>str_id;
        std::string tmp = str_id + "_app";
        int offset = getFeatureOffset(tmp);
        data[uid].add_feature(date, {offset, 1});
    }
}

bool profile_handle(std::istream &s,
                    UserContainer &data){
    long uid;
    char date[128];
    int apps;
    s>>uid>>date>>apps;
    while(s.eof())
        return false;

    if(!data.count(uid)){
        data.insert(std::make_pair(uid, User(uid)));
    }

    std::string str_id;
    for(int i = 0; i < apps; ++i){
        s>>str_id;
        std::string tmp = str_id + "_prof";
        int offset = getFeatureOffset(tmp);
        data[uid].add_feature(date, {offset, 1});
    }
}


bool profile_handle(const std::string &s, UserContainer &data){
    return true;
}

static int read_data_from_file(
    const char* filename,
    UserContainer &data,
    read_handle handle){
    std::ifstream ifs(filename);
    int count = 0;
    while(!ifs.eof()){
        if(count % 1000 == 0){
            std::cerr<<"\rReading: "<<filename<<
                    "\tLoadFactor:"<<data.load_factor()<<
                    "\t"<<count;
        }
        handle(ifs, data);
        ++count;
    }

    std::cerr<<"\rReading: "<<filename
             <<"\tLoadFactor:"<<data.load_factor()
             <<"\t"<<count<<std::endl;

    return 0;
}

int read_data(const char* stay_dirtemp,
              const char* app_dirtemp,
              const char* profile_dirtemp,
              const char* start_day,
              const char* end_day,
              UserContainer &data){
    using namespace boost::gregorian;
    std::string ud(start_day);
    date start(from_undelimited_string(ud));
    ud = end_day;
    date end(from_undelimited_string(ud));
    date_duration inc_date(1);
    int count = 0;
    char filename[256];

    std::cerr<<"Reading profile data"<<std::endl;
    for(date d = start; d <= end; d = d + inc_date){    
        std::string cur_date = to_iso_string(d);
        snprintf(filename,
                 sizeof(filename),
                 profile_dirtemp,
                 cur_date.c_str());
        count += read_data_from_file(filename,
                                     data,
                                     profile_handle);
    }

    std::cerr<<"Reading app data"<<std::endl;
    for(date d = start; d <= end; d = d + inc_date){    
        std::string cur_date = to_iso_string(d);
        snprintf(filename,
                 sizeof(filename),
                 app_dirtemp,
                 cur_date.c_str());
        count += read_data_from_file(filename,
                                     data,
                                     app_handle);
    }

    std::cerr<<"Reading stay data"<<std::endl;
    for(date d = start; d <= end; d = d + inc_date){    
        std::string cur_date = to_iso_string(d);
        snprintf(filename,
                 sizeof(filename),
                 stay_dirtemp,
                 cur_date.c_str());
        count += read_data_from_file(filename,
                                     data,
                                     stay_handle);
    }
    return count;
}

