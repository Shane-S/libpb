#ifndef PB_FLOAT_MATH_H
#define PB_FLOAT_MATH_H

#include <stddef.h>
#include <stdint.h>
#include <pb/util/util_exports.h>

/**
 * Masks out the last "fuzz_bits" bits of the mantissa so that the float can be
 * more reliably compared for equality.
 *
 * 0 and all subnormals are converted to the smallest possible float value
 * (2 ^ -126 * (1 + 2 ^ -23)) since that's close enough to 0 for this application.
 * The sign is kept though.
 *
 * Note: this function doesn't try to handle NaN or Infinity.
 *
 * @param f         The float from which to remove precision.
 * @param fuzz_bits The number of bits to remove. Must be between 0 and 23 (should
 *                  probably be somewhere in the range of 3-6).
 * @return An unsigned int with the bits masked out as described above.
 */
PB_UTIL_DECLSPEC uint32_t PB_UTIL_CALL pb_fuzz_float(float f, size_t fuzz_bits);

/**
 * Compares floats for approximate equality. Based partially on this StackOverflow answer:
 * http://stackoverflow.com/questions/4238122/hash-function-for-floats#4238236
 *
 *
 * @param f1        The first float to compare.
 * @param f2        The second float to compare.
 * @param fuzz_bits The number of bits to and out of the mantissa for comparison.
 *
 * @return Whether f1 and f2 are separated by <= max_ulps.
 */
PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_float_approx_eq(float f1, float f2, size_t fuzz_bits);

#endif /* PB_FLOAT_MATH_H */