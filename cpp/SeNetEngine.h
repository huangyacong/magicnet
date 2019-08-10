#ifndef __SE_NETENGINE_H__
#define __SE_NETENGINE_H__

#include<map>
#include<list>
#include "SeNetCore.h"

using namespace std;

class SeNetEngine;
class IClient
{
public:
	IClient() { m_bUsed = false; m_bOnConnect = false;  m_ullUpdateTime = SeTimeGetTickCount(); }
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
	unsigned long long m_ullUpdateTime;
	friend class SeNetEngine;
};

class SeNetEngine;
class IServer
{
public:
	IServer() { m_bUsed = false; m_ullUpdateTime = SeTimeGetTickCount(); }
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
	unsigned long long m_ullUpdateTime;
	friend class SeNetEngine;
};

class SeNetEngine
{
public:
	SeNetEngine();
	virtual ~SeNetEngine();
public:
	virtual bool Init(const char *pcLogName, int iLogLV, unsigned short usMax);
	virtual bool AddTCPListen(IServer* pkIServer, bool bReusePort, const char *pcIP, unsigned short usPort, int iTimeOut, bool bNoDelay, bool bBigHeader);
	virtual HSOCKET AddTCPConnect(IClient* pkIClient, const char *pcIP, unsigned short usPort, int iTimeOut, int iConnectTimeOut, bool bNoDelay, bool bBigHeader);
public:
	virtual char *GetSendBuf(int &riLen);
	virtual bool SendEngineData(HSOCKET kHSocket, const char* pcBuf, int iSize);
	virtual bool SendEngineData(HSOCKET kHSocket, const char* pcBufF, int iSizeF, const char* pcBufS, int iSizeS);
	virtual void DiscEngineSocket(HSOCKET kHSocket);
public:
	virtual void StartEngine();
	virtual void StopEngine();
	virtual void SetWaitTime(unsigned int uiWaitTime);
	virtual void SetStatDelayTime(unsigned int uiStatDelayTime);
	virtual void SetUpdateDelayTime(unsigned int uiUpdateDelayTime);
	virtual void SetLogContext(SELOGCONTEXT pkLogContextFunc, void *pkLogContect);
public:
	virtual void OnIdleUpdate() = 0;
	virtual void OnUpdate() = 0;
	virtual void OnPrintStat(int iSendNum, unsigned long long ullSendSpeed, int iRecvNum, unsigned long long ullRecvSpeed) = 0;
private:
	static bool SeSetHeader(unsigned char* pcHeader, const int iheaderlen, const int ilen);
	static bool SeGetHeader(const unsigned char* pcHeader, const int iheaderlen, int *ilen);
private:
	virtual void Run();
	virtual void Stat(unsigned long long ullNow);
	virtual void Update(unsigned long long ullNow);
private:
	void AddClientToList(IClient *pkIClient);
	void DelClientToList(IClient *pkIClient);
	void AddListenToList(IServer *pkIServer);
	void DelListenToList(IServer *pkIServer);
private:
	char* m_pkRecvBuf;
	char* m_pkSendBuf;
private:
	bool m_bStop;
	bool m_bInit;
	unsigned long long m_kullTime;
	unsigned int m_uiUpdateDelayTime;
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
	unsigned int m_uiWaitTime;
	friend class IClient;
};

#endif
