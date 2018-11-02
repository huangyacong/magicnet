#ifndef __SE_TIME_H__
#define __SE_TIME_H__

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#ifdef __linux

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif

#include <unistd.h>

#elif (defined(_WIN32) || defined(WIN32))

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <process.h>

#ifdef _USE_32BIT_TIME_T
#error time_t not 64 bit!! dont use _USE_32BIT_TIME_T.
#endif

#endif

enum SE_WEEK
{
	SE_WEEK_SUNDAY = 1,
	SE_WEEK_MONDAY = 2,
	SE_WEEK_TUESDAY = 3,
	SE_WEEK_WEDNESDAY = 4,
	SE_WEEK_THURSDAY = 5,
	SE_WEEK_FRIDAY = 6,
	SE_WEEK_SATURDAY = 7,
};

struct SeTime
{
	int iSec;			/* seconds after the minute - [0,59] */
	int iMin;			/* minutes after the hour - [0,59] */
	int iHour;			/* hours since midnight - [0,23] */
	int iDay;			/* day of the month - [1,31] */
	int iMon;			/* months since January - [1,12] */
	int iYear;			/* years */
	enum SE_WEEK iWDay;	/* days since Sunday - [1,7] */
	int iYDay;			/* days since January 1 - [0,365] */
};

// mysql: datetime->'1971-01-01 00:00:00'to'9999-12-31 23:59:59'; TIMESTAMP->1970to2037
// valid time in '1971-01-01 00:00:00'->'9999-12-31 23:59:59'

bool TestTimeValid();

// now time
time_t SeTimeTime();

// get timeszone
int SeGetTimeZone();

// time_t to struct
struct SeTime SeGetTime(time_t kTime);

// convert string time to time_t, pcTimeChar format to '9999-02-31 23:00:59'
// if pcTimeChar error,return nowtime
time_t SeTimeStringToTime(const char* pcTimeChar);
bool SeTimeCheckStringTime(const char* pcTimeChar);

// return sec
long long SeTimeDiffTime(time_t timeEnd, time_t timeBegin);

// add sec
time_t SeTimeAddTime(time_t srcTime, int sec);

// format to '9999-02-31 23:00:59', len >=20
void SeTimeFormatTime(time_t srcTime, char *pOut, int len);

// format to '9999-02-31', len >=20
void SeTimeFormatDayTime(time_t srcTime, char *pOut, int len);

// format to '23:00:59', len >=9
void SeTimeFormatSecondTime(time_t srcTime, char *pOut, int len);

void SeTimeSleep(unsigned long ulMillisecond);

bool SeIsSameDay(time_t iTimeA, time_t iTimeB);

bool SeIsSameDate(time_t iTimeA, time_t iTimeB);

unsigned long long SeTimeGetTickCount();

#ifdef	__cplusplus
}
#endif

#endif



