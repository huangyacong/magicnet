#ifndef __SE_NETENGINE_H__
#define __SE_NETENGINE_H__

#include<map>
#include<list>
#include "SeNetCore.h"

using namespace std;

enum SeHeaderType
{
	HEADER_TYPE_ZERO = 0,
	HEADER_TYPE_SMALL = 2,
	HEADER_TYPE_BIG = 4,
};

class SeNetEngine;
class IClient
{
public:
	IClient() { m_bUsed = false; m_bOnConnect = false; }
	virtual ~IClient() {}
public:
	virtual bool IsUsed() { return m_bUsed; }
public:
	virtual void OnConnect(const char *pcIP, int iLen) = 0;
	virtual void OnConnectFailed() = 0;
	virtual void OnDisConnect() = 0;
	virtual void OnRecv(const char *pcBuf, int iLen, int iSendSize, int iRecvSize) = 0;
	virtual void OnUpdate() = 0;
private:
	bool m_bUsed;
	bool m_bOnConnect;
	friend class SeNetEngine;
};

class SeNetEngine;
class IServer
{
public:
	IServer() { m_bUsed = false; }
	virtual ~IServer() {}
public:
	virtual bool IsUsed() { return m_bUsed; }
public:
	virtual void OnConnect(HSOCKET kHSocket, const char *pcIP, int iLen) = 0;
	virtual void OnDisConnect(HSOCKET kHSocket) = 0;
	virtual void OnRecv(HSOCKET kHSocket, const char *pcBuf, int iLen, int iSendSize, int iRecvSize) = 0;
	virtual void OnUpdate() = 0;
private:
	bool m_bUsed;
	friend class SeNetEngine;
};

class SeNetEngine
{
public:
	SeNetEngine();
	virtual ~SeNetEngine();
public:
	virtual bool Init(const char *pcLogName, int iLogLV, unsigned short usMax, int iTimerCnt);
	virtual bool AddTCPListen(IServer* pkIServer, int iDomain, bool bReusePort, const char *pcIP, unsigned short usPort, int iTimeOut, bool bNoDelay, SeHeaderType eHeaderType);
	virtual HSOCKET AddTCPClient(IClient* pkIClient, int iDomain, const char *pcIP, unsigned short usPort, int iTimeOut, int iConnectTimeOut, bool bNoDelay, SeHeaderType eHeaderType);
public:
	virtual char *GetSendBuf(int &riLen);
	virtual struct SELOG* GetEngineLogInstance();
	virtual bool SendEngineData(HSOCKET kHSocket, const char* pcBuf, int iSize);
	virtual bool SendEngineData(HSOCKET kHSocket, const char* pcBufF, int iSizeF, const char* pcBufS, int iSizeS);
	virtual void DiscEngineSocket(HSOCKET kHSocket);
public:
	virtual void StartEngine();
	virtual void StopEngine();
	virtual void SetWaitTime(int iWaitTime);
	virtual void SetStatDelayTime(unsigned int uiStatDelayTime);
	virtual void SetLogContext(SELOGCONTEXT pkLogContextFunc, void *pkLogContect);
public:
	virtual void OnUpdate() = 0;
	virtual void OnPrintStat(int iSendNum, unsigned long long ullSendSpeed, int iRecvNum, unsigned long long ullRecvSpeed) = 0;
private:
	virtual void Run();
	virtual void RunStat();
	virtual void RunUpdate();
private:
	void AddClientToList(IClient *pkIClient);
	void DelClientToList(IClient *pkIClient);
	void AddListenToList(IServer *pkIServer);
private:
	char* m_pkRecvBuf;
	char* m_pkSendBuf;
private:
	bool m_bStop;
	bool m_bInit;
	int m_iWaitTime;
private:
	SENETCORE m_kNetEngine;
	list<IClient*> m_kClientList;
	list<IServer*> m_kListenList;
	map<HSOCKET, IClient*> m_kClient;
	map<HSOCKET, IServer*> m_kListen;
private:
	struct MsgIDStat
	{
		int iSendNum;
		unsigned long long ullSendByteNum;
		int iRecvNum;
		unsigned long long ullRecvByteNum;
	};
	MsgIDStat	m_kMsgIDStat;
	unsigned int m_uiStatDelayTime;
	unsigned long long m_ullStatTime;
	friend class IClient;
};

#define NETENGINE_CACHE_LOG(NetEngine, LogLv, format, ...) SeLogWrite((NetEngine).GetEngineLogInstance(), (LogLv), false, (format), ##__VA_ARGS__)
#define NETENGINE_FLUSH_LOG(NetEngine, LogLv, format, ...) SeLogWrite((NetEngine).GetEngineLogInstance(), (LogLv), true, (format), ##__VA_ARGS__)

#endif
