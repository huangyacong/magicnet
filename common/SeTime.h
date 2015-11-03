#ifndef __SE_TIME_H__
#define __SE_TIME_H__

#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#ifdef __linux

#define _XOPEN_SOURCE
#include <unistd.h>

#elif (defined(_WIN32) || defined(WIN32))

#define   WIN32_LEAN_AND_MEAN 
#include <windows.h>
#include <process.h>

#endif

// mysql: datetime->'1971-01-01 00:00:00'to'9999-12-31 23:59:59'; TIMESTAMP->1970to2037
// valid time in '1971-01-01 00:00:00'->'9999-12-31 23:59:59'

// now time
time_t SeTimeTime();

// convert string time to time_t, pcTimeChar format to '9999-02-31 23:00:59'
// if pcTimeChar error,return nowtime
time_t SeTimeStringToTime(const char* pcTimeChar);

// return sec
long long SeTimeDiffTime(time_t timeEnd, time_t timeBegin);

// add sec
time_t SeTimeAddTime(time_t srcTime, int sec);

// format to '9999-02-31 23:00:59', len >=20
void SeTimeFormatTime(time_t srcTime, char *pOut, int len);

void SeTimeSleep(unsigned long ulMillisecond);

unsigned long long SeTimeGetTickCount();

#endif



