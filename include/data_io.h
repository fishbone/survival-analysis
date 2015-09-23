#ifndef __DATA_IO_H__
#define __DATA_IO_H__
#include <vector>
#include "user.h"
int read_data(const char* stay_dirtemp,
              const char* app_dirtemp,
              const char* profile_dirtemp,
              const char* start_day,
              const char* end_day,
              UserContainer &data);
#endif
