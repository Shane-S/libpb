#include <libcompat.h>
#include <check.h>
#include "pb_public_test.h"

#ifdef _WIN32
#include <crtdbg.h>
#endif

int main(void)
{
	SRunner *sr = srunner_create(make_pb_generation_suite());
	int nf;

#ifdef _WIN32
	_CrtSetDbgFlag(_CRTDBG_CHECK_ALWAYS_DF);
#endif
    srunner_add_suite(sr, make_pb_extrusion_suite());
    srunner_add_suite(sr, make_pb_hash_suite());
	srunner_set_tap(sr, "public_test_results.tap"); /* Write the test results to a TAP (Test Anything Protocol) file for test harness analysis */

    srunner_run_all(sr, CK_ENV);
    nf = srunner_ntests_failed(sr);
    srunner_free(sr);

    return nf == 0 ? 0 : 1;
}