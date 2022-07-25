#include "SeTool.h"
#include "SeTime.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdarg.h>

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

unsigned long long CreateMailID(unsigned short usID, unsigned int uiCount)
{
	unsigned int uiTmpID;
	unsigned short uiMaxID;
	unsigned int iMaxCount;
	unsigned long long tNowTime, ullResult;

	iMaxCount = 1048576;// pow(2, 22);
	uiMaxID = 256; // pow(2, 8);
	uiCount = (unsigned int)uiCount % (unsigned int)iMaxCount;

	if (usID < 0 || usID >= uiMaxID)
		return 0;

	if (uiCount < 0 || uiCount >= iMaxCount)
		return 0;

	tNowTime = SeTimeTime();
	tNowTime = tNowTime << 30;

	uiTmpID = usID;
	uiTmpID = uiTmpID << 22;

	ullResult = tNowTime | uiTmpID | uiCount;
	return (ullResult & 0x7FFFFFFFFFFFFFFF);
}

unsigned long long CreateUniqueID(int iServerID, unsigned int uiCount)
{
	int iMaxSeverID;
	unsigned int iMaxCount;
	unsigned long long tNowTime, ullResult;

	iMaxCount = 4096;// pow(2, 12);
	iMaxSeverID = 262144; // pow(2, 18);
	uiCount = (unsigned int)uiCount % (unsigned int)iMaxCount;

	if (iServerID <= 0 || iServerID >= iMaxSeverID)
		return 0;

	if (uiCount < 0 || uiCount >= iMaxCount)
		return 0;

	tNowTime = SeTimeTime();
	tNowTime = tNowTime << 30;

	iServerID = iServerID << 12;

	ullResult = tNowTime | iServerID | uiCount;
	return (ullResult & 0x7FFFFFFFFFFFFFFF);
}

int GetServerIDByUniqueID(unsigned long long ullUniqueID)
{
	unsigned long long ullRet = ((ullUniqueID >> 12) << 46) >> 46;
	return (int)ullRet;
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
