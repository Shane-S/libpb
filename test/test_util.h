#ifndef TEST_UTIL_H
#define TEST_UTIL_H

/* Check seems to link against libcompat, but it doesn't provide a replacement for pid_t 
 * outside of libcompat.h */
#ifdef _MSC_VER
typedef int pid_t;
#endif

#include <check.h>
#include <math.h>
#include <stdlib.h>

/* Usable AlmostEqual function */
int assert_close_enough(float A, float B, int maxUlps);

#endif