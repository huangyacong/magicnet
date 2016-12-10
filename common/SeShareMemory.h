#ifndef __SE_SHAREMEMORY_H__
#define __SE_SHAREMEMORY_H__

#ifdef	__cplusplus
extern "C" {
#endif

#include "SeNetBase.h"

#if (defined(WIN32) || defined(_WIN32))

#include <windows.h>

#ifdef _MSC_VER
#pragma comment(lib, "user32.lib")
#endif

#elif defined(__linux)

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>

#endif

HANDLE SeCreateShareMemory(const char *pcName, unsigned long long ullSize);

HANDLE SeOpenShareMemory(const char *pcName);

void SeCloseShareMemory(HANDLE kHandle);

void *SeViewShareMemory(HANDLE kHandle);

bool SeUnViewShareMemory(void *pvBuf);

int SeGetShareMemoryErrorID();

#ifdef	__cplusplus
}
#endif

#endif


