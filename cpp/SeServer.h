#ifndef __SE_SERVER_H__
#define __SE_SERVER_H__

#include <string>
#include "SeTime.h"
#include "SeNetEngine.h"

class SeServer : private IServer
{
public:
	SeServer();
	virtual ~SeServer();
public:
	virtual void SetReusePort();
	virtual void SetNoDelay(bool bNoDelay);
	virtual void Init(SeNetEngine* pkSeNetEngine, const char* IP, int Port, int iTimeOut, bool bBigHeader);
	virtual bool Listen();
	virtual void DisConnect(HSOCKET kHSocket);
	virtual bool SendData(HSOCKET kHSocket, const char* pcBuf, int iSize);
	virtual bool SendData(HSOCKET kHSocket, const char* pcBufF, int iSizeF, const char* pcBufS, int iSizeS);
public:
	virtual void OnServerConnect(HSOCKET kHSocket, const char *pcIP, int iLen) = 0;
	virtual void OnServerDisConnect(HSOCKET kHSocket) = 0;
	virtual void OnServerRecv(HSOCKET kHSocket, const char *pcBuf, int iLen, int iSendSize, int iRecvSize) = 0;
	virtual void OnServerUpdate() = 0;
private:
	void OnConnect(HSOCKET kHSocket, const char *pcIP, int iLen);
	void OnDisConnect(HSOCKET kHSocket);
	void OnRecv(HSOCKET kHSocket, const char *pcBuf, int iLen, int iSendSize, int iRecvSize);
	void OnUpdate();
private:
	bool m_bInit;
	bool m_bReusePort;
	bool m_bNoDelay;
	bool m_bBigHeader;
	int m_iSvrPort;
	int m_iTimeOut;
	char m_acSvrIP[16];
protected:
	SeNetEngine* m_pkSeNetEngine;
};

#endif
