#ifndef __SE_TOOL_H__
#define __SE_TOOL_H__

#include "SeBool.h"
#include <string.h>
#include <assert.h>

bool SeCHStrStr(const char* pcDstChar,const char* pcSrcChar);

void SeStrNcpy(char* pcDstChar,int iDstLen,const char* pcSrcChar);

unsigned int SeStr2Hash(const char *pcStr,int iLen);

#endif



