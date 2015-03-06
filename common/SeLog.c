#include "SeLog.h"

void SeInitLog(struct SELOG *pkLog)
{
	pkLog->iFlag = 0;
	pkLog->iDate = 0;
	pkLog->pFile = 0;

	time_t tt_now = 0;
	tt_now = time(NULL);
}

void SeFinLog(struct SELOG *pkLog)
{
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
