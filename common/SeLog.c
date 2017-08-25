#include "SeLog.h"
#include "SeTool.h"
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

void SetLogTime(struct LogTime *pkDate, struct tm *pkTM)
{
	pkDate->m_iHear = pkTM->tm_year;
	pkDate->m_iMon = pkTM->tm_mon;
	pkDate->m_iDay = pkTM->tm_mday;
	pkDate->m_iHour = pkTM->tm_hour;
}

FILE * newFile(const char *pkFileName, int year, int mon, int day)
{
	char fname[512] = "";
	SeSnprintf(fname, (int)sizeof(fname), "%s%04d%02d%02d.log", pkFileName, year, mon, day);
	return fopen(fname, "a");
}

void SeInitLog(struct SELOG *pkLog, const char *pkFileName)
{
	time_t my_time = SeTimeTime();
	struct tm kTM = *localtime(&my_time);

	pkLog->iFlag = 0;
	pkLog->iLockContextFunc = 0;
	pkLog->pkLogContect = 0;
	pkLog->pkLogContextFunc = 0;
	SetLogTime(&pkLog->ttDate, &kTM);
	SeStrNcpy(pkLog->acfname, sizeof(pkLog->acfname), pkFileName);
	pkLog->pFile = newFile(pkLog->acfname, pkLog->ttDate.m_iHear + 1900, pkLog->ttDate.m_iMon + 1, pkLog->ttDate.m_iDay);
	memset(pkLog->actext, 0, sizeof(pkLog->actext));
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
		pkLog->iLockContextFunc = 0;
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
	char acHeadr[128] = {0};
	time_t my_time = SeTimeTime();
	struct tm tt_now = *localtime(&my_time);

	if(!SeHasLogLV(pkLog, iLogLv) || iLogLv == LT_SPLIT || iLogLv == LT_PRINT || pkLog->iLockContextFunc == 1/*lock the callback function*/)
	{
		return;
	}

	maxlen = (int)sizeof(pkLog->actext);

	va_start(argptr, argv);
	writelen = vsnprintf(pkLog->actext, maxlen - 3, argv, argptr);
	va_end(argptr);

	if(writelen <= 0 || writelen > (maxlen - 3))
	{
		return;
	}

	pkLog->actext[writelen + 0] = '\0';
	pkLog->actext[writelen + 1] = '\0';
	pkLog->actext[writelen + 2] = '\0';

	if(iLogLv == LT_ERROR)
	{
		SeSnprintf(acHeadr, (int)sizeof(acHeadr), "%04d-%02d-%02d %02d:%02d:%02d [ERROR] ", tt_now.tm_year + 1900,
			tt_now.tm_mon + 1, tt_now.tm_mday, tt_now.tm_hour, tt_now.tm_min, tt_now.tm_sec);
	}
	else if(iLogLv == LT_WARNING)
	{
		SeSnprintf(acHeadr, (int)sizeof(acHeadr), "%04d-%02d-%02d %02d:%02d:%02d [WARNING] ", tt_now.tm_year + 1900,
			tt_now.tm_mon + 1, tt_now.tm_mday, tt_now.tm_hour, tt_now.tm_min, tt_now.tm_sec);
	}
	else if(iLogLv == LT_INFO)
	{
		SeSnprintf(acHeadr, (int)sizeof(acHeadr), "%04d-%02d-%02d %02d:%02d:%02d [INFO] ", tt_now.tm_year + 1900,
			tt_now.tm_mon + 1, tt_now.tm_mday, tt_now.tm_hour, tt_now.tm_min, tt_now.tm_sec);
	}
	else if(iLogLv == LT_DEBUG)
	{
		SeSnprintf(acHeadr, (int)sizeof(acHeadr), "%04d-%02d-%02d %02d:%02d:%02d [DEBUG] ", tt_now.tm_year + 1900,
			tt_now.tm_mon + 1, tt_now.tm_mday, tt_now.tm_hour, tt_now.tm_min, tt_now.tm_sec);
	}
	else if(iLogLv == LT_CRITICAL)
	{
		SeSnprintf(acHeadr, (int)sizeof(acHeadr), "%04d-%02d-%02d %02d:%02d:%02d [CRITICAL] ", tt_now.tm_year + 1900,
			tt_now.tm_mon + 1, tt_now.tm_mday, tt_now.tm_hour, tt_now.tm_min, tt_now.tm_sec);
	}
	else if(iLogLv == LT_SOCKET)
	{
		SeSnprintf(acHeadr, (int)sizeof(acHeadr), "%04d-%02d-%02d %02d:%02d:%02d [SOCKET] ", tt_now.tm_year + 1900,
			tt_now.tm_mon + 1, tt_now.tm_mday, tt_now.tm_hour, tt_now.tm_min, tt_now.tm_sec);
	}
	else if(iLogLv == LT_NOHEAD)
	{
	}
	else
	{
		SeSnprintf(acHeadr, (int)sizeof(acHeadr), "%04d-%02d-%02d %02d:%02d:%02d ", tt_now.tm_year + 1900,
			tt_now.tm_mon + 1, tt_now.tm_mday, tt_now.tm_hour, tt_now.tm_min, tt_now.tm_sec);
	}

	if(SeHasLogLV(pkLog, LT_SPLIT))
	{
		if(tt_now.tm_year != pkLog->ttDate.m_iHear || tt_now.tm_mon != pkLog->ttDate.m_iMon || tt_now.tm_mday != pkLog->ttDate.m_iDay)
		{
			if (pkLog->pFile) { fflush(pkLog->pFile); fclose(pkLog->pFile); pkLog->pFile = 0; }
			SetLogTime(&pkLog->ttDate, &tt_now);
			pkLog->pFile = newFile(pkLog->acfname, pkLog->ttDate.m_iHear + 1900, pkLog->ttDate.m_iMon + 1, pkLog->ttDate.m_iDay);
		}
	}

	bWrite = true;
	bPrint = SeHasLogLV(pkLog, LT_PRINT);

	if(pkLog->pkLogContextFunc && pkLog->iLockContextFunc == 0)
	{
		pkLog->iLockContextFunc = 1;
		pkLog->pkLogContextFunc(pkLog->pkLogContect, acHeadr, pkLog->actext, iLogLv, &bPrint, &bWrite);
		pkLog->iLockContextFunc = 0;
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

#if defined(__linux)
	pkLog->actext[writelen + 0] = '\n';
	pkLog->actext[writelen + 1] = '\0';
	pkLog->actext[writelen + 2] = '\0';
#elif (defined(_WIN32) || defined(WIN32))
	pkLog->actext[writelen + 0] = '\r';
	pkLog->actext[writelen + 1] = '\n';
	pkLog->actext[writelen + 2] = '\0';
#endif

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

#if defined(SECOLOR)
	WORD wColor = 0x000F;
	if(iLogLv == LT_ERROR)
	{
		wColor = 0x000C;
	}
	if(iLogLv == LT_WARNING)
	{
		wColor = FOREGROUND_GREEN;
	}
	if(iLogLv == LT_CRITICAL)
	{
		wColor = 0x000E;
	}
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), wColor);
#endif

