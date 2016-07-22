#include "SeTool.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#define SEALIGNMENT 64

bool SeCHStrStr(const char* pcDstChar,const char* pcSrcChar)
{
	int iLen = 0;
	int iDstIndex = 0;

	if(!pcDstChar || !pcSrcChar)
	{
		return false;
	}
	if(strlen(pcDstChar) <= 0 && strlen(pcSrcChar) <= 0)
	{
		return true;
	}
	if(strlen(pcDstChar) > 0 && strlen(pcSrcChar) <= 0)
	{
		return false;
	}

	iLen = (int)strlen(pcDstChar);
	for(iDstIndex = 0; iDstIndex < iLen;)
	{
		const char* pcTmpDst = &pcDstChar[iDstIndex];
		const char* pcTmpSrc = pcSrcChar;

		while(*pcTmpDst == *pcTmpSrc && pcTmpDst != '\0' && pcTmpSrc != '\0')
		{
			pcTmpDst++;
			pcTmpSrc++;
		}

		if(*pcTmpSrc == '\0')
		{
			return true;
		}

		// not ascii
		if((unsigned char)pcDstChar[iDstIndex] >= 0x80)
		{
			iDstIndex += 2;
		}
		else
		{
			iDstIndex++;
		}
	}

	return false;
}

void SeStrNcpy(char* pcDstChar,int iDstLen,const char* pcSrcChar)
{
	assert(iDstLen > 1);
	assert(pcDstChar != pcSrcChar);
	if(pcDstChar == pcSrcChar)
	{
		return;
	}
	strncpy(pcDstChar,pcSrcChar,iDstLen);
	pcDstChar[iDstLen - 1] = '\0';
}

unsigned int SeStr2Hash(const char *pcStr,int iLen)
{
	unsigned int i = 0;
	unsigned int uiResult = iLen;
	unsigned int uiStep = (iLen>>5) + 1;
	assert(iLen > 0);
	
	for(i = iLen;i >= uiStep;i -= uiStep)
	{
		uiResult = uiResult ^ ((uiResult<<5) + (uiResult>>2) + (unsigned int)pcStr[i - 1]);
	}

	return uiResult;
}

void * SeMallocMem(size_t size)
{
#if defined(__linux)
	return memalign(SEALIGNMENT, size);
#elif (defined(_WIN32) || defined(WIN32))
	return _aligned_malloc(size, SEALIGNMENT);
#endif
}

void SeFreeMem(void* pvPtr)
{
#if defined(__linux)
	free(pvPtr);
#elif (defined(_WIN32) || defined(WIN32))
	_aligned_free(pvPtr);
#endif
}
