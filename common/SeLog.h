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

#define LT_SPLIT	(1<<0)   // log split by diff day
#define LT_ERROR	(1<<1)   // error
#define LT_WARNING	(1<<2)   // warning
#define LT_INFO		(1<<3)   // info
#define LT_DEBUG	(1<<4)   // debug
#define LT_CRITICAL (1<<5)   // CRITICAL
#define LT_SOCKET	(1<<6)   // socket
#define LT_NOTSET	(1<<7)   // NOTSET
#define LT_PRINT	(1<<8)   // print log to screen

struct SELOG
{
	int			iFlag;
	FILE*		pFile;
	struct tm	ttDate;
	char		acfname[128];
	char		actext[4096];
};

void SeInitLog(struct SELOG *pkLog, char *pkFileName);

void SeFinLog(struct SELOG *pkLog);

void SeLogWrite(struct SELOG *pkLog, int iLogLv, bool bFlushToDisk, const char *argv, ...);

void SeLogFlushToDisk(struct SELOG *pkLog);

bool SeHasLogLV(struct SELOG *pkLog, int iLogLv);

void SeAddLogLV(struct SELOG *pkLog, int iLogLv);

void SeClearLogLV(struct SELOG *pkLog, int iLogLv);

#ifdef	__cplusplus
}
#endif

#endif
