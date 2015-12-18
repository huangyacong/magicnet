#ifndef __SE_TOOL_H__
#define __SE_TOOL_H__

#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdarg.h>
#include <stdio.h>

#if defined(__linux)
#define SE_CONTAINING_RECORD(ptr, type, member) ((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))
#define SE_ALIGN(x) __attribute__((__aligned__((x))))
#elif (defined(_WIN32) || defined(WIN32))
#include <winsock2.h>
#define SE_CONTAINING_RECORD(address, type, field) ((type *)((PCHAR)(address) - (ULONG_PTR)(&((type *)0)->field)))
#define SE_ALIGN(x) __declspec(align(x))
#endif

bool SeCHStrStr(const char* pcDstChar,const char* pcSrcChar);

void SeStrNcpy(char* pcDstChar,int iDstLen,const char* pcSrcChar);

unsigned int SeStr2Hash(const char *pcStr,int iLen);

void * SeMallocMem(size_t size);

void SeFreeMem(void* pvPtr);

#endif



