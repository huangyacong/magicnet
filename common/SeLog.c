#include "SeLog.h"
#include "SeTool.h"
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

FILE * newFile(const char *pkFileName, int year, int mon, int day)
{
	char fname[2048] = "";
	sprintf(fname, "%s%04d%02d%02d.log", pkFileName, year, mon, day);
	return fopen(fname, "a");
}

void SeInitLog(struct SELOG *pkLog, char *pkFileName)
{
	time_t my_time = SeTimeTime();

	pkLog->iFlag = 0;
	memcpy(&pkLog->ttDate, localtime(&my_time), sizeof(pkLog->ttDate));
	SeStrNcpy(pkLog->acfname, sizeof(pkLog->acfname), pkFileName);
	memset(pkLog->actext, 0, sizeof(pkLog->actext));
	
	pkLog->pFile = newFile(pkLog->acfname, pkLog->ttDate.tm_year + 1900, pkLog->ttDate.tm_mon + 1, pkLog->ttDate.tm_mday);
}

void SeFinLog(struct SELOG *pkLog)
{
	if(pkLog->pFile)
	{
		fclose(pkLog->pFile);
		pkLog->pFile = 0;
	}
}

void SeLogWrite(struct SELOG *pkLog, int iLogLv, bool bFlushToDisk, char *argv, ...)
{
	int writelen;
	int maxlen;
	va_list argptr;
	struct tm tt_now;
	char acHeadr[128] = {0};
	time_t my_time = SeTimeTime();
	memcpy(&tt_now, localtime(&my_time), sizeof(tt_now));

	if(!SeHasLogLV(pkLog, iLogLv))
	{
		return;
	}

	maxlen = (int)sizeof(pkLog->actext);

	va_start(argptr, argv);
	writelen = vsnprintf(pkLog->actext, maxlen - 3, argv, argptr);
	va_end(argptr);

	if(writelen <= 0)
	{
		return;
	}

#if defined(__linux)
	if(writelen <= (maxlen - 3))
	{
		pkLog->actext[writelen + 0] = '\n';
		pkLog->actext[writelen + 1] = '\0';
		pkLog->actext[writelen + 2] = '\0';
	}
#elif (defined(_WIN32) || defined(WIN32))
	if (writelen <= (maxlen - 3))
	{
		pkLog->actext[writelen + 0] = '\r';
		pkLog->actext[writelen + 1] = '\n';
		pkLog->actext[writelen + 2] = '\0';
	}
#endif

	if(iLogLv == LT_ERROR)
	{
		sprintf(acHeadr, "%04d-%02d-%02d %02d:%02d:%02d [ERROR] ", tt_now.tm_year + 1900, 
			tt_now.tm_mon + 1, tt_now.tm_mday, tt_now.tm_hour, tt_now.tm_min, tt_now.tm_sec);
	}
	else if(iLogLv == LT_WARNING)
	{
		sprintf(acHeadr, "%04d-%02d-%02d %02d:%02d:%02d [WARNING] ", tt_now.tm_year + 1900, 
			tt_now.tm_mon + 1, tt_now.tm_mday, tt_now.tm_hour, tt_now.tm_min, tt_now.tm_sec);
	}
	else if(iLogLv == LT_INFO)
	{
		sprintf(acHeadr, "%04d-%02d-%02d %02d:%02d:%02d [INFO] ", tt_now.tm_year + 1900, 
			tt_now.tm_mon + 1, tt_now.tm_mday, tt_now.tm_hour, tt_now.tm_min, tt_now.tm_sec);
	}
	else if(iLogLv == LT_DEBUG)
	{
		sprintf(acHeadr, "%04d-%02d-%02d %02d:%02d:%02d [DEBUG] ", tt_now.tm_year + 1900, 
			tt_now.tm_mon + 1, tt_now.tm_mday, tt_now.tm_hour, tt_now.tm_min, tt_now.tm_sec);
	}
	else if(iLogLv == LT_CRITICAL)
	{
		sprintf(acHeadr, "%04d-%02d-%02d %02d:%02d:%02d [CRITICAL] ", tt_now.tm_year + 1900, 
			tt_now.tm_mon + 1, tt_now.tm_mday, tt_now.tm_hour, tt_now.tm_min, tt_now.tm_sec);
	}
	else if(iLogLv == LT_SOCKET)
	{
		sprintf(acHeadr, "%04d-%02d-%02d %02d:%02d:%02d [SOCKET] ", tt_now.tm_year + 1900, 
			tt_now.tm_mon + 1, tt_now.tm_mday, tt_now.tm_hour, tt_now.tm_min, tt_now.tm_sec);
	}
	else if(iLogLv == LT_NOTSET)
	{
		sprintf(acHeadr, "%04d-%02d-%02d %02d:%02d:%02d [NOTSET] ", tt_now.tm_year + 1900, 
			tt_now.tm_mon + 1, tt_now.tm_mday, tt_now.tm_hour, tt_now.tm_min, tt_now.tm_sec);
	}
	else
	{
		sprintf(acHeadr, "%04d-%02d-%02d %02d:%02d:%02d ", tt_now.tm_year + 1900, 
			tt_now.tm_mon + 1, tt_now.tm_mday, tt_now.tm_hour, tt_now.tm_min, tt_now.tm_sec);
	}

	if(SeHasLogLV(pkLog, LT_PRINT))
	{
		printf("%s%s", acHeadr, pkLog->actext);
	}

	if(SeHasLogLV(pkLog, LT_SPLIT))
	{
		if(tt_now.tm_year != pkLog->ttDate.tm_year || tt_now.tm_mon != pkLog->ttDate.tm_mon || tt_now.tm_mday != pkLog->ttDate.tm_mday)
		{
			if(pkLog->pFile) { fclose(pkLog->pFile); pkLog->pFile = 0;}
			pkLog->ttDate.tm_year = tt_now.tm_year;
			pkLog->ttDate.tm_mon = tt_now.tm_mon;
			pkLog->ttDate.tm_mday = tt_now.tm_mday;
			pkLog->pFile = newFile(pkLog->acfname, pkLog->ttDate.tm_year + 1900, pkLog->ttDate.tm_mon + 1, pkLog->ttDate.tm_mday);
		}
	}

	if(!pkLog->pFile)
	{
		return;
	}
	
	fwrite(acHeadr, 1, strlen(acHeadr), pkLog->pFile);
	fwrite(pkLog->actext, 1, strlen(pkLog->actext), pkLog->pFile);
	if(bFlushToDisk) {
		fflush(pkLog->pFile);
	}
}

void SeLogFlushToDisk(struct SELOG *pkLog)
{
	if(pkLog->pFile) {
		fflush(pkLog->pFile);
	}
}

bool SeHasLogLV(struct SELOG *pkLog, int iLogLv)
{
	return (((pkLog->iFlag) & iLogLv) == iLogLv ? true : false);
}

void SeAddLogLV(struct SELOG *pkLog, int iLogLv)
{
	pkLog->iFlag |= iLogLv;
}

void SeClearLogLV(struct SELOG *pkLog, int iLogLv)
{
	pkLog->iFlag &= ~iLogLv;
}
