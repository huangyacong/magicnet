#include "SeNetEngine.h"

const int NET_ENGINE_MAX_LEN = 1024 * 1024 * 4 - 256;

SeNetEngine::SeNetEngine()
{
	m_bStop = false;
	m_bInit = false;
	m_pkRecvBuf = 0;
	m_pkSendBuf = 0;
	m_ullStatTime = SeTimeGetTickCount();
	m_uiStatDelayTime = 1000;
	m_iWaitTime = NET_CORE_WAIT_TIME;
	memset(&m_kMsgIDStat, 0, (int)sizeof(m_kMsgIDStat));
}

SeNetEngine::~SeNetEngine()
{
	if(!m_bInit) 
	{ 
		return; 
	}

	m_bInit = false;
	SeNetCoreFin(&m_kNetEngine);
	SeFreeMem(m_pkRecvBuf);
	SeFreeMem(m_pkSendBuf);
	m_kClient.clear();
	m_kListen.clear();
	m_kClientList.clear();
	m_kListenList.clear();
}

bool SeNetEngine::Init(const char *pcLogName, int iLogLV, unsigned short usMax, int iTimerCnt)
{
	if(m_bInit) 
	{ 
		return true; 
	}

	m_bInit = true;
	SeNetCoreInit(&m_kNetEngine, pcLogName, usMax, iTimerCnt, iLogLV);
	m_pkRecvBuf = (char*)SeMallocMem(NET_ENGINE_MAX_LEN);
	m_pkSendBuf = (char*)SeMallocMem(NET_ENGINE_MAX_LEN);
	return true;
}

bool SeNetEngine::AddTCPListen(IServer* pkIServer, int iDomain, bool bReusePort, const char *pcIP, unsigned short usPort, int iTimeOut, bool bNoDelay, bool bBigHeader)
{
	if(!m_bInit) 
	{ 
		return false; 
	}

	if(pkIServer->m_bUsed) 
	{ 
		return false; 
	}

	HSOCKET kHSocket = SeNetCoreTCPListen(&m_kNetEngine, iDomain, bReusePort, pcIP, usPort, bBigHeader ? 4 : 2, bNoDelay, iTimeOut, SeNetSreamGetHeader, SeNetSreamSetHeader);
	if (kHSocket > 0) 
	{ 
		m_kListen[kHSocket] = pkIServer; 
		pkIServer->m_bUsed = true;
		AddListenToList(pkIServer);
	}

	return kHSocket > 0 ? true : false;
}

HSOCKET SeNetEngine::AddTCPClient(IClient* pkIClient, int iDomain, const char *pcIP, unsigned short usPort, int iTimeOut, int iConnectTimeOut, bool bNoDelay, bool bBigHeader)
{
	if(!m_bInit) 
	{ 
		return 0; 
	}

	if(pkIClient->m_bUsed) 
	{ 
		return 0;
	}

	HSOCKET kHSocket = SeNetCoreTCPClient(&m_kNetEngine, iDomain, pcIP, usPort, bBigHeader ? 4 : 2, bNoDelay, iTimeOut, iConnectTimeOut, SeNetSreamGetHeader, SeNetSreamSetHeader);
	if(kHSocket <= 0) 
	{ 
		return 0; 
	}

	pkIClient->m_bUsed = true;
	m_kClient[kHSocket] = pkIClient;
	AddClientToList(pkIClient);
	return kHSocket;
}

void SeNetEngine::SetWaitTime(int iWaitTime)
{
	if(!m_bInit) 
	{ 
		return; 
	}

	m_iWaitTime = iWaitTime;
	SeNetCoreSetWaitTime(&m_kNetEngine, m_iWaitTime);
}

void SeNetEngine::SetStatDelayTime(unsigned int uiStatDelayTime)
{
	if (!m_bInit)
	{
		return;
	}

	m_uiStatDelayTime = uiStatDelayTime;
}

void SeNetEngine::SetLogContext(SELOGCONTEXT pkLogContextFunc, void *pkLogContect)
{
	SeNetCoreSetLogContextFunc(&m_kNetEngine, pkLogContextFunc, pkLogContect);
}

void SeNetEngine::AddClientToList(IClient *pkIClient)
{
	m_kClientList.push_back(pkIClient);
}

