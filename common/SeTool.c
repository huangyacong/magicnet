#include "SeTool.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdarg.h>

bool SeCHStrStr(const char* pcDstChar, const char* pcSrcChar)
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

void SeStrNcpy(char* pcDstChar, int iDstLen, const char* pcSrcChar)
{
	assert(iDstLen > 1);
	assert(pcDstChar != pcSrcChar);

	if (iDstLen <= 0)
	{
		return;
	}

	if (iDstLen <= 1)
	{
		pcDstChar[0] = '\0';
		return;
	}

	if(pcDstChar == pcSrcChar)
	{
		pcDstChar[0] = '\0';
		return;
	}
	strncpy(pcDstChar,pcSrcChar,iDstLen);
	pcDstChar[iDstLen - 1] = '\0';
}

int SeCopyData(char *dst, int iDstlen, const char *src, int iSrclen)
{
	assert(iSrclen >= 0);
	assert(iDstlen >= 0);

	if(iSrclen <= 0 || iDstlen <= 0)
	{
		return 0;
	}

	if(iDstlen > iSrclen)
	{
		memcpy(dst, src, iSrclen);
		return iSrclen;
	}
	else
	{
		memcpy(dst, src, iDstlen);
		return iDstlen;
	}

	return 0;
}

unsigned int SeStr2Hash(const char *pcStr, int iLen)
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

long long SeAToLongLong(const char *pcString)
{
#if defined(__linux)
	return atoll(pcString);
#elif (defined(_WIN32) || defined(WIN32))
	return _atoi64(pcString);
#endif
}

int SeAToInt(const char *pcString)
{
	return atoi(pcString);
}

double SeAToDouble(const char *pcString)
{
	return atof(pcString);
}

bool SeSnprintf(char *pcStr, int iSize, const char *format, ...)
{
	int writelen;
	va_list argptr;

	if (!pcStr || iSize <= 1)
	{
		return false;
	}

	pcStr[0] = '\0';

	va_start(argptr, format);
	writelen = vsnprintf(pcStr, iSize - 1, format, argptr);
	va_end(argptr);

	if (writelen < 0 || writelen > (iSize - 1))
	{
		pcStr[0] = '\0';
		return false;
	}

	pcStr[iSize - 1] = '\0';
	return true;
}

bool SeLockMem()
{
#if defined(__linux)
	return mlockall(MCL_CURRENT|MCL_FUTURE) == 0 ? true : false;
#endif
	return true;
}

void *SeMallocMem(size_t size)
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
