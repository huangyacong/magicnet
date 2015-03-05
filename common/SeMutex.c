#include "SeMutex.h"

void SeMutexInit(SeMutex *pkMutex)
{
	MUTEX_INIT(pkMutex);
}

void SeMutexDes(SeMutex *pkMutex)
{
	MUTEX_DESTROY(pkMutex);
}

void SeMutexLock(SeMutex *pkMutex)
{
	MUTEX_LOCK(pkMutex);
}

void SeMutexUnLock(SeMutex *pkMutex)
{
	MUTEX_UNLOCK(pkMutex);
}
