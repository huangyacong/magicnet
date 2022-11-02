#ifndef __SE_WSBASE_H__
#define __SE_WSBASE_H__

#include "SeNetEngine.h"
#include "SeWSFrame.h"
#include <vector>

using namespace std;

class SeWSBase
{
public:
	SeWSBase(SeNetEngine* pkSeNetEngine, HSOCKET kHSocket, const string& strIP);
	~SeWSBase();
public:
	HSOCKET GetHSocket() { return m_kHSocket; }
	bool IsHandShake() { return m_bHandShake; }
	void SetHandShake() { m_bHandShake = true; }
	const string& GetIP() { return m_strIP; }
public:
	bool ServerHandShake(bool& bHandShakeOK);
public:
	bool PushRecvData(const char *pcBuf, int iLen);
	int GetReadLen();
public:
	enum { WEB_SOCKET_VERSION = 13, };
private:
	string m_strIP;
	bool m_bHandShake;
	HSOCKET m_kHSocket;
	SENETSTREAM m_kRecvNetStream;
	SeNetEngine* m_pkSeNetEngine;
	map<string, string> m_mapHeadrs;

	SeWSFrame* m_pkWSFrame;
	vector<SeWSFrame*> m_vecFrameData;
};

#endif


