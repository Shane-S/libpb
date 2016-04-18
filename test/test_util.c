#include "test_util.h"

/* Usable AlmostEqual function */
int assert_close_enough(float A, float B, int maxUlps)
{
    /* Make sure maxUlps is non-negative and small enough that the 
     * default NAN won't compare as equal to anything. */
    ck_assert_msg(maxUlps > 0 && maxUlps < 4 * 1024 * 1024, "Max units of last place musst be >= 0 and < 2^22");
    
    int aInt = *(int*)&A;
    /* Make aInt lexicographically ordered as a twos-complement int */
    if (aInt < 0)
        aInt = 0x80000000 - aInt;
    /* Make bInt lexicographically ordered as a twos-complement int */
    int bInt = *(int*)&B;
    if (bInt < 0)
        bInt = 0x80000000 - bInt;
    int intDiff = abs(aInt - bInt);
    if (intDiff <= maxUlps)
        return 1;
    return 0;
}
