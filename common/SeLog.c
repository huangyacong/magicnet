#include "SeLog.h"
#include "SeTool.h"
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

void SetLogTime(struct LogTime *pkDate, const struct tm *pkTM)
{
	pkDate->m_iHear = pkTM->tm_year;
	pkDate->m_iMon = pkTM->tm_mon;
	pkDate->m_iDay = pkTM->tm_mday;
	pkDate->m_iHour = pkTM->tm_hour;
}

bool IsDiffDay(const struct LogTime *pkDate, const struct tm *pkTMTime)
{
	return (pkTMTime->tm_year != pkDate->m_iHear || pkTMTime->tm_mon != pkDate->m_iMon || pkTMTime->tm_mday != pkDate->m_iDay) ? true : false;
}

void SetHeaderString(char *pcHeader, int iHeaderLen, const char *pcLTString, const struct tm *pkTMTime)
{
	SeSnprintf(pcHeader, iHeaderLen, "%04d-%02d-%02d %02d:%02d:%02d%s", pkTMTime->tm_year + 1900,
		pkTMTime->tm_mon + 1, pkTMTime->tm_mday, pkTMTime->tm_hour, pkTMTime->tm_min, pkTMTime->tm_sec, pcLTString);
}

FILE * newFile(const char *pkFileName, const struct LogTime *pkLogTime)
{
	char fname[512] = "";
	SeSnprintf(fname, (int)sizeof(fname), "%s%04d%02d%02d.log", pkFileName, pkLogTime->m_iHear + 1900, pkLogTime->m_iMon + 1, pkLogTime->m_iDay);
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
	pkLog->pFile = newFile(pkLog->acfname, &pkLog->ttDate);
	memset(pkLog->actext, 0, sizeof(pkLog->actext));
}

void SeLogSetLogContextFunc(struct SELOG *pkLog, SELOGCONTEXT pkLogContextFunc, void *pkLogContect)
{
	pkLog->pkLogContect = pkLogContect;
	pkLog->pkLogContextFunc = pkLogContextFunc;
}

void SeFinLog(struct SELOG *pkLog)
{
	if (pkLog->pFile)
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

	if (!SeHasLogLV(pkLog, iLogLv) || iLogLv == LT_SPLIT || iLogLv == LT_PRINT || pkLog->iLockContextFunc == 1/*lock the callback function*/)
	{
		return;
	}

	maxlen = (int)sizeof(pkLog->actext);

	va_start(argptr, argv);
	writelen = vsnprintf(pkLog->actext, maxlen - 3, argv, argptr);
	va_end(argptr);

	if (writelen <= 0 || writelen > (maxlen - 3))
	{
		return;
	}

	pkLog->actext[writelen + 0] = '\0';
	pkLog->actext[writelen + 1] = '\0';
	pkLog->actext[writelen + 2] = '\0';

	switch (iLogLv)
	{
		case LT_ERROR:
		{
			SetHeaderString(acHeadr, (int)sizeof(acHeadr), " [ERROR] ", &tt_now);
			break;
		}
		case LT_WARNING:
		{
			SetHeaderString(acHeadr, (int)sizeof(acHeadr), " [WARNING] ", &tt_now);
			break;
		}
		case LT_INFO:
		{
			SetHeaderString(acHeadr, (int)sizeof(acHeadr), " [INFO] ", &tt_now);
			break;
		}
		case LT_DEBUG:
		{
			SetHeaderString(acHeadr, (int)sizeof(acHeadr), " [DEBUG] ", &tt_now);
			break;
		}
		case LT_CRITICAL:
		{
			SetHeaderString(acHeadr, (int)sizeof(acHeadr), " [CRITICAL] ", &tt_now);
			break;
		}
		case LT_SOCKET:
		{
			SetHeaderString(acHeadr, (int)sizeof(acHeadr), " [SOCKET] ", &tt_now);
			break;
		}
		case LT_NOHEAD:
		{
			break;
		}
		default:
		{
			SetHeaderString(acHeadr, (int)sizeof(acHeadr), "", &tt_now);
			break;
		}
	}

	bWrite = true;
	bPrint = SeHasLogLV(pkLog, LT_PRINT);

	if (pkLog->pkLogContextFunc && pkLog->iLockContextFunc == 0)
	{
		pkLog->iLockContextFunc = 1;
		pkLog->pkLogContextFunc(pkLog->pkLogContect, acHeadr, pkLog->actext, iLogLv, &bPrint, &bWrite);
		pkLog->iLockContextFunc = 0;
	}

	if (bPrint)
	{
		SePrintf(iLogLv, !SeHasLogLV(pkLog, LT_NOHEAD) ? acHeadr : "", pkLog->actext);
	}

	if (SeHasLogLV(pkLog, LT_SPLIT))
	{
		if (IsDiffDay(&pkLog->ttDate, &tt_now))
		{
			if (pkLog->pFile)
			{
				fflush(pkLog->pFile);
				fclose(pkLog->pFile);
				pkLog->pFile = 0;
			}
			SetLogTime(&pkLog->ttDate, &tt_now);
			pkLog->pFile = newFile(pkLog->acfname, &pkLog->ttDate);
		}
	}

	if (!pkLog->pFile || !bWrite)
	{
		return;
	}

	if (!SeHasLogLV(pkLog, LT_NOHEAD))
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

	if (bFlushToDisk)
	{
		fflush(pkLog->pFile);
	}
}

void SeLogFlushToDisk(struct SELOG *pkLog)
{
	if (pkLog->pFile)
	{
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

#ifndef SECOLOR

	printf("%s%s\n", pcHeader ? pcHeader : "", pcString ? pcString : "");
	
#elif (defined(WIN32) || defined(_WIN32))

	#define NONE				0x000F
	#define RED					0x000C
	#define GREEN				0x0002
	#define YELLOW				0x000E

	switch(iLogLv)
	{
		case LT_ERROR:
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), RED);
			printf("%s%s\n", pcHeader ? pcHeader : "", pcString ? pcString : "");
			break;
		}
		case LT_WARNING:
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), GREEN);
			printf("%s%s\n", pcHeader ? pcHeader : "", pcString ? pcString : "");
			break;
		}
		case LT_CRITICAL:
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), YELLOW);
			printf("%s%s\n", pcHeader ? pcHeader : "", pcString ? pcString : "");
			break;
		}
		default:
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), NONE);
			printf("%s%s\n", pcHeader ? pcHeader : "", pcString ? pcString : "");
			break;
		}
	}

#elif defined(__linux)

	#define NONE				"\e[0m\n"
	#define RED					"\e[1;31m"
	#define GREEN				"\e[1;32m"
	#define YELLOW				"\e[1;33m"

	switch (iLogLv)
	{
		case LT_ERROR:
		{
			printf(RED"%s%s"NONE, pcHeader ? pcHeader : "", pcString ? pcString : "");
			break;
		}
		case LT_WARNING:
		{
			printf(GREEN"%s%s"NONE, pcHeader ? pcHeader : "", pcString ? pcString : "");
			break;
		}
		case LT_CRITICAL:
		{
			printf(YELLOW"%s%s"NONE, pcHeader ? pcHeader : "", pcString ? pcString : "");
			break;
		}
		default:
		{
			printf("%s%s"NONE, pcHeader ? pcHeader : "", pcString ? pcString : "");
			break;
		}
	}
#endif

}
