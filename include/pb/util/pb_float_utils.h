#ifndef PB_FLOAT_MATH_H
#define PB_FLOAT_MATH_H

#include <pb/util/pb_util_exports.h>

/**
 * Clamps f between min and max.
 *
 * Precondition: min <= max.
 *
 * @param min The smallest allowable value for f.
 * @param max The largest allowable value for f.
 * 
 * @return if min <= f <= max, returns f; if f < min, returns min; if f > max, returns max.
 */
PB_UTIL_DECLSPEC float PB_UTIL_CALL pb_float_clamp(float f, float min, float max);

/**
 * Compares floats for approximate equality. The floats are said to be equal if they're
 * within max_ulps (ULP = Unit of Last Place) of one another. Taken from this article:
 * https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
 *
 * NOTE: This function doesn't work if comparing against 0 (not "close to 0", but actual 0).
 *
 * @param f1       The first float to compare.
 * @param f2       The second float to compare.
 * @param max_ulps The maximum number of ULPs allowed between f1 and f2 for the to be considered equal.
 *
 * @return Whether f1 and f2 are separated by <= max_ulps.
 */
PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_float_approx_eq(float f1, float f2, size_t max_ulps);

#endif /* PB_FLOAT_MATH_H */