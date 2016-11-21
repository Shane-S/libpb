#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pb/util/util_exports.h>
#include <float.h>

PB_UTIL_DECLSPEC float PB_UTIL_CALL pb_float_clamp(float f, float min, float max) {
    if (f < min) {
        f = min;
    } else if (f > max) {
        f = max;
    }

    return f;
}

PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_float_approx_eq(float f1, float f2, size_t max_ulps) {
    int32_t f1_int;
    int32_t f2_int;

    unsigned int ulps_diff;

    /* ULPs method doesn't work well around 0 because of 0's bit pattern 
     * Use FLT_EPSILON instead of 0 in this case (not sure if this actually works) */
    if(f1 == 0 || f2 == 0) {
        if(f1 == f2) {
            return 1;
        } else if(f1 == 0) {
            f1 = FLT_EPSILON;
        } else {
            f2 = FLT_EPSILON;
        }
    }


    /* Use memcpy to avoid breaking strict aliasing rules */
    /* We're assuming that float is the IEEE 754 single-precision float, which it probably is */
    memcpy(&f1_int, &f1, sizeof(float));
    memcpy(&f2_int, &f2, sizeof(float));
    
    /* Check for different sign bits; if we have +0 and -0, then they're the same, otherwise
     * they're definitely not equal */
    if ((f1_int & 0x80000000) != (f2_int & 0x80000000)) {
        /* +0 and -0 are the same, so we can use == here */
        return f1 == f2;
    }

    /* Figure out how many units of last place are different between the floats */
    ulps_diff = abs(f2_int - f1_int);
    return ulps_diff <= max_ulps;
}
