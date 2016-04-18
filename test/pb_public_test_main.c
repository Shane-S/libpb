#include <libcompat.h>
#include <check.h>
#include "pb_public_test.h"

#ifdef _WIN32
#include <crtdbg.h>
#endif

int main(void)
{
#ifdef _WIN32
	_CrtSetDbgFlag(_CRTDBG_CHECK_ALWAYS_DF);
#endif
	SRunner *sr = srunner_create(make_pb_generation_suite());
    srunner_add_suite(sr, make_pb_extrusion_suite());
    int nf;

    srunner_run_all(sr, CK_ENV);
    nf = srunner_ntests_failed(sr);
    srunner_free(sr);

    return nf == 0 ? 0 : 1;
}