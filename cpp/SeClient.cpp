#include "SeClient.h"

// 重连间隔时间,没次无法连接，就加上这个时间，几次之后，立马重连
const int iReConnectDelayTime = 1000;
// 重连循环次数
const int iReConnectCount = 10;

// 初始化
SeClient::SeClient() : IClient()
{
	m_bInit = false;
	m_bNoDelay = false;
	m_pkSeNetEngine = NULL;
	m_iPingTimeDelay = 1000 * 2;
}

SeClient::~SeClient()
{
	m_bInit = false;
}

void SeClient::SetNoDelay(bool bNoDelay)
{
	m_bNoDelay = bNoDelay;
}

void SeClient::Init(SeNetEngine* pkSeNetEngine, const char* IP, int Port, int iTimeOut, int iConnectTimeOut, bool bBigHeader)
{
	if(m_bInit) 
	{ 
		return; 
	}

	unsigned long long ullNowTime = SeTimeGetTickCount();

	m_bInit = true;
	m_kHSocket = 0;
	SeStrNcpy(m_acSvrIP, (int)sizeof(m_acSvrIP), IP);
	m_iSvrPort = Port;
	m_ullTime = ullNowTime;
	m_ullReConnectTime = ullNowTime;
	m_pkSeNetEngine = pkSeNetEngine;
	m_iReConnectNum = 0;
	m_iTimeOut = iTimeOut;
	m_iConnectTimeOut = iConnectTimeOut;
	m_bBigHeader = bBigHeader;
}

// 连接
void SeClient::Connect()
{
	if(!m_bInit) 
	{ 
		return; 
	}

	if(m_kHSocket > 0)
	{ 
		return; 
	}

	unsigned long long ullNowTime = SeTimeGetTickCount();
	if (m_ullReConnectTime > ullNowTime) 
	{ 
		return; 
	}

	m_ullReConnectTime = ullNowTime;
	m_kHSocket = m_pkSeNetEngine->AddTCPConnect(this, m_acSvrIP, m_iSvrPort, m_iTimeOut, m_iConnectTimeOut, m_bNoDelay, m_bBigHeader);

	if(m_kHSocket == 0) 
	{ 
		OnConnectFailed(); 
	}
}

void SeClient::DisConnect()
{
	if(!m_bInit) 
	{ 
		return; 
	}

	if(m_kHSocket <= 0) 
	{ 
		return;
	}

	m_pkSeNetEngine->DiscEngineSocket(m_kHSocket);
}

bool SeClient::IsConnect()
{
	if(!m_bInit) 
	{ 
		return false;
	}

	return m_kHSocket != 0;
}

bool SeClient::IsUsed()
{
	return IClient::IsUsed();
}

void SeClient::SetPingDelayTime(int iPingTime)
{
	m_iPingTimeDelay = iPingTime;
}

bool SeClient::SendData(const char* pcBuf, int iSize)
{
	if(!m_bInit) 
	{ 
		return false; 
	}

	if(m_kHSocket <= 0) 
	{ 
		return false; 
	}

	return m_pkSeNetEngine->SendEngineData(m_kHSocket, pcBuf, iSize);
}

bool SeClient::SendData(const char* pcBufF, int iSizeF, const char* pcBufS, int iSizeS)
{
	if (!m_bInit) 
	{
		return false; 
	}

	if (m_kHSocket <= 0) 
	{ 
		return false; 
	}

	return m_pkSeNetEngine->SendEngineData(m_kHSocket, pcBufF, iSizeF, pcBufS, iSizeS);
}

void SeClient::OnConnect(const char *pcIP, int iLen)
{
	unsigned long long ullNowTime = SeTimeGetTickCount();

	m_iReConnectNum = 0;
	m_ullTime = ullNowTime;
	m_ullReConnectTime = ullNowTime;
	OnSendPacketAttach();
	OnClientConnect(pcIP, iLen);
}

void SeClient::OnConnectFailed()
{
	m_iReConnectNum++;
	if(m_iReConnectNum > iReConnectCount) 
	{ 
		m_iReConnectNum = 0; 
	}

	m_ullReConnectTime = SeTimeGetTickCount() + m_iReConnectNum*iReConnectDelayTime;

	m_kHSocket = 0;
	OnClientConnectFailed();
}

void SeClient::OnDisConnect()
{
	m_iReConnectNum++;
	if(m_iReConnectNum > iReConnectCount) 
	{
		m_iReConnectNum = 0; 
	}

	m_ullReConnectTime = SeTimeGetTickCount() + m_iReConnectNum*iReConnectDelayTime;

	m_kHSocket = 0;
	OnClientDisConnect();
}

void SeClient::OnRecv(const char *pcBuf, int iLen, int iSendSize, int iRecvSize)
{
	OnClientRecv(pcBuf, iLen, iSendSize, iRecvSize);
}

void SeClient::OnUpdate()
{
	if(!m_bInit) 
	{ 
		return; 
	}

	if(m_kHSocket <= 0)
	{ 
		return; 
	}

	OnClientUpdate();

	unsigned long long ullNowTime = SeTimeGetTickCount();
	if (m_iPingTimeDelay + m_ullTime > ullNowTime)
	{ 
		return; 
	}

	m_ullTime = ullNowTime;
	OnSendPacketPing();
}
