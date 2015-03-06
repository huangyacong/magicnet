#ifndef __SE_LOG_H__
#define __SE_LOG_H__

#include "SeBool.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#define LT_SPLIT	(1<<0)   // log split
#define LT_ERROR	(1<<1)   // error
#define LT_WARNING	(1<<2)   // warning
#define LT_INFO		(1<<3)   // info
#define LT_DEBUG	(1<<4)   // debug
#define LT_CRITICAL (1<<5)   // CRITICAL
#define LT_SOCKET	(1<<6)   // socket
#define LT_NOTSET	(1<<7)   // NOTSET

struct SELOG
{
	int		iFlag;
	int		iDate;
	FILE*	pFile;
};

void SeInitLog(struct SELOG *pkLog);

void SeFinLog(struct SELOG *pkLog);

bool SeHasLogLV(struct SELOG *pkLog, int iLogLv);

void SeAddLogLV(struct SELOG *pkLog, int iLogLv);

void SeClearLogLV(struct SELOG *pkLog, int iLogLv);

#endif
