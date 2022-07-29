#ifndef __SE_WSBASE_H__
#define __SE_WSBASE_H__

#include "SeNetEngine.h"

class SeWSBase
{
public:
	SeWSBase(SeNetEngine* pkSeNetEngine, HSOCKET kHSocket, const string& strIP);
	~SeWSBase();
public:
	HSOCKET GetHSocket();
	bool IsHandShake();
	void SetHandShake();
	const string& GetIP();
public:
	bool ServerHandShake();
public:
	bool PushRecvData(const char *pcBuf, int iLen);
private:
	string m_strIP;
	HSOCKET m_kHSocket;
	bool m_bHandShake;
	SENETSTREAM m_kRecvNetStream;
	SeNetEngine* m_pkSeNetEngine;
};

#endif


