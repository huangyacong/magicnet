#include "SeTime.h"
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


bool TestTimeValid()
{
	return sizeof(time_t) == 8;
}

time_t SeTimeTime()
{
	assert(TestTimeValid() == true);

	return time(NULL);
}

int SeGetTimeZone()
{
	int time_zone;
	time_t time_utc;
	struct tm tm_gmt;
	struct tm tm_local;

	assert(TestTimeValid() == true);

	// Get the UTC time
	time(&time_utc);

	// Get the local time
	tm_local = *localtime(&time_utc);

	// Change it to GMT tm
	tm_gmt = *gmtime(&time_utc);

	time_zone = tm_local.tm_hour - tm_gmt.tm_hour;

	if (time_zone < -12)
	{
		time_zone += 24;
	} 
	else if (time_zone > 12)
	{
		time_zone -= 24;
	}

	return time_zone;
}

time_t SeTimeStringToTime(const char* pcTimeChar)
{
	struct tm tb;
	unsigned int uiLen = 0, uiBegin = 0;
	int iYear = 0, iMon = 0, iDay = 0, iHour = 0, iMin = 0, iSec = 0;
	
	assert(TestTimeValid() == true);
	memset(&tb, 0, sizeof(tb));
	uiLen = (unsigned int)strlen(pcTimeChar);

	if(uiLen != 19) {
		return time(NULL);
	}
	
	for(uiBegin = 0; uiBegin < uiLen; uiBegin++)
	{
		// -
		if(uiBegin == 4 || uiBegin == 7)
		{
			if((char)pcTimeChar[uiBegin] != 45)
			{
				return time(NULL);
			}
		}
		// space
		else if(uiBegin == 10)
		{
			if((char)pcTimeChar[uiBegin] != 32)
			{
				return time(NULL);
			}
		}
		// :
		else if(uiBegin == 13 || uiBegin == 16)
		{
			if((char)pcTimeChar[uiBegin] != 58)
			{
				return time(NULL);
			}
		}
		// number(0-9)
		else
		{
			if((char)pcTimeChar[uiBegin] < 48 || (char)pcTimeChar[uiBegin] > 57)
			{
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
	
	if(iYear < 1971 || iYear > 9999)
	{
		return time(NULL);
	}
	if(iMon < 1 || iMon > 12)
	{
		return time(NULL);
	}
	if(iDay < 1 || iDay > 31)
	{
		return time(NULL);
	}
	if(iHour < 0 || iHour > 24)
	{
		return time(NULL);
	}
	if(iMin < 0 || iMin > 60)
	{
		return time(NULL);
	}
	if(iSec < 0 || iSec > 60)
	{
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
	assert(TestTimeValid() == true);
	if(timeEnd < 0)
	{
		timeEnd = SeTimeTime();
	}
	if(timeBegin < 0)
	{ 
		timeBegin = SeTimeTime();
	}
	return (long long)difftime(timeEnd, timeBegin);
}

time_t SeTimeAddTime(time_t srcTime, int sec)
{
	assert(TestTimeValid() == true);
	if(srcTime < 0)
	{
		srcTime = SeTimeTime();
	}
	return srcTime + sec;
}

void SeTimeFormatTime(time_t srcTime, char *pOut, int len)
{
	assert(TestTimeValid() == true);
	if(srcTime < 0)
	{
		srcTime = SeTimeTime();
	}
	strftime(pOut, len, "%Y-%m-%d %H:%M:%S", localtime(&srcTime));
}

void SeTimeFormatDayTime(time_t srcTime, char *pOut, int len)
{
	assert(TestTimeValid() == true);
	if(srcTime < 0)
	{
		srcTime = SeTimeTime();
	}
	strftime(pOut, len, "%Y-%m-%d", localtime(&srcTime));
}

void SeTimeFormatSecondTime(time_t srcTime, char *pOut, int len)
{
	assert(TestTimeValid() == true);
	if(srcTime < 0)
	{
		srcTime = SeTimeTime();
	}
	strftime(pOut, len, "%H:%M:%S", localtime(&srcTime));
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
	unsigned long long ulTime;
	struct timespec kCurrentTime = {0, 0};
	clock_gettime(CLOCK_MONOTONIC, &kCurrentTime);
	ulTime = kCurrentTime.tv_sec * 1000 + kCurrentTime.tv_nsec/(1000 * 1000);
#endif
	return ulTime;
}
bool SeIsSameDay(time_t iTimeA, time_t iTimeB)
{
	struct tm A;
	struct tm B;

	assert(TestTimeValid() == true);
	if (iTimeA < 0 || iTimeB < 0) { return false; }
	memcpy(&A, localtime(&iTimeA), sizeof(A));
	memcpy(&B, localtime(&iTimeB), sizeof(B));
	return A.tm_mday == B.tm_mday;
}
