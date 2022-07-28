#ifndef __SE_WSSERVER_H__
#define __SE_WSSERVER_H__

#include <string>
#include "SeTime.h"
#include "SeNetEngine.h"

class SeWSServer : private IServer
{
public:
	SeWSServer();
	virtual ~SeWSServer();
public:
	virtual void SetReusePort();
	virtual void SetNoDelay(bool bNoDelay);
	virtual void Init(SeNetEngine* pkSeNetEngine, int iDomain, const char* IP, int Port, int iTimeOut);
	virtual bool Listen();
	virtual void DisConnect(HSOCKET kHSocket);
	virtual bool SendData(HSOCKET kHSocket, const char* pcBuf, int iSize);
	virtual bool SendData(HSOCKET kHSocket, const char* pcBufF, int iSizeF, const char* pcBufS, int iSizeS);
public:
	virtual void OnWSServerConnect(HSOCKET kHSocket, const char *pcIP, int iLen) = 0;
	virtual void OnWSServerDisConnect(HSOCKET kHSocket) = 0;
	virtual void OnWSServerRecv(HSOCKET kHSocket, const char *pcBuf, int iLen, int iSendSize, int iRecvSize) = 0;
	virtual void OnWSServerUpdate() = 0;
private:
	void OnConnect(HSOCKET kHSocket, const char *pcIP, int iLen);
	void OnDisConnect(HSOCKET kHSocket);
	void OnRecv(HSOCKET kHSocket, const char *pcBuf, int iLen, int iSendSize, int iRecvSize);
	void OnUpdate();
private:
	bool m_bInit;
	int m_iDomain;
	bool m_bReusePort;
	bool m_bNoDelay;
	int m_iSvrPort;
	int m_iTimeOut;
	string m_acSvrIP;
protected:
	SeNetEngine* m_pkSeNetEngine;
};

#endif
