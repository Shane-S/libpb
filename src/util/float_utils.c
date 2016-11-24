#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pb/util/util_exports.h>

PB_UTIL_DECLSPEC uint32_t PB_UTIL_CALL pb_fuzz_float(float f, size_t fuzz_bits) {
    const uint32_t min_normal = 0x00800001;

    uint32_t fuzzed;
    uint32_t mask = 0xFFFFFFFF << fuzz_bits;
    uint32_t exponent_mask = 0x7F800000;
    uint32_t sign_mask = 0x80000000;

    /* Use memcpy to avoid breaking strict aliasing rules */
    memcpy(&fuzzed, &f, sizeof(float));

    /* Convert subnormals and 0 to min_normal, keeping the sign bit */
    fuzzed = fuzzed & exponent_mask == 0 ? (min_normal | (fuzzed & sign_mask)) : fuzzed;

    return fuzzed & mask;
}

PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_float_approx_eq(float f1, float f2, size_t fuzz_bits) {
    uint32_t f1_fuzzed = pb_fuzz_float(f1, fuzz_bits);
    uint32_t f2_fuzzed = pb_fuzz_float(f2, fuzz_bits);

    return f1_fuzzed == f2_fuzzed;
}
