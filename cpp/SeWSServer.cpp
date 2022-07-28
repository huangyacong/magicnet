#include "SeWSServer.h"

SeWSServer::SeWSServer() : IServer()
{
	m_bInit = false;
	m_iDomain = 0;
	m_bNoDelay = false;
	m_bReusePort = false;
	m_pkSeNetEngine = NULL;
}

SeWSServer::~SeWSServer()
{
	m_bInit = false;
	m_pkSeNetEngine = NULL;
}

void SeWSServer::SetReusePort()
{
	m_bReusePort = true;
}

void SeWSServer::SetNoDelay(bool bNoDelay)
{
	m_bNoDelay = bNoDelay;
}

void SeWSServer::Init(SeNetEngine* pkSeNetEngine, int iDomain, const char* IP, int Port, int iTimeOut)
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
}

bool SeWSServer::Listen()
{
	if(!m_bInit) 
	{ 
		return false;
	}

	return m_pkSeNetEngine->AddTCPListen(this, m_iDomain, m_bReusePort, m_acSvrIP.c_str(), m_iSvrPort, m_iTimeOut, m_bNoDelay, HEADER_TYPE_ZERO);
}

void SeWSServer::DisConnect(HSOCKET kHSocket)
{
	if(!m_bInit) 
	{ 
		return;
	}

	m_pkSeNetEngine->DiscEngineSocket(kHSocket);
}

bool SeWSServer::SendData(HSOCKET kHSocket, const char* pcBuf, int iSize)
{
	if(!m_bInit) 
	{ 
		return false; 
	}

	return m_pkSeNetEngine->SendEngineData(kHSocket, pcBuf, iSize);
}

bool SeWSServer::SendData(HSOCKET kHSocket, const char* pcBufF, int iSizeF, const char* pcBufS, int iSizeS)
{
	if (!m_bInit) 
	{
		return false; 
	}

	return m_pkSeNetEngine->SendEngineData(kHSocket, pcBufF, iSizeF, pcBufS, iSizeS);
}

void SeWSServer::OnConnect(HSOCKET kHSocket, const char *pcIP, int iLen)
{
	OnWSServerConnect(kHSocket, pcIP, iLen);
}

void SeWSServer::OnDisConnect(HSOCKET kHSocket)
{
	OnWSServerDisConnect(kHSocket);
}

void SeWSServer::OnRecv(HSOCKET kHSocket, const char *pcBuf, int iLen, int iSendSize, int iRecvSize)
{
	OnWSServerRecv(kHSocket, pcBuf, iLen, iSendSize, iRecvSize);
}

void SeWSServer::OnUpdate()
{
	OnWSServerUpdate();
}

