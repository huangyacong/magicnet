#ifndef __SE_TOOL_H__
#define __SE_TOOL_H__

#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdarg.h>
#include <stdio.h>

bool SeCHStrStr(const char* pcDstChar,const char* pcSrcChar);

void SeStrNcpy(char* pcDstChar,int iDstLen,const char* pcSrcChar);

unsigned int SeStr2Hash(const char *pcStr,int iLen);

void * SeMallocMem(size_t size);

void SeFreeMem(void* pvPtr);

#endif



