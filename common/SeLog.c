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
	pkLog->pkLogContect = 0;
	pkLog->pkLogContextFunc = 0;
	memcpy(&pkLog->ttDate, localtime(&my_time), sizeof(pkLog->ttDate));
	SeStrNcpy(pkLog->acfname, sizeof(pkLog->acfname), pkFileName);
	memset(pkLog->actext, 0, sizeof(pkLog->actext));
	
	pkLog->pFile = newFile(pkLog->acfname, pkLog->ttDate.tm_year + 1900, pkLog->ttDate.tm_mon + 1, pkLog->ttDate.tm_mday);
}

void SeLogSetLogContextFunc(struct SELOG *pkLog, SELOGCONTEXT pkLogContextFunc, void *pkLogContect)
{
	pkLog->pkLogContect = pkLogContect;
	pkLog->pkLogContextFunc = pkLogContextFunc;
}

void SeFinLog(struct SELOG *pkLog)
{
	if(pkLog->pFile)
	{
		fclose(pkLog->pFile);
		pkLog->iFlag = 0;
		pkLog->pFile = 0;
		pkLog->pkLogContect = 0;
		pkLog->pkLogContextFunc = 0;
	}
}

void SeLogWrite(struct SELOG *pkLog, int iLogLv, bool bFlushToDisk, const char *argv, ...)
{
	bool bPrint;
	bool bWrite;
	int writelen;
	int maxlen;
	va_list argptr;
	struct tm tt_now;
	char acHeadr[128] = {0};
	time_t my_time = SeTimeTime();
	memcpy(&tt_now, localtime(&my_time), sizeof(tt_now));

	if(!SeHasLogLV(pkLog, iLogLv) || iLogLv == LT_SPLIT || iLogLv == LT_PRINT)
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
	else if(iLogLv == LT_NOHEAD)
	{
	}
	else
	{
		sprintf(acHeadr, "%04d-%02d-%02d %02d:%02d:%02d ", tt_now.tm_year + 1900, 
			tt_now.tm_mon + 1, tt_now.tm_mday, tt_now.tm_hour, tt_now.tm_min, tt_now.tm_sec);
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

	bWrite = true;
	bPrint = SeHasLogLV(pkLog, LT_PRINT);

	if(pkLog->pkLogContextFunc)
	{
		pkLog->pkLogContextFunc(pkLog->pkLogContect, acHeadr, pkLog->actext, iLogLv, &bPrint, &bWrite);
	}

	if(SeHasLogLV(pkLog, LT_PRINT) && bPrint)
	{
		SePrintf(iLogLv, !SeHasLogLV(pkLog, LT_NOHEAD) ? acHeadr : "", pkLog->actext);
	}

	if(!pkLog->pFile || !bWrite)
	{
		return;
	}

	if(!SeHasLogLV(pkLog, LT_NOHEAD))
	{
		fwrite(acHeadr, 1, strlen(acHeadr), pkLog->pFile);
	}

	fwrite(pkLog->actext, 1, strlen(pkLog->actext), pkLog->pFile);

	if(bFlushToDisk)
	{
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

void SePrintf(int iLogLv, const char *pcHeader, const char *pcString)
{
#if (defined(WIN32) || defined(_WIN32))
	WORD wColor = 0x000F;
	if (iLogLv == LT_ERROR)
	{
		wColor = 0x000C;
	}
	if (iLogLv == LT_WARNING)
	{
		wColor = FOREGROUND_GREEN;
	}
	if (iLogLv == LT_CRITICAL)
	{
		wColor = 0x000E;
	}
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), wColor);
#endif
	printf("%s%s", pcHeader ? pcHeader : "", pcString ? pcString : "");
}
