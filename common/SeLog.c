#include "SeLog.h"
#include "SeTool.h"

FILE * newFile(const char *pkFileName, int year, int mon, int day)
{
	char fname[2048] = "";
	sprintf(fname, "%s%04d%02d%02d.log", pkFileName, year, mon, day);
	return fopen(fname, "a");
}

void SeInitLog(struct SELOG *pkLog, char *pkFileName)
{
	time_t my_time = time(NULL);

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

void SeLogWrite(struct SELOG *pkLog, int iLogLv, const char *text)
{
	if(!pkLog->pFile)
	{
		return;
	}

	struct tm tt_now;
	time_t my_time = time(NULL);
	memcpy(&tt_now, localtime(&my_time), sizeof(tt_now));
	
	if(SeHasLogLV(pkLog, LT_SPLIT))
	{
		if(tt_now.tm_year != pkLog->ttDate.tm_year || tt_now.tm_mon != pkLog->ttDate.tm_mon || tt_now.tm_mday != pkLog->ttDate.tm_mday)
		{
			fclose(pkLog->pFile);

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

	if(iLogLv == LT_ERROR)
	{
		sprintf(pkLog->actext, "%04d-%02d-%02d %02d:%02d:%02d [LOG ERROR] %s\n", tt_now.tm_year + 1900, 
			tt_now.tm_mon + 1, tt_now.tm_mday, tt_now.tm_hour, tt_now.tm_min, tt_now.tm_sec, text);
	}
	else if(iLogLv == LT_WARNING)
	{
		sprintf(pkLog->actext, "%04d-%02d-%02d %02d:%02d:%02d [LOG WARNING] %s\n", tt_now.tm_year + 1900, 
			tt_now.tm_mon + 1, tt_now.tm_mday, tt_now.tm_hour, tt_now.tm_min, tt_now.tm_sec, text);
	}
	else if(iLogLv == LT_INFO)
	{
		sprintf(pkLog->actext, "%04d-%02d-%02d %02d:%02d:%02d [LOG INFO] %s\n", tt_now.tm_year + 1900, 
			tt_now.tm_mon + 1, tt_now.tm_mday, tt_now.tm_hour, tt_now.tm_min, tt_now.tm_sec, text);
	}
	else if(iLogLv == LT_DEBUG)
	{
		sprintf(pkLog->actext, "%04d-%02d-%02d %02d:%02d:%02d [LOG DEBUG] %s\n", tt_now.tm_year + 1900, 
			tt_now.tm_mon + 1, tt_now.tm_mday, tt_now.tm_hour, tt_now.tm_min, tt_now.tm_sec, text);
	}
	else if(iLogLv == LT_CRITICAL)
	{
		sprintf(pkLog->actext, "%04d-%02d-%02d %02d:%02d:%02d [LOG CRITICAL] %s\n", tt_now.tm_year + 1900, 
			tt_now.tm_mon + 1, tt_now.tm_mday, tt_now.tm_hour, tt_now.tm_min, tt_now.tm_sec, text);
	}
	else if(iLogLv == LT_SOCKET)
	{
		sprintf(pkLog->actext, "%04d-%02d-%02d %02d:%02d:%02d [LOG SOCKET] %s\n", tt_now.tm_year + 1900, 
			tt_now.tm_mon + 1, tt_now.tm_mday, tt_now.tm_hour, tt_now.tm_min, tt_now.tm_sec, text);
	}
	else if(iLogLv == LT_NOTSET)
	{
		sprintf(pkLog->actext, "%04d-%02d-%02d %02d:%02d:%02d [LOG NOTSET] %s\n", tt_now.tm_year + 1900, 
			tt_now.tm_mon + 1, tt_now.tm_mday, tt_now.tm_hour, tt_now.tm_min, tt_now.tm_sec, text);
	}
	else
	{
		sprintf(pkLog->actext, "%04d-%02d-%02d %02d:%02d:%02d %s\n", tt_now.tm_year + 1900, 
			tt_now.tm_mon + 1, tt_now.tm_mday, tt_now.tm_hour, tt_now.tm_min, tt_now.tm_sec, text);
	}
	
	fwrite(pkLog->actext, 1, strlen(pkLog->actext), pkLog->pFile);
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