void SeNetEngine::DelClientToList(IClient *pkIClient)
{
	for(list<IClient*>::iterator itr = m_kClientList.begin(); itr != m_kClientList.end(); itr++)
	{
		if ((*itr) == pkIClient)
		{
			m_kClientList.erase(itr);
			break;
		}
	}
}

void SeNetEngine::AddListenToList(IServer *pkIServer)
{
	m_kListenList.push_back(pkIServer);
}

void SeNetEngine::RunUpdate()
{
	if(!m_bInit) 
	{ 
		return; 
	}

	for (list<IClient*>::iterator itrClient = m_kClientList.begin(); itrClient != m_kClientList.end(); itrClient++)
	{
		IClient* pkIClient = (*itrClient);
		pkIClient->OnUpdate();
	}

	for (list<IServer*>::iterator itrListen = m_kListenList.begin(); itrListen != m_kListenList.end(); itrListen++)
	{
		IServer* pkIServer = (*itrListen);
		pkIServer->OnUpdate();
	}
}

void SeNetEngine::RunStat()
{
	if (!m_bInit)
	{ 
		return; 
	}
	
	int iTime = 1;
	unsigned long long ullNow = SeTimeGetTickCount();
	if (m_ullStatTime + m_uiStatDelayTime > ullNow)
	{ 
		return; 
	}
	iTime = (int)(ullNow - m_ullStatTime);
	if (iTime <= 0)
	{
		return;
	}
	
	m_ullStatTime = ullNow;
	int iSendNum = (m_kMsgIDStat.iSendNum * 1000) / (int)iTime;
	unsigned long long ullSendSpeed = (unsigned long long)((unsigned long long)(m_kMsgIDStat.ullSendByteNum * 1000) / (unsigned long long)iTime);
	int iRecvNum = (m_kMsgIDStat.iRecvNum * 1000) / (int)iTime;
	unsigned long long ullRecvSpeed = (unsigned long long)((unsigned long long)(m_kMsgIDStat.ullRecvByteNum * 1000) / (unsigned long long)iTime);

	OnPrintStat(iSendNum, ullSendSpeed, iRecvNum, ullRecvSpeed);

	memset(&m_kMsgIDStat, 0, (int)sizeof(m_kMsgIDStat));
}

void SeNetEngine::StartEngine()
{
	while(m_bInit && !m_bStop) 
	{
		Run(); 
	}
}

void SeNetEngine::StopEngine()
{
	m_bStop = true;
}

