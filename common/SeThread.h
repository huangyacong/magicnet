#ifndef __SE_THREAD_H__
#define __SE_THREAD_H__

#include <stdbool.h>

#ifdef __linux

#define _GNU_SOURCE
#include <pthread.h>
#include <unistd.h>
#include <sched.h>

// compile the code with -lrt and -lpthread
#define	THREADHANDLE        pthread_t

#elif (defined(_WIN32) || defined(WIN32))

#define  WIN32_LEAN_AND_MEAN 
#include <windows.h>
#include <process.h>

#define	THREADHANDLE        uintptr_t

#endif

typedef void (*SETHREADPROC)(void *);

THREADHANDLE SeCreateThread(SETHREADPROC pkFun,void *pkFunArgs);

bool SeSchedSetaffinity(int iCpu);

long SeGetCpuNum();

#endif



