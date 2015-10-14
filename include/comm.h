#ifndef __COMM_H__
#define __COMM_H__
#define SESSION_MAX_STOP (30 * 60)
// Hour
extern double BIN_WIDTH;
extern int NUM_BIN;
//#define BIN_WIDTH (0.5)
// number of bins
//#define NUM_BIN (72)
#define NUM_KERNEL_TYPE 6
#define EPS_LAMBDA 1e-5
#define FLOAT float
#define NO_FEATURE 0
#define HAWKES_FEATURE 1
#define AUX_FEATURE 2
#define BOTH_FEATURE 3
#endif
