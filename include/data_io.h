#ifndef __DATA_IO_H__
#define __DATA_IO_H__
#include <vector>
#include "user.h"
int read_data(bool lastfm,
    std::string feature,
    const char* stay_dirtemp,
    const char* app_dirtemp,
    const char* profile_dirtemp,
    const char* start_day,
    const char* end_day,
    UserContainer &data);
int load_article_information(const char* filename);
int load_lastfm_profile(const char* filename);
#endif
