#include "SeTime.h"

time_t SeTimeTime()
{
	return time(NULL);
}

time_t SeTimeStringToTime(const char* pcTimeChar)
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
	
	if(iYear < 1971 || iYear > 9999) {
		return time(NULL);
	}
	if(iMon < 1 || iMon > 12) {
		return time(NULL);
	}
	if(iDay < 1 || iDay > 31) {
		return time(NULL);
	}
	if(iHour < 0 || iHour > 24) {
		return time(NULL);
	}
	if(iMin < 0 || iMin > 60) {
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

long long SeTimeDiffTime(time_t timeEnd, time_t timeBegin)
{
	return (long long)difftime(timeEnd, timeBegin);
}

time_t SeTimeAddTime(time_t srcTime, int sec)
{
	if(sec >= 3600*24*366 || sec <= 3600*24*366*(-1)) {
		assert(0 != 0);
	}
	return srcTime + sec;
}

void SeTimeFormatTime(time_t srcTime, char *pOut, int len)
{
	strftime(pOut, len, "%Y-%m-%d %H:%M:%S", localtime(&srcTime));
}

void SeTimeSleep(unsigned long ulMillisecond)
{
#if (defined(_WIN32) || defined(WIN32))
	Sleep(ulMillisecond);
#elif defined(__linux)
	usleep(ulMillisecond * 1000);
#endif
}

unsigned long long SeTimeGetTickCount()
{
#if (defined(_WIN32) || defined(WIN32))
	unsigned long long ulTime = GetTickCount64();
#elif defined(__linux)
	struct timespec kCurrentTime = {0, 0};
	clock_gettime(CLOCK_MONOTONIC, &kCurrentTime);
	unsigned long long ulTime = kCurrentTime.tv_sec * 1000 + kCurrentTime.tv_nsec/(1000 * 1000);
#endif
	return ulTime;
}

