#ifndef __SE_TOOL_H__
#define __SE_TOOL_H__

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdarg.h>
#include <stdio.h>

#if defined(__linux)

#include<sys/mman.h>

#define SE_CONTAINING_RECORD(ptr, type, member) ((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))
#define SE_ALIGN(x) __attribute__((__aligned__((x))))

#elif (defined(_WIN32) || defined(WIN32))

#include <winsock2.h>

#define SE_CONTAINING_RECORD(address, type, field) ((type *)((PCHAR)(address) - (ULONG_PTR)(&((type *)0)->field)))
#define SE_ALIGN(x) __declspec(align(x))

#endif

#define SEALIGNMENT 16

#define SE_OFFSET(type, member) ((unsigned long)(&((type *)0)->member))

void SeStrNcpy(char* pcDstChar, int iDstLen, const char* pcSrcChar);

int SeCopyData(char *dst, int iDstlen, const char *src, int iSrclen);

unsigned int SeStr2Hash(const char *pcStr, int iLen);

long long SeAToLongLong(const char *pcString);

int SeAToInt(const char *pcString);

double SeAToDouble(const char *pcString);

unsigned long long CreateUniqueID(int iServerID, unsigned int uiCount);

int GetServerIDByUniqueID(unsigned long long ullUniqueID);

/*
short [%d] unsigned short [%u] int [%d] unsigned int [%u] long [%ld] unsigned long [%lu] long long [%lld] unsigned long long [%llu] float [%f] double [%f]
please see http://en.cppreference.com/w/cpp/io/c/vfprintf or man vsnprintf
*/
bool SeSnprintf(char *pcStr, int iSize, const char *format, ...);

bool SeLockMem();

void *SeMallocMem(size_t size);

void SeFreeMem(void* pvPtr);

#ifdef	__cplusplus
}
#endif

#endif



