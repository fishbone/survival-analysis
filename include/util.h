#ifndef __UTIL_H__
#define __UTIL_H__
#include <vector>
#include <unordered_map>
#include "comm.h"
#include "user.h"
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <iostream>
enum Kernels{rbf, rbf_24h, rbf_7d};

double evalKernel(Kernels &kernel, double sigma, double t1, double t2);
#endif
