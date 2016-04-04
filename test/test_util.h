#ifndef TEST_UTIL_H
#define TEST_UTIL_H

#include <check.h>

/* Usable AlmostEqual function */
int assert_close_enough(float A, float B, int maxUlps);

#endif