#include "ComplexityProfiler.h"

#define	IS_CORRECT_PID		(getpid() == profProcessId)

#define	PERF_LOG_BUF_SIZE	256

using namespace std::chrono;

ComplexityProfiler* ComplexityProfiler::profInst = NULL;

// Constructor
ComplexityProfiler::ComplexityProfiler(void)
{
	profProcessId = getpid();
	profListLock = PTHREAD_MUTEX_INITIALIZER;
}

// Destructor
ComplexityProfiler::~ComplexityProfiler(void)
{
	profProcessId = 0;
	pthread_mutex_destroy(&profListLock);

	profResultTotal_.clear();
	profResultMax_.clear();
	profResultMin_.clear();
	profResultTemp_.clear();
	profResultCount_.clear();
	profLockList_.clear();
}

// Singleton, get(create) instance
ComplexityProfiler *ComplexityProfiler::GetInstance()
{
	/*
	 * actually, singleton is not 100% multi-thread safe,
	 * because it is not protected by mutex during creation.
	 */
	if (!(profInst)) {
		profInst = new ComplexityProfiler();
	}

	return profInst;
}

// Singleton, delete instance
void ComplexityProfiler::DeleteInstance()
{
	if (profInst) {
		delete profInst;
		profInst = NULL;
	}
}

// Prove on(start watch)
void ComplexityProfiler::StartProve(std::string& proveName) throw (std::errc)
{
	system_clock::time_point start, dummy;

	/* check pid if it's forked */
	if ( !(IS_CORRECT_PID) ) {
		fprintf(stdout, "%s: pid is not correct, call _PERF_DEL_INST\n", __FUNCTION__);
		throw std::errc::invalid_argument;
	}

	(void) pthread_mutex_lock(&profListLock);
	if ( profLockList_.find(proveName) == profLockList_.end() ) {
		profLockList_[proveName] = PTHREAD_MUTEX_INITIALIZER;
	}
	(void) pthread_mutex_unlock(&profListLock);

	/* now get the lock and time, and save it on the list */
	(void) pthread_mutex_lock(&profLockList_[proveName]);
	start = system_clock::now();
	profResultTemp_[proveName] = duration_cast<std::chrono::nanoseconds>(start - dummy).count();
}

// Prove off(stop watch)
void ComplexityProfiler::StopProve(std::string& proveName) throw (std::errc)
{
	system_clock::time_point dummy, end;
	uint64_t endns, diffns;

	/* get the time ASAP */
	end = system_clock::now();
	endns = duration_cast<std::chrono::nanoseconds>(end - dummy).count();

	 /* check pid */
	if ( !(IS_CORRECT_PID) ) {
		fprintf(stdout, "%s: pid is not correct, call _PERF_DEL_INST\n", __FUNCTION__);
		(void) pthread_mutex_unlock(&profLockList_[proveName]);
		throw std::errc::invalid_argument;
	}

	/* check see if it's been proved */
	if ( profResultTemp_[proveName] == 0 ) {
		fprintf(stdout, "%s: this has not been proved yet\n", __FUNCTION__);
		//(void) pthread_mutex_unlock(&profLockList_[proveName]);
		throw std::errc::invalid_argument;
	}
	diffns = endns - profResultTemp_[proveName];

	/* reinit temporary buffer */
	profResultTemp_[proveName] = 0;

	/* save each profile result */
	profResultCount_[proveName]++;
	profResultTotal_[proveName] += diffns;
	if (diffns >  profResultMax_[proveName]) {
		profResultMax_[proveName] = diffns;
	}
	if ((diffns <  profResultMin_[proveName]) || (profResultMin_[proveName] == 0)) {
		profResultMin_[proveName] = diffns;
	}
	(void) pthread_mutex_unlock(&profLockList_[proveName]);
}

// Prove cancel(cancel watch anyway)
void ComplexityProfiler::CancelProve(std::string& proveName)
{
	/* no check if it's been proved or not, just cancel*/
	profResultTemp_[proveName] = 0;

	/* and release lock */
	(void) pthread_mutex_unlock(&profLockList_[proveName]);
}

// Print all of the result & clear buffer
void ComplexityProfiler::SaveProfileResult(std::string& fileName) throw (std::errc)
{
	int ret;
	FILE *fp;
	struct	tm	ltime;
	struct	timeval	now;
	char filename[PERF_LOG_BUF_SIZE];
	char timebuf[PERF_LOG_BUF_SIZE];

	ret = snprintf(filename, PERF_LOG_BUF_SIZE, "./complexity_%s.log", fileName.c_str());
	if (ret < 0) {
		fprintf(stdout, "%s: snprintf failed\n", __FUNCTION__);
		throw std::errc::invalid_argument;
	}

	if (gettimeofday(&now, NULL) != 0) {
		fprintf(stdout, "%s: gettimeofday failed\n", __FUNCTION__);
		throw std::errc::invalid_argument;
	}
	if (localtime_r(&now.tv_sec, &ltime) == NULL) {
		fprintf(stdout, "%s: localtime_r failed\n", __FUNCTION__);
		throw std::errc::invalid_argument;
	}
	if (strftime(timebuf, PERF_LOG_BUF_SIZE, "%b %e %T ", &ltime) == 0) {
		fprintf(stdout, "%s: strftime failed\n", __FUNCTION__);
		throw std::errc::invalid_argument;
	}

	fp = fopen(filename, "a+");
	if (fp == NULL) {
		throw std::errc::invalid_argument;
	}

	fprintf(fp, "\n");
	fprintf(fp, "%s:  Library Profile Result[nanoseconds]\n", timebuf);
	fprintf(fp, "%s: ProveName\t Total\t Max\t Min\t Count\t Ave\n", timebuf);

	(void) pthread_mutex_lock(&profListLock);
	for (auto itr = profLockList_.begin(); itr != profLockList_.end(); itr++) {
		std::string pN = itr->first;
		(void) pthread_mutex_lock(&profLockList_[pN]);
	}

	std::map<std::string, uint64_t> sortHash(profResultTotal_.begin(), profResultTotal_.end());
	for (auto itr = sortHash.begin(); itr != sortHash.end(); itr++) {
		std::string pN = itr->first;

		fprintf(fp, "%s: %s\t %ld\t %ld\t %ld\t %ld\t %ld\n", timebuf, pN.c_str(),
			profResultTotal_[pN], profResultMax_[pN], profResultMin_[pN],
			profResultCount_[pN], (profResultTotal_[pN] / profResultCount_[pN]));
	}
	fprintf(fp, "\n");
	fclose(fp);

	/* initialize buffer */
	profResultTotal_.clear();
	profResultMax_.clear();
	profResultMin_.clear();
	profResultTemp_.clear();
	profResultCount_.clear();

	for (auto itr = profLockList_.begin(); itr != profLockList_.end(); itr++) {
		std::string pN = itr->first;
		(void) pthread_mutex_unlock(&profLockList_[pN]);
	}
	(void) pthread_mutex_unlock(&profListLock);
}
