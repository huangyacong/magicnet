#ifndef __SE_LOCK_H__
#define __SE_LOCK_H__

#include "SeMutex.h"

class SeLock
{
public:
	SeLock();
	virtual ~SeLock();

	void Lock();
	void UnLock();
private:
	MUTEX_TYPE m_kLock;
};

class SeAutoLock
{
public:
	SeAutoLock(SeLock *pkLock);
	virtual ~SeAutoLock();
private:
	SeLock *m_pkLock;
};

#endif


