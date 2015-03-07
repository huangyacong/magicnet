#include "SeTool.h"

time_t newtime_t()
{
	return time(NULL);
}

time_t string_to_time_t(const char* pcTimeChar)
{
	if(strlen(pcTimeChar) != 19) {
		return time(NULL);
	}

	// -
	if((char)pcTimeChar[4] != 45 || (char)pcTimeChar[7] != 45) {
		return time(NULL);
	}
	// space
	if((char)pcTimeChar[10] != 32) {
		return time(NULL);
	}
	// :
	if((char)pcTimeChar[13] != 58 || (char)pcTimeChar[16] != 58) {
		return time(NULL);
	}

	struct tm tb;
	if(strptime(pcTimeChar, "%Y-%m-%d %H:%M:%S", &tb) == 0) {
		return time(NULL);
	}
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
	return memalign(16, size);
}

void SeFreeMem(void* pvPtr)
{
	free(pvPtr);
}
