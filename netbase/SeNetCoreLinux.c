#include "SeNetCore.h"

#if defined(__linux)

void SeNetCoreInit(struct SENETCORE *pkNetCore, char *pcLogName, unsigned short usMax)
{
	pkNetCore->kHandle = epoll_create(usMax);
	assert(pkNetCore->kHandle != SE_INVALID_HANDLE);
	SeInitLog(&pkNetCore->kLog, pcLogName);
	SeAddLogLV(&pkNetCore->kLog, LT_PRINT);
	SeNetSocketMgrInit(&pkNetCore->kSocketMgr, usMax);
}

void SeNetCoreFin(struct SENETCORE *pkNetCore)
{
	SeCloseHandle(pkNetCore->kHandle);
	SeFinLog(&pkNetCore->kLog);
	SeNetSocketMgrFin(&pkNetCore->kSocketMgr);
}

HSOCKET SeNetCoreTCPListen(struct SENETCORE *pkNetCore, const char *pcIP, unsigned short usPort,\
				int iHeaderLen, SEGETHEADERLENFUN pkGetHeaderLenFun, SESETHEADERLENFUN pkSetHeaderLenFun)
{
	SOCKET socket;
	HSOCKET kHSocket;
	struct sockaddr kServerAddr;
	struct epoll_event kEvent;

	socket = SeSocket(SOCK_STREAM);
	if(socket == SE_INVALID_SOCKET) return 0;
	SeSetSockAddr(&kServerAddr, pcIP, usPort);
	SeSetReuseAddr(socket);
	if(SeBind(socket, &kServerAddr) != 0) { SeCloseSocket(socket); return 0; }
	if(SeListen(socket, 1024) != 0) { SeCloseSocket(socket); return 0; }
	if(SeSetNoBlock(socket) != 0) { SeCloseSocket(socket); return 0; }
	kEvent.events = EPOLLIN | EPOLLOUT;
	if(epoll_ctl(pkNetCore->kHandle, EPOLL_CTL_ADD, socket, &kEvent) != 0) { SeCloseSocket(socket); return 0; }

	kHSocket = SeNetSocketMgrAdd(&pkNetCore->kSocketMgr, socket, LISTEN_TCP_TYPE_SOCKET, iHeaderLen, pkGetHeaderLenFun, pkSetHeaderLenFun);
	if(kHSocket <= 0) { SeCloseSocket(socket); return 0; }
	return kHSocket;
}

HSOCKET SeNetCoreTCPClient(struct SENETCORE *pkNetCore, const char *pcIP, unsigned short usPort,\
				int iHeaderLen, SEGETHEADERLENFUN pkGetHeaderLenFun, SESETHEADERLENFUN pkSetHeaderLenFun)
{
	int iErrorno;
	int iResult;
	SOCKET socket;
	HSOCKET kHSocket;
	struct sockaddr kAddr;
	struct linger so_linger;
	struct epoll_event kEvent;
	struct SESOCKET *pkNetSocket;

	socket = SeSocket(SOCK_STREAM);
	if(socket == SE_INVALID_SOCKET) { SeCloseSocket(socket); return 0; }
	if(SeSetNoBlock(m_kSocket) != 0) { SeCloseSocket(socket); return 0; }
	so_linger.l_onoff = true;
	so_linger.l_linger = 0;
	if(SeSetSockOpt(m_kSocket, SOL_SOCKET, SO_LINGER, (char*)&so_linger, sizeof(so_linger)) != 0) { SeCloseSocket(socket); return 0; }

	kHSocket = SeNetSocketMgrAdd(&pkNetCore->kSocketMgr, socket, CLIENT_TCP_TYPE_SOCKET, iHeaderLen, pkGetHeaderLenFun, pkSetHeaderLenFun);
	if(kHSocket <= 0) { SeCloseSocket(socket); return 0; }

	SeSetSockAddr(&kAddr, pcIP, usPort);
	iResult = SeConnect(m_kSocket,&m_kAddr);
	iErrorno = SeErrno();

	pkNetSocket = SeNetSocketMgrGet(&pkNetCore->kSocketMgr, kHSocket);
	if(iResult != 0 && iErrorno != SE_EINPROGRESS) { SeCloseSocket(socket); SeNetSocketMgrDel(&pkNetCore->kSocketMgr, kHSocket); return 0; }
#ifndef EPOLLRDHUP
	kEvent.events = EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLHUP | EPOLLET;
#else
	kEvent.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLERR | EPOLLHUP | EPOLLET;
#endif
	kEvent.data.u64 = kHSocket;
	if(epoll_ctl(pkNetCore->kHandle, EPOLL_CTL_ADD, socket, &kEvent) != 0) { SeCloseSocket(socket); SeNetSocketMgrDel(&pkNetCore->kSocketMgr, kHSocket); return 0; }
	if(iResult == 0) { pkNetSocket->usStatus = SOCKET_STATUS_CONNECTED; return kHSocket; }
	pkNetSocket->usStatus = SOCKET_STATUS_DISCONNECTING;
	return kHSocket;
}

