#ifndef __SE_WSBASE_H__
#define __SE_WSBASE_H__

#include "SeNetEngine.h"

class SeWSBase
{
public:
	SeWSBase(SeNetEngine* pkSeNetEngine, HSOCKET kHSocket);
	~SeWSBase();
public:
	bool PushRecvData(const char *pcBuf, int iLen);
private:
	HSOCKET m_kHSocket;
	bool m_bHandShake;
	SENETSTREAM m_kRecvNetStream;
	SeNetEngine* m_pkSeNetEngine;
};

#endif


