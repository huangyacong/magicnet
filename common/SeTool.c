#include "SeTool.h"

time_t newtime_t()
{
	return time(NULL);
}

time_t string_to_time_t(const char* pcTimeChar)
{
	struct tm tb;
	unsigned int uiLen = 0, uiBegin = 0;
	int iYear = 0, iMon = 0, iDay = 0, iHour = 0, iMin = 0, iSec = 0;
	
	memset(&tb, 0, sizeof(tb));
	uiLen = strlen(pcTimeChar);

	if(uiLen != 19) {
		return time(NULL);
	}
	
	for(uiBegin = 0; uiBegin < uiLen; uiBegin++) {
		// -
		if(uiBegin == 4 || uiBegin == 7) {
			if((char)pcTimeChar[uiBegin] != 45) {
				return time(NULL);
			}
		}
		// space
		else if(uiBegin == 10) {
			if((char)pcTimeChar[uiBegin] != 32) {
				return time(NULL);
			}
		}
		// :
		else if(uiBegin == 13 || uiBegin == 16) {
			if((char)pcTimeChar[uiBegin] != 58) {
				return time(NULL);
			}
		}
		// number(0-9)
		else {
			if((char)pcTimeChar[uiBegin] < 48 || (char)pcTimeChar[uiBegin] > 57) {
				return time(NULL);
			}
		}
	}

	iYear = atoi(pcTimeChar);
	iMon = atoi(pcTimeChar + 5);
	iDay = atoi(pcTimeChar + 8);
	iHour = atoi(pcTimeChar + 11);
	iMin = atoi(pcTimeChar + 14);
	iSec = atoi(pcTimeChar + 17);
	
	if(iYear < 1900 || iYear > 9999) {
		return time(NULL);
	}
	if(iMon < 1 || iMon > 12) {
		return time(NULL);
	}
	if(iDay < 1 || iDay > 31) {
		return time(NULL);
	}
	if(iHour < 0 || iHour > 23) {
		return time(NULL);
	}
	if(iMin < 0 || iMin > 59) {
		return time(NULL);
	}
	if(iSec < 0 || iSec > 60) {
		return time(NULL);
	}
	
	tb.tm_year = iYear - 1900;
	tb.tm_mon = iMon - 1;
	tb.tm_mday = iDay;
	tb.tm_hour = iHour;
	tb.tm_min = iMin;
	tb.tm_sec = iSec;
	
	return mktime(&tb);
}

long long difftime_t(time_t timeEnd, time_t timeBegin)
{
	return (long long)difftime(timeEnd, timeBegin);
}

time_t addtime_t(time_t srcTime, unsigned int sec)
{
	return srcTime + sec;
}

void formattime_t(time_t srcTime, char *pOut, int len)
{
	strftime(pOut, len, "%Y-%m-%d %H:%M:%S", localtime(&srcTime));
}

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
#ifdef __linux
	return memalign(16, size);
#elif (defined(_WIN32) || defined(WIN32))
	return malloc(size);
#endif
}

void SeFreeMem(void* pvPtr)
{
	free(pvPtr);
}
