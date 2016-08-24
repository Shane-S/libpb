#include <libcompat.h>
#include <check.h>
#include "pb_internal_test.h"

#ifdef _WIN32
#include <crtdbg.h>
#endif

int main(void)
{
	SRunner *sr;
	int nf;

#ifdef _WIN32
	_CrtSetDbgFlag(_CRTDBG_CHECK_ALWAYS_DF);
#endif

	sr = srunner_create(make_pb_vertex_suite());
	srunner_add_suite(sr, make_pb_graph_suite());
	srunner_add_suite(sr, make_pb_squarify_suite());
    srunner_add_suite(sr, make_pb_heap_suite());
	srunner_set_tap(sr, "internal_test_results.tap");

    srunner_run_all(sr, CK_ENV);
    nf = srunner_ntests_failed(sr);
    srunner_free(sr);

    return nf == 0 ? 0 : 1;
}