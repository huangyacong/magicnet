#ifndef __SE_THREAD_H__
#define __SE_THREAD_H__

#include "SeBool.h"

#ifdef __linux

#define _GNU_SOURCE
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <sched.h>
#include <time.h>

// compile the code with -lrt and -lpthread

#define	THREADHANDLE        pthread_t
#define SLEEP(m)            usleep((m * 1000))
#define THREAD(id,fun,args) (pthread_create((pthread_t*)&id, 0, (void*(*)(void*)) fun, args))

#elif (defined(_WIN32) || defined(WIN32))

#define   WIN32_LEAN_AND_MEAN 
#include <windows.h>
#include <process.h>

#define	THREADHANDLE        uintptr_t
#define SLEEP(m)            Sleep(m)
#define THREAD(id,fun,args) (id = _beginthread((void(*)(void*))fun, 0, args))

#endif

typedef void (*SETHREADPROC)(void *);

long SeCreateThread(SETHREADPROC pkFun,void *pkFunArgs);
void SeSleep(unsigned long ulMillisecond);
unsigned long long SeGetTickCount();
bool SeSchedSetaffinity(int iCpu);
long SeGetCpuNum();

#endif