bool SeNetCoreSend(struct SENETCORE *pkNetCore, HSOCKET kHSocket, const char* pcBuf, int iSize)
{
	bool bRet;
	struct SESOCKET *pkNetSocket;

	pkNetSocket = SeNetSocketMgrGet(&pkNetCore->kSocketMgr, kHSocket);
	if(!pkNetSocket) return false;
	if(pkNetSocket->iTypeSocket != CLIENT_TCP_TYPE_SOCKET && pkNetSocket->iTypeSocket != ACCEPT_TCP_TYPE_SOCKET) return false;
	if(pkNetSocket->usStatus != SOCKET_STATUS_ACTIVECONNECT) return false;
	SeNetSocketMgrUpdateNetStreamIdle(&pkNetCore->kSocketMgr, 1024*4, SENETCORE_MAX_SOCKET_BUF_LEN);
	bRet = SeNetSreamWrite(&pkNetSocket->kSendNetStream, &pkNetCore->kSocketMgr.kNetStreamIdle, &pkNetSocket->pkSetHeaderLenFun, pkNetSocket->iHeaderSize, pcBuf, iSize);
	if(bRet) { SeNetSocketMgrAddSendOrRecvInList(&pkNetCore->kSocketMgr, pkNetSocket, true); }
	return bRet;
}

void SeNetCoreDisconnect(struct SENETCORE *pkNetCore, HSOCKET kHSocket)
{
	SOCKET socket;
	struct SESOCKET *pkNetSocket;

	pkNetSocket = SeNetSocketMgrGet(&pkNetCore->kSocketMgr, kHSocket);
	if(!pkNetSocket) return;
	if(pkNetSocket->iTypeSocket != CLIENT_TCP_TYPE_SOCKET && pkNetSocket->iTypeSocket != ACCEPT_TCP_TYPE_SOCKET) return;
	if(pkNetSocket->usStatus < SOCKET_STATUS_ACCEPT || pkNetSocket->usStatus > SOCKET_STATUS_CONNECTED) return;
	pkNetSocket->usStatus = SOCKET_STATUS_DISCONNECTING;
	socket = SeGetSocketByHScoket(kHSocket);
	SeCloseSocket(socket);
}

void SeNetCoreAccept(struct SENETCORE *pkNetCore, struct SESOCKET *pkNetSocketListen)
{
	HSOCKET kHSocket;
	SOCKET kSocket, kListenSocket;
	struct sockaddr ksockaddr;
	struct linger so_linger;
	struct SESOCKET *pkNetSocketAccept;
	
	kListenSocket = SeGetSocketByHScoket(pkNetSocketListen->kHSocket);
	while(true)
	{
		kSocket = SeAccept(kListenSocket, &ksockaddr);
		if(kSocket == SE_INVALID_SOCKET) break;
		kHSocket = SeNetSocketMgrAdd(&pkNetCore->kSocketMgr, kSocket, ACCEPT_TCP_TYPE_SOCKET, pkNetSocketListen->iHeaderLen, pkNetSocketListen->pkGetHeaderLenFun, pkNetSocketListen->pkSetHeaderLenFun);
		if(kHSocket <= 0) { SeCloseSocket(kSocket); continue; }

		so_linger.l_onoff = true;
		so_linger.l_linger = 0;
		if(SeSetSockOpt(kSocket, SOL_SOCKET, SO_LINGER, (char*)&so_linger, sizeof(so_linger)) != 0) { SeCloseSocket(kSocket); SeNetSocketMgrDel(&pkNetCore->kSocketMgr, kHSocket); continue; }
		if(SeSetNoBlock(kSocket) != 0) { SeCloseSocket(kSocket); SeNetSocketMgrDel(&pkNetCore->kSocketMgr, kHSocket); continue; }
		pkNetSocketAccept = SeNetSocketMgrGet(&pkNetCore->kSocketMgr, kHSocket);
		pkNetSocketAccept->usStatus = SOCKET_STATUS_ACCEPT;
		pkNetSocketAccept->kBelongListenHSocket = pkNetSocketListen->kHSocket;
	}
}

bool SeNetCoreRead(struct SENETCORE *pkNetCore, int *riEvent, HSOCKET *rkHSocket, char *pcBuf, int *riLen)
{
	int i;
	int iNum;
	HSOCKET kHSocket;
	struct SESOCKET *pkNetSocket;
	struct epoll_event akEvents[1024];

	memset(akEvents, 0, sizeof(epoll_event) * 1024);
	iNum = epoll_wait(pkNetCore->kHandle, akEvents, 1024, 0);
	if(iNum <= 0) return false;

	for(int i = 0; i < iNum; i++)
	{
		kHSocket = (HSOCKET)akEvents[i].data.u64;
		pkNetSocket = SeNetSocketMgrGet(&pkNetCore->kSocketMgr, kHSocket);
		if(!pkNetSocket) continue;

		if(pkNetSocket->iTypeSocket == LISTEN_TCP_TYPE_SOCKET)
		{
			SeNetCoreAccept(pkNetCore, pkNetSocket);
			continue;
		}

		if(pkNetSocket->iTypeSocket == CLIENT_TCP_TYPE_SOCKET)
		{
			//accpet
			continue;
		}

		if(pkNetSocket->iTypeSocket == ACCEPT_TCP_TYPE_SOCKET)
		{
			//accpet
			continue;
		}

		if(akEvents[i].events & EPOLLIN)
		{
			rkTransmit.AddEvent(SE_READ);
		}

		if(akEvents[i].events & EPOLLOUT)
		{
			rkTransmit.AddEvent(SE_WRITE);
		}

#ifndef EPOLLRDHUP
		if((akEvents[i].events &  EPOLLERR) || (akEvents[i].events &  EPOLLHUP))
		{
			rkTransmit.SetState(STS_READY_CLOSE);
		}
#else
		if((akEvents[i].events & EPOLLRDHUP) || (akEvents[i].events &  EPOLLERR) || (akEvents[i].events &  EPOLLHUP))
		{
			rkTransmit.SetState(STS_READY_CLOSE);
		}
#endif
	}

	return true;
}

#endif
