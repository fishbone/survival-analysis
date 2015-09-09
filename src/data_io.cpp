#include "data_io.h"
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/filesystem.hpp>
#include <string>
#include <chrono>
#include <ctime>
static int read_data_from_file(
    const char* filename,
    UserArray &data){
    return 0;
}
int read_data(const char* dirtemp,
              const char* start_day,
              const char* end_day,
              UserArray &data){
    using namespace boost::gregorian;
    std::string ud(start_day);
    date start(from_undelimited_string(ud));
    ud = end_day;
    date end(from_undelimited_string(ud));
    date_duration inc_date(1);
    int count = 0;
    while(start != end){
        std::string cur_date = to_iso_string(start);
        char filename[128];
        snprintf(filename,
                 sizeof(filename),
                 dirtemp, cur_date.c_str());
        count +=  read_data_from_file(filename,
                                      data);
        start = start + inc_date;
    }
    return count;
}
