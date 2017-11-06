#ifndef COMPLEXITY_PROFILER_H_
#define COMPLEXITY_PROFILER_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#include <syslog.h>

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <queue>
#include <chrono>
#include <iostream>
#include <system_error>
#include <unordered_map>

using namespace std;

/*
 * WARNING:
 *  be advised, once prove starts with _PERF_PROVE_START, it must be ended with
 *  _PERF_PROVE_END precisely on the same context. it is because that profiler
 *  internally uses global mutex_lock to measure complexity.
 */
#define	_PERF_GET_INST(inst)	\
	ComplexityProfiler *inst = ComplexityProfiler::GetInstance()
#define	_PERF_DEL_INST					\
	do {						\
		ComplexityProfiler::DeleteInstance();	\
	} while (0)
#define	_PERF_PROVE_START(inst, str)									\
	do {												\
		std::string proveName(str);								\
		try {											\
			inst->StartProve(proveName);							\
		} catch (std::errc e) {									\
			fprintf(stderr, "%s: _PERF_PROVE_START failed with %d\n", __FUNCTION__, (int)e);\
		}											\
	} while(0)
#define	_PERF_PROVE_END(inst, str)									\
	do {												\
		std::string proveName(str);								\
		try {											\
			inst->StopProve(proveName);							\
		} catch (std::errc e) {									\
			fprintf(stderr, "%s: _PERF_PROVE_STOP failed with %d\n", __FUNCTION__, (int)e); \
		}											\
	} while(0)
#define	_PERF_PROVE_CANCEL(inst, str)		\
	do {					\
		std::string proveName(str);	\
		inst->CancelProve(proveName);	\
	} while(0)
#define	_PERF_PRINT_RESULT(inst, str)							\
	do {										\
		std::string fileName(str);						\
		try {									\
			inst->SaveProfileResult(fileName);				\
		} catch (std::errc e) {							\
			fprintf(stderr, "%s: _PERF_PRINT_RESULT failed with %d\n",	\
				__FUNCTION__, (int)e);					\
		}									\
	} while(0)

/**
 * Complexity Profiler class
 *
 * Complexity Profiler provides measurement for complexity.
 * User application can use this class any time easily to use prepered macros,
 * to check complexity for CPU load/store, map/unmap, and so on in user application layer.
 *
 * This class can be used under 1 instance(1 process): multi-thread.
 * be advised if the time measurement points are recursive, it includes lock/unlock overhead in result.
 */
class ComplexityProfiler {
public:
	/**
	 * Singleton, get(create) instance
	 */
	static ComplexityProfiler* GetInstance();

	/**
	 * Singleton, delete instance
	 */
	static void DeleteInstance();

	/**
	 * Start prove point with specified prove name.
	 * @param[in] &proveName prove name to start meature.
	 * @exception std::errc invalid argument
	 */
	void StartProve(std::string& proveName) throw (std::errc);

	/**
	 * Stop prove point with specified prove name.
	 * @param[in] &proveName prove name to finish meature.
	 * @exception std::errc invalid argument
	 */
	void StopProve(std::string& proveName) throw (std::errc);

	/**
	 * Cancel prove point with specified prove name.
	 * @param[in] &proveName prove name to cancel meature.
	 */
	void CancelProve(std::string& proveName);

	/**
	 * Save Complexity Profile Result
	 * @exception std::errc system error during log output
	 */
	void SaveProfileResult(std::string& fileName) throw (std::errc);

private:
	/**
	 * Constructor
	 */
	ComplexityProfiler();

	/**
	 * Destructor
	 */
	~ComplexityProfiler();

	/*! pointer for profiler instance  */
	static ComplexityProfiler* profInst;

	/*! complexity temporary buffer table */
	unordered_map<std::string, uint64_t> profResultTemp_;

	/*! complexity total amount table */
	unordered_map<std::string, uint64_t> profResultTotal_;

	/*! complexity max value table */
	unordered_map<std::string, uint64_t> profResultMax_;

	/*! complexity min value table */
	unordered_map<std::string, uint64_t> profResultMin_;

	/*! complexity total count table */
	unordered_map<std::string, uint64_t> profResultCount_;

	/*! profiler global lock */
	pthread_mutex_t	profListLock;

	/*! profiler global lock */
	unordered_map<std::string, pthread_mutex_t> profLockList_;

	/*! profiler process id */
	pid_t		profProcessId;

};

#endif /* COMPLEXITY_PROFILER_H_ */
