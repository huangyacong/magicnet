#include "SeThread.h"

long SeCreateThread(SETHREADPROC pkFun,void *pkFunArgs)
{
	THREADHANDLE id = 0;
	return THREAD(id,pkFun,pkFunArgs);
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

