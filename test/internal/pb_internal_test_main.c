#include "pb_internal_test.h"
#include <check.h>

#ifdef _WIN32
#include <crtdbg.h>
#endif

int main(void)
{
	SRunner *sr;
	int nf;

#ifdef _WIN32
	_CrtSetDbgFlag(_CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_ALLOC_MEM_DF);
#endif

    sr = srunner_create(make_pb_sq_house_layout_suite());
    srunner_add_suite(sr, make_pb_sq_house_graph_suite());
    srunner_add_suite(sr, make_pb_astar_suite());
	srunner_set_tap(sr, "internal_test_results.tap");

    srunner_set_fork_status(sr, CK_NOFORK);
    srunner_run_all(sr, CK_ENV);
    nf = srunner_ntests_failed(sr);
    srunner_free(sr);

    return nf == 0 ? 0 : 1;
}