#include "SeThread.h"

void SeSleep(unsigned long ulMillisecond)
{
	SLEEP(ulMillisecond);
}

long SeCreateThread(SETHREADPROC pkFun,void *pkFunArgs)
{
	THREADHANDLE id = 0;
	return THREAD(id,pkFun,pkFunArgs);
}

unsigned long long SeGetTickCount()
{
#if (defined(_WIN32) || defined(WIN32))
	unsigned long long ulTime = GetTickCount64();
#elif defined(__linux)
	struct timespec kCurrentTime = {0, 0};
	clock_gettime(CLOCK_MONOTONIC, &kCurrentTime);
	unsigned long long ulTime = kCurrentTime.tv_sec * 1000 + kCurrentTime.tv_nsec/(1000 * 1000);
#endif
	return ulTime;
}

bool SeSchedSetaffinity(int iCpu)
{
#if defined(__linux)
	cpu_set_t kCpuSet;
	CPU_ZERO(&kCpuSet); 
	CPU_SET(iCpu, &kCpuSet);
	return sched_setaffinity(0,sizeof(kCpuSet),&kCpuSet) == 0 ? true:false;
#elif (defined(_WIN32) || defined(WIN32))
	return false;
#endif
}

long SeGetCpuNum()
{
	long lCpuNum;
#if (defined(_WIN32) || defined(WIN32))
	SYSTEM_INFO kSysInfo;
	GetSystemInfo(&kSysInfo);
	lCpuNum = (long)kSysInfo.dwNumberOfProcessors;
#elif defined(__linux)
	lCpuNum = sysconf(_SC_NPROCESSORS_ONLN);
#endif
	return lCpuNum;
}

