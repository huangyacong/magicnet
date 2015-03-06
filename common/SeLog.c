#include "SeLog.h"
#include "SeTool.h"

void SeInitLog(struct SELOG *pkLog, char *pkFileName)
{
	pkLog->iFlag = 0;
	time_t my_time = time(NULL);
	memcpy(&pkLog->ttDate, localtime(&my_time), sizeof(pkLog->ttDate));
	SeStrNcpy(pkLog->acfname, sizeof(pkLog->acfname), pkFileName);
	
	char fname[2048] = "";
	sprintf(fname, "%s%04d%02d%02d.log", pkLog->acfname, pkLog->ttDate.tm_year + 1900, 
			pkLog->ttDate.tm_mon + 1, pkLog->ttDate.tm_mday);
	pkLog->pFile = fopen(fname, "a");
	memset(pkLog->actext, 0, sizeof(pkLog->actext));
}

void SeFinLog(struct SELOG *pkLog)
{
	if(pkLog->pFile)
	{
		fclose(pkLog->pFile);
		pkLog->pFile = 0;
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

void SeLogWrite(struct SELOG *pkLog, int iLogLv, const char *text)
{
	if(!SeHasLogLV(pkLog, iLogLv) || !pkLog->pFile)
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
			pkLog->ttDate.tm_year = tt_now.tm_year;
			pkLog->ttDate.tm_mon = tt_now.tm_mon;
			pkLog->ttDate.tm_mday = tt_now.tm_mday;
			fclose(pkLog->pFile);

			char fname[2048] = "";
			sprintf(fname, "%s%04d%02d%02d.log", pkLog->acfname, pkLog->ttDate.tm_year + 1900, 
					pkLog->ttDate.tm_mon + 1, pkLog->ttDate.tm_mday);
			pkLog->pFile = fopen(fname, "a");
		}

		if(!pkLog->pFile)
		{
			return;
		}
	}

	if(SeHasLogLV(pkLog, LT_ERROR))
	{
		sprintf(pkLog->actext, "%04d-%02d-%02d %02d:%02d:%02d [LOG ERROR] %s\n", tt_now.tm_year + 1900, 
			tt_now.tm_mon + 1, tt_now.tm_mday, tt_now.tm_hour, tt_now.tm_min, tt_now.tm_sec, text);
	}
	
	if(SeHasLogLV(pkLog, LT_WARNING))
	{
		sprintf(pkLog->actext, "%04d-%02d-%02d %02d:%02d:%02d [LOG WARNING] %s\n", tt_now.tm_year + 1900, 
			tt_now.tm_mon + 1, tt_now.tm_mday, tt_now.tm_hour, tt_now.tm_min, tt_now.tm_sec, text);
	}

	if(SeHasLogLV(pkLog, LT_INFO))
	{
		sprintf(pkLog->actext, "%04d-%02d-%02d %02d:%02d:%02d [LOG INFO] %s\n", tt_now.tm_year + 1900, 
			tt_now.tm_mon + 1, tt_now.tm_mday, tt_now.tm_hour, tt_now.tm_min, tt_now.tm_sec, text);
	}

	if(SeHasLogLV(pkLog, LT_DEBUG))
	{
		sprintf(pkLog->actext, "%04d-%02d-%02d %02d:%02d:%02d [LOG DEBUG] %s\n", tt_now.tm_year + 1900, 
			tt_now.tm_mon + 1, tt_now.tm_mday, tt_now.tm_hour, tt_now.tm_min, tt_now.tm_sec, text);
	}

	if(SeHasLogLV(pkLog, LT_CRITICAL))
	{
		sprintf(pkLog->actext, "%04d-%02d-%02d %02d:%02d:%02d [LOG CRITICAL] %s\n", tt_now.tm_year + 1900, 
			tt_now.tm_mon + 1, tt_now.tm_mday, tt_now.tm_hour, tt_now.tm_min, tt_now.tm_sec, text);
	}

	if(SeHasLogLV(pkLog, LT_SOCKET))
	{
		sprintf(pkLog->actext, "%04d-%02d-%02d %02d:%02d:%02d [LOG SOCKET] %s\n", tt_now.tm_year + 1900, 
			tt_now.tm_mon + 1, tt_now.tm_mday, tt_now.tm_hour, tt_now.tm_min, tt_now.tm_sec, text);
	}

	if(SeHasLogLV(pkLog, LT_NOTSET))
	{
		sprintf(pkLog->actext, "%04d-%02d-%02d %02d:%02d:%02d [LOG NOTSET] %s\n", tt_now.tm_year + 1900, 
			tt_now.tm_mon + 1, tt_now.tm_mday, tt_now.tm_hour, tt_now.tm_min, tt_now.tm_sec, text);
	}
	
	fwrite(pkLog->actext, 1, strlen(pkLog->actext), pkLog->pFile);
}
