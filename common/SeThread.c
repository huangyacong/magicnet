#include "SeThread.h"

THREADHANDLE SeCreateThread(SETHREADPROC pkFun,void *pkFunArgs)
{
	THREADHANDLE id = 0;
#ifdef __linux
	pthread_create((pthread_t*)&id, 0, (void*(*)(void*))pkFun, pkFunArgs);
	return id;
#elif (defined(_WIN32) || defined(WIN32))
	id = _beginthread((void(*)(void*))pkFun, 0, pkFunArgs);
	return id;
#endif
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

