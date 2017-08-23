#ifndef __SE_THREAD_H__
#define __SE_THREAD_H__

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdbool.h>

#ifdef __linux

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <pthread.h>
#include <unistd.h>
#include <sched.h>

// compile the code with -lrt and -lpthread
#define	THREADHANDLE		pthread_t

#elif (defined(_WIN32) || defined(WIN32))

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <process.h>

#define	THREADHANDLE		uintptr_t

#endif

typedef void (*SETHREADPROC)(void *);

THREADHANDLE SeCreateThread(SETHREADPROC pkFun,void *pkFunArgs);

bool SeSchedSetaffinity(int iCpu);

long SeGetCpuNum();

#ifdef	__cplusplus
}
#endif

#endif



