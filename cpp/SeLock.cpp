#include "SeLock.h"

SeLock::SeLock()
{
	SeMutexInit(&m_kLock);
}

SeLock::~SeLock()
{
	SeMutexDes(&m_kLock);
}

void SeLock::Lock()
{
	SeMutexLock(&m_kLock);
}

void SeLock::UnLock()
{
	SeMutexUnLock(&m_kLock);
}

SeAutoLock::SeAutoLock(SeLock *pkLock)
{
	m_pkLock = pkLock;
	m_pkLock->Lock();
}

SeAutoLock::~SeAutoLock()
{
	m_pkLock->UnLock();
	m_pkLock = 0;
}