void SeNetEngine::Run()
{
	int iLen;
	int riEvent;
	int rSSize;
	int rRSize;
	HSOCKET rkHSocket;
	HSOCKET rkListenHSocket;

	if (!m_bInit)
	{
		return;
	}

	iLen = NET_ENGINE_MAX_LEN;
	if(!SeNetCoreRead(&m_kNetEngine, &riEvent, &rkListenHSocket, &rkHSocket, m_pkRecvBuf, &iLen, &rSSize, &rRSize))  
	{ 
		return; 
	}

	if (riEvent == SENETCORE_EVENT_SOCKET_IDLE) 
	{ 
		if (m_iWaitTime == 0)
			SeTimeSleep(1);
		return; 
	}

	if (riEvent == SENETCORE_EVENT_SOCKET_TIMER)
	{
		RunStat();
		RunUpdate();
		OnUpdate();
		return;
	}

	// 连接失败
	if(riEvent == SENETCORE_EVENT_SOCKET_CONNECT_FAILED)
	{
		// 发出连接失败
		map<HSOCKET, IClient*>::iterator itrClient;
		itrClient = m_kClient.find(rkHSocket);
		if(itrClient != m_kClient.end())
		{
			IClient* pkIClient = itrClient->second;
			DelClientToList(pkIClient);
			m_kClient.erase(itrClient);
			pkIClient->m_bUsed = false;
			pkIClient->m_bOnConnect = false;
			pkIClient->OnConnectFailed();
		}
		return;
	}

	// 收到一个连接
	if(riEvent == SENETCORE_EVENT_SOCKET_CONNECT)
	{
		m_pkRecvBuf[iLen] = '\0';

		// 发出的连接成功
		map<HSOCKET, IClient*>::iterator itrClient;
		itrClient = m_kClient.find(rkHSocket);
		if(itrClient != m_kClient.end())
		{
			IClient* pkIClient = itrClient->second;
			pkIClient->m_bOnConnect = true;
			pkIClient->OnConnect(m_pkRecvBuf, iLen);
			return;
		}

		// 某个端口收到网络连接
		map<HSOCKET, IServer*>::iterator itrListen;
		itrListen = m_kListen.find(rkListenHSocket);
		if(itrListen != m_kListen.end())
		{
			IServer* pkIServer = itrListen->second;
			pkIServer->OnConnect(rkHSocket, m_pkRecvBuf, iLen);
			return;
		}
		return;
	}

	// 收到断开连接
	if(riEvent == SENETCORE_EVENT_SOCKET_DISCONNECT)
	{
		// 发出的连接断开了
		map<HSOCKET, IClient*>::iterator itrClient;
		itrClient = m_kClient.find(rkHSocket);
		if(itrClient != m_kClient.end())
		{
			IClient* pkIClient = itrClient->second;
			DelClientToList(pkIClient);
			m_kClient.erase(itrClient);
			pkIClient->m_bUsed = false;
			pkIClient->m_bOnConnect = false;
			pkIClient->OnDisConnect();
			return;
		}

		// 某个端口收到网络断开连接
		map<HSOCKET, IServer*>::iterator itrListen;
		itrListen = m_kListen.find(rkListenHSocket);
		if(itrListen != m_kListen.end())
		{
			IServer* pkIServer = itrListen->second;
			pkIServer->OnDisConnect(rkHSocket);
			return;
		}
		return;
	}

	// 收到网络数据
	if(riEvent == SENETCORE_EVENT_SOCKET_RECV_DATA)
	{
		m_pkRecvBuf[iLen] = '\0';
		m_kMsgIDStat.iRecvNum++;
		m_kMsgIDStat.ullRecvByteNum += iLen;
		
		// 发出的连接收到数据
		map<HSOCKET, IClient*>::iterator itrClient;
		itrClient = m_kClient.find(rkHSocket);
		if(itrClient != m_kClient.end())
		{
			IClient* pkIClient = itrClient->second;
			pkIClient->OnRecv(m_pkRecvBuf, iLen, rSSize, rRSize);
			return;
		}

		// 某个端口收到数据
		map<HSOCKET, IServer*>::iterator itrListen;
		itrListen = m_kListen.find(rkListenHSocket);
		if(itrListen != m_kListen.end())
		{
			IServer* pkIServer = itrListen->second;
			pkIServer->OnRecv(rkHSocket, m_pkRecvBuf, iLen, rSSize, rRSize);
			return;
		}
		return;
	}
}

char *SeNetEngine::GetSendBuf(int &riLen)
{
	if (!m_bInit) 
	{ 
		return 0; 
	}
	riLen = NET_ENGINE_MAX_LEN;
	return m_pkSendBuf;
}

struct SELOG* SeNetEngine::GetEngineLogInstance()
{
	if (!m_bInit) 
	{ 
		return 0; 
	}
	return &m_kNetEngine.kLog;
}

bool SeNetEngine::SendEngineData(HSOCKET kHSocket, const char* pcBuf, int iSize)
{
	if(!m_bInit) 
	{ 
		return false;
	}

	bool bRet = SeNetCoreSend(&m_kNetEngine, kHSocket, pcBuf, iSize);

	if (bRet)
	{
		m_kMsgIDStat.iSendNum++;
		m_kMsgIDStat.ullSendByteNum += iSize;
	}

	return bRet;
}

bool SeNetEngine::SendEngineData(HSOCKET kHSocket, const char* pcBufF, int iSizeF, const char* pcBufS, int iSizeS)
{
	if (!m_bInit)
	{ 
		return false; 
	}

	struct SENETSTREAMBUF kBuf[] = { { pcBufF, iSizeF }, { pcBufS, iSizeS } };
	bool bRet = SeNetCoreSendExtend(&m_kNetEngine, kHSocket, kBuf, (int)(sizeof(kBuf) / sizeof(struct SENETSTREAMBUF)));

	if (bRet)
	{
		m_kMsgIDStat.iSendNum++;
		m_kMsgIDStat.ullSendByteNum += iSizeF + iSizeS;
	}
	
	return bRet;
}

void SeNetEngine::DiscEngineSocket(HSOCKET kHSocket)
{
	if(!m_bInit)
	{ 
		return; 
	}

	SeNetCoreDisconnect(&m_kNetEngine, kHSocket);
}

