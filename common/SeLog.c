#include "SeLog.h"
#include "SeTool.h"
#include "SeTime.h"
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#define SELOG_MAX_FILE_LEN 1024 * 1024 * 10

bool IsDiffDay(const struct SeTime *pkDate, const struct SeTime *pkTMTime)
{
	return (pkTMTime->iYear != pkDate->iYear || pkTMTime->iMon != pkDate->iMon || pkTMTime->iDay != pkDate->iDay) ? true : false;
}

void SetHeaderString(char *pcHeader, int iHeaderLen, const char *pcLTString, const struct SeTime *pkTMTime)
{
	SeSnprintf(pcHeader, iHeaderLen, "%04d-%02d-%02d %02d:%02d:%02d%s", pkTMTime->iYear,
		pkTMTime->iMon, pkTMTime->iDay, pkTMTime->iHour, pkTMTime->iMin, pkTMTime->iSec, pcLTString);
}

FILE * newFile(const char *pkFileName, const struct SeTime *pkLogTime)
{
	char fname[512] = "";
	SeSnprintf(fname, (int)sizeof(fname), "%s%04d%02d%02d.log", pkFileName, pkLogTime->iYear, pkLogTime->iMon, pkLogTime->iDay);
	return fopen(fname, "a");
}

void SeInitLog(struct SELOG *pkLog, const char *pkFileName)
{
	time_t my_time = SeTimeTime();

	pkLog->iFlag = 0;
	pkLog->iLockContextFunc = 0;
	pkLog->pkLogContect = 0;
	pkLog->pkLogContextFunc = 0;
	pkLog->ttDate = SeGetTime(my_time);
	SeStrNcpy(pkLog->acfname, sizeof(pkLog->acfname), pkFileName);
	pkLog->pFile = newFile(pkLog->acfname, &pkLog->ttDate);
	pkLog->pctext = (char*)SeMallocMem(SELOG_MAX_FILE_LEN);
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
	if (pkLog->pctext)
	{
		SeFreeMem(pkLog->pctext);
	}
	pkLog->pctext = 0;
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
	struct SeTime tt_now = SeGetTime(my_time);

	if (!SeHasLogLV(pkLog, iLogLv) || iLogLv == LT_SPLIT || iLogLv == LT_PRINT || pkLog->iLockContextFunc == 1/*lock the callback function*/)
	{
		return;
	}

	maxlen = SELOG_MAX_FILE_LEN - 1;

	va_start(argptr, argv);
	writelen = vsnprintf(pkLog->pctext, maxlen - 3, argv, argptr);
	va_end(argptr);

	if (writelen <= 0 || writelen > (maxlen - 3))
	{
		return;
	}

	pkLog->pctext[writelen] = '\0';

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
			SetHeaderString(acHeadr, (int)sizeof(acHeadr), " ", &tt_now);
			break;
		}
		case LT_RESERROR:
		{
			SetHeaderString(acHeadr, (int)sizeof(acHeadr), " [RESERROR] ", &tt_now);
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
		pkLog->pkLogContextFunc(pkLog->pkLogContect, acHeadr, pkLog->pctext, iLogLv, &bPrint, &bWrite);
		pkLog->iLockContextFunc = 0;
	}

	if (bPrint)
	{
		SePrintf(iLogLv, acHeadr, pkLog->pctext);
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
			pkLog->ttDate = tt_now;
			pkLog->pFile = newFile(pkLog->acfname, &pkLog->ttDate);
		}
	}

	if (!pkLog->pFile || !bWrite)
	{
		return;
	}

	if (iLogLv != LT_NOHEAD)
	{
		fwrite(acHeadr, 1, strlen(acHeadr), pkLog->pFile);
	}

	pkLog->pctext[writelen] = '\n';
	fwrite(pkLog->pctext, 1, writelen + 1, pkLog->pFile);

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
	char acTime[32];

	if (!pcHeader)
	{
		// format to '9999-02-31 23:00:59', len == 19
		SeTimeFormatTime(SeTimeTime(), acTime, (int)sizeof(acTime));
		acTime[19] = ' ';
		acTime[20] = '\0';
	}
	
#ifndef SECOLOR

	printf("%s%s\n", pcHeader ? pcHeader : acTime, pcString ? pcString : "");
	
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
			printf("%s%s\n", pcHeader ? pcHeader : acTime, pcString ? pcString : "");
			break;
		}
		case LT_WARNING:
		case LT_NOHEAD:
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), GREEN);
			printf("%s%s\n", pcHeader ? pcHeader : acTime, pcString ? pcString : "");
			break;
		}
		case LT_CRITICAL:
		case LT_RESERROR:
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), YELLOW);
			printf("%s%s\n", pcHeader ? pcHeader : acTime, pcString ? pcString : "");
			break;
		}
		default:
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), NONE);
			printf("%s%s\n", pcHeader ? pcHeader : acTime, pcString ? pcString : "");
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
			printf(RED"%s%s"NONE, pcHeader ? pcHeader : acTime, pcString ? pcString : "");
			break;
		}
		case LT_WARNING:
		case LT_NOHEAD:
		{
			printf(GREEN"%s%s"NONE, pcHeader ? pcHeader : acTime, pcString ? pcString : "");
			break;
		}
		case LT_CRITICAL:
		case LT_RESERROR:
		{
			printf(YELLOW"%s%s"NONE, pcHeader ? pcHeader : acTime, pcString ? pcString : "");
			break;
		}
		default:
		{
			printf("%s%s"NONE, pcHeader ? pcHeader : acTime, pcString ? pcString : "");
			break;
		}
	}
#endif

}
