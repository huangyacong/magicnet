#ifndef __SE_CLIENT_H__
#define __SE_CLIENT_H__

#include <string>
#include "SeTime.h"
#include "SeNetEngine.h"

class SeClient : private IClient
{
public:
	SeClient();
	virtual ~SeClient();
public:
	virtual void SetNoDelay(bool bNoDelay);
	virtual void Init(SeNetEngine* pkSeNetEngine, const char* IP, int Port, int iTimeOut, int iConnectTimeOut, bool bBigHeader);
	virtual void Connect();
	virtual void DisConnect();
	virtual bool IsConnect();
	virtual bool IsUsed();
	virtual void SetPingDelayTime(int iPingTime);
	virtual bool SendData(const char* pcBuf, int iSize);
	virtual bool SendData(const char* pcBufF, int iSizeF, const char* pcBufS, int iSizeS);
public:
	virtual void OnSendPacketAttach() = 0;
	virtual void OnSendPacketPing() = 0;
	virtual void OnClientConnect(const char *pcIP, int iLen) = 0;
	virtual void OnClientConnectFailed() = 0;
	virtual void OnClientDisConnect() = 0;
	virtual void OnClientRecv(const char *pcBuf, int iLen, int iSendSize, int iRecvSize) = 0;
	virtual void OnClientUpdate() = 0;
private:
	void OnConnect(const char *pcIP, int iLen);
	void OnConnectFailed();
	void OnDisConnect();
	void OnRecv(const char *pcBuf, int iLen, int iSendSize, int iRecvSize);
	void OnUpdate();
private:
	bool m_bInit;
	HSOCKET m_kHSocket;
private:
	int m_iSvrPort;
	int m_iTimeOut;
	bool m_bBigHeader;
	int m_iConnectTimeOut;
private:
	bool m_bNoDelay;
	int m_iPingTimeDelay;
	unsigned long long m_ullTime;
private:
	int m_iReConnectNum;
	unsigned long long m_ullReConnectTime;
private:
	string m_acSvrIP;
protected:
	SeNetEngine* m_pkSeNetEngine;
};

#endif
