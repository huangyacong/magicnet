#ifndef __SE_MUTEX_H__
#define __SE_MUTEX_H__

#if (defined(WIN32) || defined(_WIN32))

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

#define   WIN32_LEAN_AND_MEAN 
#include <windows.h>

#define MUTEX_TYPE              CRITICAL_SECTION
#define MUTEX_INIT(m)           InitializeCriticalSection((CRITICAL_SECTION*)(m))
#define MUTEX_DESTROY(m)        DeleteCriticalSection((CRITICAL_SECTION*)(m))
#define MUTEX_LOCK(m)           EnterCriticalSection((CRITICAL_SECTION*)(m))
#define MUTEX_UNLOCK(m)         LeaveCriticalSection((CRITICAL_SECTION*)(m))

#elif defined(__linux)

#include <pthread.h>

#define MUTEX_TYPE              pthread_mutex_t
#define MUTEX_INIT(m)           pthread_mutex_init((pthread_mutex_t*)(m), 0)
#define MUTEX_DESTROY(m)        pthread_mutex_destroy((pthread_mutex_t*)(m))
#define MUTEX_LOCK(m)           pthread_mutex_lock((pthread_mutex_t*)(m))
#define MUTEX_UNLOCK(m)         pthread_mutex_unlock((pthread_mutex_t*)(m))

#endif

#define SeMutex MUTEX_TYPE

void SeMutexInit(SeMutex *pkMutex);

void SeMutexDes(SeMutex *pkMutex);

void SeMutexLock(SeMutex *pkMutex);

void SeMutexUnLock(SeMutex *pkMutex);

#endif


