#include "data_io.h"
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <string>
#include <chrono>
#include <ctime>
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
/*
2252190	14	20150803	5190310387	1438589567	1	114
2252190	14	20150803	5181564928	1438581433	0	0
2252190	14	20150803	5134925829	1438580830	0	0
2252190	14	20150803	5188765191	1438580830	0	0
*/
static int read_data_from_file(
    const char* filename,
    UserContainer &data){
    std::ifstream ifs(filename);
    std::string s;
    int count = 0;
    while(std::getline(ifs, s)){
        if(count % 1000 == 0){
            std::cerr<<"\rReading: "<<filename<<
                    "\tLoadFactor:"<<data.load_factor()<<
                    "\t"<<count;
        }
        int uid_type, read_date, read, stay_time;
        long uid, impr_time, group_id;
        /*
        std::vector<std::string> split_result;
        boost::split(split_result, s, boost::is_any_of("\t"));
        if(split_result.size() != 7){
        }

        uid = atol(split_result[0].c_str());
        impr_time = atol(split_result[4].c_str());
        */
        // uid, uid_type, date, group_id, impr_time, read, stay_time
        int r_num = sscanf(s.c_str(),
                           "%ld\t%d\t%d\t%ld\t%ld\t%d\t%d",
                           &uid, &uid_type, &read_date, &group_id, &impr_time, &read, &stay_time);
        if(r_num != 7){
            std::cerr<<"Error: "<<s<<std::endl;
            continue;
        }
        data[uid].add_impr(impr_time);
        ++count;
    }
    std::cerr<<"\nReading: "<<filename<<
            "\tLoadFactor:"<<data.load_factor()<<
            "\t"<<count;

    return 0;
}
static int read_data_from_dir(
    const char* dirname,
    UserContainer &data){
    using namespace boost::filesystem;
    using namespace std;
    path p(dirname);
    if(!exists(p)){
        std::cerr<<"Path doesn't exist ["<<dirname<<"]"<<endl;
        return 0;
    }
    if(!is_directory(p)){
        std::cerr<<"It't not a directory ["<<dirname<<"]"<<endl;
        return 0;
    }
    int count = 0;
    for (directory_entry& x : directory_iterator(p)){
        const path& f = x.path();
        const char* filename = f.filename().c_str();
        if(filename[0] == '.')
            continue;
        count += read_data_from_file(f.c_str(), data);
    }
    return count;
}
int read_data(const char* dirtemp,
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
    while(start <= end){
        std::string cur_date = to_iso_string(start);
        char dirname[128];
        snprintf(dirname,
                 sizeof(dirname),
                 dirtemp, cur_date.c_str());
        count +=  read_data_from_dir(dirname,
                                     data);
        start = start + inc_date;
    }
    return count;
}
