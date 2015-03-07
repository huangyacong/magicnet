#ifndef __SE_TOOL_H__
#define __SE_TOOL_H__

#include "SeBool.h"
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

// mysql: datetime->'1000-01-01 00:00:00'to'9999-12-31 23:59:59'; TIMESTAMP->1970to2037



// convert string time to time_t, pcTimeChar format to '9999-02-31 23:00:59'
// if pcTimeChar error,return nowtime
time_t string_to_time_t(const char* pcTimeChar);

bool SeCHStrStr(const char* pcDstChar,const char* pcSrcChar);

void SeStrNcpy(char* pcDstChar,int iDstLen,const char* pcSrcChar);

unsigned int SeStr2Hash(const char *pcStr,int iLen);

void * SeMallocMem(size_t size);

void SeFreeMem(void* pvPtr);

#endif



