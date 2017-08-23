#ifndef __SE_LOG_H__
#define __SE_LOG_H__

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "SeTime.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#define LT_SPLIT	(1<<0)	// log split by diff day
#define LT_PRINT	(1<<1)	// print log to screen
#define LT_ERROR	(1<<2)	// error
#define LT_WARNING	(1<<3)	// warning
#define LT_INFO		(1<<4)	// info
#define LT_DEBUG	(1<<5)	// debug
#define LT_CRITICAL (1<<6)	// CRITICAL
#define LT_SOCKET	(1<<7)	// socket
#define LT_NOHEAD	(1<<8)	// not print header

typedef void(*SELOGCONTEXT)(void *pkLogContect, const char* pcHeader, const char* pcContext, int iLogLv, bool *rbPrint, bool *rbWrite);

struct LogTime
{
	int m_iHear;
	int m_iMon;
	int m_iDay;
	int m_iHour;
	int iTagA;
	int iTagB;
	int iTagC;
	int iTagD;
};

struct SELOG
{
	int			iFlag;
	int			iLockContextFunc;
	void*		pkLogContect;
	SELOGCONTEXT pkLogContextFunc;
	FILE*		pFile;
	struct LogTime ttDate;

	char		acfname[128];
	char		actext[1024*10];
};

void SeInitLog(struct SELOG *pkLog, const char *pkFileName);

void SeFinLog(struct SELOG *pkLog);

void SeLogSetLogContextFunc(struct SELOG *pkLog, SELOGCONTEXT pkLogContextFunc, void *pkLogContect);

void SeLogWrite(struct SELOG *pkLog, int iLogLv, bool bFlushToDisk, const char *argv, ...);

void SeLogFlushToDisk(struct SELOG *pkLog);

bool SeHasLogLV(struct SELOG *pkLog, int iLogLv);

void SeAddLogLV(struct SELOG *pkLog, int iLogLv);

void SeClearLogLV(struct SELOG *pkLog, int iLogLv);

void SePrintf(int iLogLv, const char *pcHeader, const char *pcString);

#ifdef	__cplusplus
}
#endif

#endif