#if ((defined(SENOPRINT) && defined(SEPRINTIM)) || !defined(SENOPRINT))

#if defined(SEPRINTIM)
	if(iLogLv != LT_ERROR && iLogLv != LT_WARNING && iLogLv != LT_CRITICAL)
	{
		return;
	}
#endif

	printf("%s%s\n", pcHeader ? pcHeader : "", pcString ? pcString : "");

#if defined(SECOLOR)
	wColor = 0x000F;
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), wColor);
#endif

#endif

#else

#if defined(SECOLOR)
	#define NONE				"\e[0m\n"
	#define RED					"\e[1;31m"
	#define GREEN				"\e[1;32m"
	#define YELLOW				"\e[1;33m"
#else
	#define NONE				"\n"
	#define RED					""
	#define GREEN 				""
	#define YELLOW				""
#endif

#if ((defined(SENOPRINT) && defined(SEPRINTIM)) || !defined(SENOPRINT))

#if defined(SEPRINTIM)
	if(iLogLv != LT_ERROR && iLogLv != LT_WARNING && iLogLv != LT_CRITICAL)
	{
		return;
	}
#endif

	if(iLogLv == LT_ERROR)
	{
		printf(RED"%s%s"NONE, pcHeader ? pcHeader : "", pcString ? pcString : "");
	}
	else if(iLogLv == LT_WARNING)
	{
		printf(GREEN"%s%s"NONE, pcHeader ? pcHeader : "", pcString ? pcString : "");
	}
	else if(iLogLv == LT_CRITICAL)
	{
		printf(YELLOW"%s%s"NONE, pcHeader ? pcHeader : "", pcString ? pcString : "");
	}
	else
	{
		printf("%s%s"NONE, pcHeader ? pcHeader : "", pcString ? pcString : "");
	}

#endif

#endif
}
