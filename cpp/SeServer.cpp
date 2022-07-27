#include "SeServer.h"

SeServer::SeServer() : IServer()
{
	m_bInit = false;
	m_iDomain = 0;
	m_bNoDelay = false;
	m_bReusePort = false;
	m_pkSeNetEngine = NULL;
}

SeServer::~SeServer()
{
	m_bInit = false;
	m_pkSeNetEngine = NULL;
}

void SeServer::SetReusePort()
{
	m_bReusePort = true;
}

void SeServer::SetNoDelay(bool bNoDelay)
{
	m_bNoDelay = bNoDelay;
}

void SeServer::Init(SeNetEngine* pkSeNetEngine, int iDomain, const char* IP, int Port, int iTimeOut, bool bBigHeader)
{
	if(m_bInit) 
	{ 
		return;
	}

	m_bInit = true;
	m_iDomain = iDomain;
	m_iSvrPort = Port;
	m_acSvrIP = string(IP);
	m_pkSeNetEngine = pkSeNetEngine;
	m_iTimeOut = iTimeOut;
	m_bBigHeader = bBigHeader;
}

bool SeServer::Listen()
{
	if(!m_bInit) 
	{ 
		return false;
	}

	return m_pkSeNetEngine->AddTCPListen(this, m_iDomain, m_bReusePort, m_acSvrIP.c_str(), m_iSvrPort, m_iTimeOut, m_bNoDelay, m_bBigHeader ? HEADER_TYPE_BIG : HEADER_TYPE_SMALL);
}

void SeServer::DisConnect(HSOCKET kHSocket)
{
	if(!m_bInit) 
	{ 
		return;
	}

	m_pkSeNetEngine->DiscEngineSocket(kHSocket);
}

bool SeServer::SendData(HSOCKET kHSocket, const char* pcBuf, int iSize)
{
	if(!m_bInit) 
	{ 
		return false; 
	}

	return m_pkSeNetEngine->SendEngineData(kHSocket, pcBuf, iSize);
}

bool SeServer::SendData(HSOCKET kHSocket, const char* pcBufF, int iSizeF, const char* pcBufS, int iSizeS)
{
	if (!m_bInit) 
	{
		return false; 
	}

	return m_pkSeNetEngine->SendEngineData(kHSocket, pcBufF, iSizeF, pcBufS, iSizeS);
}

void SeServer::OnConnect(HSOCKET kHSocket, const char *pcIP, int iLen)
{
	OnServerConnect(kHSocket, pcIP, iLen);
}

void SeServer::OnDisConnect(HSOCKET kHSocket)
{
	OnServerDisConnect(kHSocket);
}

void SeServer::OnRecv(HSOCKET kHSocket, const char *pcBuf, int iLen, int iSendSize, int iRecvSize)
{
	OnServerRecv(kHSocket, pcBuf, iLen, iSendSize, iRecvSize);
}

void SeServer::OnUpdate()
{
	OnServerUpdate();
}

