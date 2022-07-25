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
	time_utc = SeTimeTime();

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

struct SeTime SeGetTime(time_t kTime)
{
	struct tm tt_now;
	struct SeTime kSeTime;

	kTime = kTime < 0 ? SeTimeTime() : kTime;
	tt_now = *localtime(&kTime);

	kSeTime.iSec = tt_now.tm_sec;
	kSeTime.iMin = tt_now.tm_min;
	kSeTime.iHour = tt_now.tm_hour;
	kSeTime.iDay = tt_now.tm_mday;
	kSeTime.iMon = tt_now.tm_mon + 1;
	kSeTime.iYear = tt_now.tm_year + 1900;
	kSeTime.iWDay = (enum SE_WEEK)(tt_now.tm_wday + 1);
	kSeTime.iYDay = tt_now.tm_yday;
	return kSeTime;
}

bool SeTimeCheckStringTime(const char* pcTimeChar)
{
	unsigned int uiLen = 0, uiBegin = 0;
	int iYear = 0, iMon = 0, iDay = 0, iHour = 0, iMin = 0, iSec = 0;

	if (!pcTimeChar)
		return false;
	assert(TestTimeValid() == true);
	uiLen = (unsigned int)strlen(pcTimeChar);

	if (uiLen != 19) {
		return false;
	}

	for (uiBegin = 0; uiBegin < uiLen; uiBegin++)
	{
		// -
		if (uiBegin == 4 || uiBegin == 7)
		{
			if ((char)pcTimeChar[uiBegin] != 45)
			{
				return false;
			}
		}
		// space
		else if (uiBegin == 10)
		{
			if ((char)pcTimeChar[uiBegin] != 32)
			{
				return false;
			}
		}
		// :
		else if (uiBegin == 13 || uiBegin == 16)
		{
			if ((char)pcTimeChar[uiBegin] != 58)
			{
				return false;
			}
		}
		// number(0-9)
		else
		{
			if ((char)pcTimeChar[uiBegin] < 48 || (char)pcTimeChar[uiBegin] > 57)
			{
				return false;
			}
		}
	}

	iYear = atoi(pcTimeChar);
	iMon = atoi(pcTimeChar + 5);
	iDay = atoi(pcTimeChar + 8);
	iHour = atoi(pcTimeChar + 11);
	iMin = atoi(pcTimeChar + 14);
	iSec = atoi(pcTimeChar + 17);

	if (iYear < 1971 || iYear > 9999)
	{
		return false;
	}
	if (iMon < 1 || iMon > 12)
	{
		return false;
	}
	if (iDay < 1 || iDay > 31)
	{
		return false;
	}
	if (iHour < 0 || iHour > 24)
	{
		return false;
	}
	if (iMin < 0 || iMin > 60)
	{
		return false;
	}
	if (iSec < 0 || iSec > 60)
	{
		return false;
	}

	return true;
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
		return SeTimeTime();
	}
	
	for(uiBegin = 0; uiBegin < uiLen; uiBegin++)
	{
		// -
		if(uiBegin == 4 || uiBegin == 7)
		{
			if((char)pcTimeChar[uiBegin] != 45)
			{
				return SeTimeTime();
			}
		}
		// space
		else if(uiBegin == 10)
		{
			if((char)pcTimeChar[uiBegin] != 32)
			{
				return SeTimeTime();
			}
		}
		// :
		else if(uiBegin == 13 || uiBegin == 16)
		{
			if((char)pcTimeChar[uiBegin] != 58)
			{
				return SeTimeTime();
			}
		}
		// number(0-9)
		else
		{
			if((char)pcTimeChar[uiBegin] < 48 || (char)pcTimeChar[uiBegin] > 57)
			{
				return SeTimeTime();
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
		return SeTimeTime();
	}
	if(iMon < 1 || iMon > 12)
	{
		return SeTimeTime();
	}
	if(iDay < 1 || iDay > 31)
	{
		return SeTimeTime();
	}
	if(iHour < 0 || iHour > 24)
	{
		return SeTimeTime();
	}
	if(iMin < 0 || iMin > 60)
	{
		return SeTimeTime();
	}
	if(iSec < 0 || iSec > 60)
	{
		return SeTimeTime();
	}

	tb.tm_year = iYear - 1900;
	tb.tm_mon = iMon - 1;
	tb.tm_mday = iDay;
	tb.tm_hour = iHour;
	tb.tm_min = iMin;
	tb.tm_sec = iSec;
	tb.tm_isdst = -1;

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

bool SeIsSameDate(time_t iTimeA, time_t iTimeB)
{
	struct tm A;
	struct tm B;

	assert(TestTimeValid() == true);
	if (iTimeA < 0 || iTimeB < 0) { return false; }
	memcpy(&A, localtime(&iTimeA), sizeof(A));
	memcpy(&B, localtime(&iTimeB), sizeof(B));
	return ((A.tm_year == B.tm_year) && (A.tm_mon == B.tm_mon) && (A.tm_mday == B.tm_mday));
}
