#include "ComplexityProfiler.h"

int main(void)
{
	struct timespec req;

	_PERF_GET_INST(inst);

	req = {0, 1000000};
	_PERF_PROVE_START(inst, "test-msec");
	nanosleep(&req, NULL);
	_PERF_PROVE_END(inst, "test-msec");


	req = {1, 0};
	_PERF_PROVE_START(inst, "test-sec");
	nanosleep(&req, NULL);
	_PERF_PROVE_END(inst, "test-sec");

	_PERF_PRINT_RESULT(inst, "test");
	_PERF_DEL_INST;

	return EXIT_SUCCESS;
}
