#include "SeNetCore.h"

#if defined(__linux)

void SeNetCoreInit(struct SENETCORE *pkNetCore, char *pcLogName, unsigned short usMax)
{
	SeNetBaseInit();
	pkNetCore->kHandle = epoll_create(usMax);
	SeInitLog(&pkNetCore->kLog, pcLogName);
	SeNetSocketMgrInit(&pkNetCore->kSocketMgr, usMax);
	SeAddLogLV(&pkNetCore->kLog, LT_PRINT);
}

void SeNetCoreFin(struct SENETCORE *pkNetCore)
{
	SeCloseHandle(pkNetCore->kHandle);
	SeFinLog(&pkNetCore->kLog);
	SeNetSocketMgrFin(&pkNetCore->kSocketMgr);
	SeNetBaseEnd();
}

HSOCKET SeNetCoreTCPListen(struct SENETCORE *pkNetCore, const char *pcIP, unsigned short usPort,\
				int iHeaderLen, SEGETHEADERLENFUN pkGetHeaderLenFun, SESETHEADERLENFUN pkSetHeaderLenFun)
{
	int backlog;
	int iErrorno;
	SOCKET socket;
	HSOCKET kHSocket;
	struct sockaddr kAddr;
	struct epoll_event kEvent;
	
	SeSetSockAddr(&kAddr, pcIP, usPort);
	socket = SeSocket(SOCK_STREAM);
	if(socket == SE_INVALID_SOCKET)
	{
		iErrorno = SeErrno();
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP LISTEN] Create Socket ERROR, errno=%d IP=%s port=%d", iErrorno, pcIP, usPort);
		return 0;
	}
	if(SeSetNoBlock(socket) != 0)
	{
		iErrorno = SeErrno();
		SeCloseSocket(socket);
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP LISTEN] SeSetNoBlock ERROR, errno=%d IP=%s port=%d", iErrorno, pcIP, usPort);
		return 0;
	}
	if(SeSetReuseAddr(socket) != 0)
	{
		iErrorno = SeErrno();
		SeCloseSocket(socket);
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP LISTEN] SeSetReuseAddr ERROR, errno=%d IP=%s port=%d", iErrorno, pcIP, usPort);
		return 0;
	}
	if(SeBind(socket, &kAddr) != 0)
	{
		iErrorno = SeErrno();
		SeCloseSocket(socket);
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP LISTEN] SeBind ERROR, errno=%d IP=%s port=%d", iErrorno, pcIP, usPort);
		return 0;
	}
	backlog = 64;
	if(SeListen(socket, backlog) != 0)
	{
		iErrorno = SeErrno();
		SeCloseSocket(socket);
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP LISTEN] SeListen ERROR, errno=%d backlog=%d IP=%s port=%d", iErrorno, backlog, pcIP, usPort);
		return 0;
	}

	kHSocket = SeNetSocketMgrAdd(&pkNetCore->kSocketMgr, socket, LISTEN_TCP_TYPE_SOCKET, iHeaderLen, pkGetHeaderLenFun, pkSetHeaderLenFun);
	if(kHSocket <= 0)
	{
		SeCloseSocket(socket);
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP LISTEN] SocketMgr is full, IP=%s port=%d", pcIP, usPort);
		return 0;
	}

	kEvent.data.u64 = kHSocket;
	kEvent.events = EPOLLIN | EPOLLOUT;
	if(epoll_ctl(pkNetCore->kHandle, EPOLL_CTL_ADD, socket, &kEvent) != 0)
	{
		iErrorno = SeErrno();
		SeCloseSocket(socket);
		SeNetSocketMgrDel(&pkNetCore->kSocketMgr, kHSocket);
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP LISTEN] epoll_ctl ERROR, errno=%d IP=%s port=%d", iErrorno, pcIP, usPort);
		return 0;
	}

	return kHSocket;
}

HSOCKET SeNetCoreTCPClient(struct SENETCORE *pkNetCore, const char *pcIP, unsigned short usPort,\
				int iHeaderLen, SEGETHEADERLENFUN pkGetHeaderLenFun, SESETHEADERLENFUN pkSetHeaderLenFun)
{
	int iResult;
	int iErrorno;
	SOCKET socket;
	HSOCKET kHSocket;
	struct sockaddr kAddr;
	struct epoll_event kEvent;
	struct SESOCKET *pkNetSocket;
	
	SeSetSockAddr(&kAddr, pcIP, usPort);
	socket = SeSocket(SOCK_STREAM);
	if(socket == SE_INVALID_SOCKET)
	{
		iErrorno = SeErrno();
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP CLIENT] Create Socket ERROR, errno=%d IP=%s port=%d", iErrorno, pcIP, usPort);
		return 0;
	}
	if(SeSetNoBlock(socket) != 0)
	{
		iErrorno = SeErrno();
		SeCloseSocket(socket);
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP CLIENT] SeSetNoBlock ERROR, errno=%d IP=%s port=%d", iErrorno, pcIP, usPort);
		return 0;
	}

	kHSocket = SeNetSocketMgrAdd(&pkNetCore->kSocketMgr, socket, CLIENT_TCP_TYPE_SOCKET, iHeaderLen, pkGetHeaderLenFun, pkSetHeaderLenFun);
	if(kHSocket <= 0)
	{
		SeCloseSocket(socket);
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP CLIENT] SocketMgr is full, IP=%s port=%d", pcIP, usPort);
		return 0;
	}
	
	iResult = SeConnect(socket, &kAddr);
	iErrorno = SeErrno();
	if(iResult != 0 && iErrorno != SE_EINPROGRESS)
	{
		SeCloseSocket(socket);
		SeNetSocketMgrDel(&pkNetCore->kSocketMgr, kHSocket);
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP CLIENT] SeConnect ERROR, errno=%d IP=%s port=%d", iErrorno, pcIP, usPort);
		return 0;
	}

	kEvent.data.u64 = kHSocket;
	kEvent.events = EPOLLOUT | EPOLLRDHUP | EPOLLPRI | EPOLLONESHOT;
	if(epoll_ctl(pkNetCore->kHandle, EPOLL_CTL_ADD, socket, &kEvent) != 0)
	{
		iErrorno = SeErrno();
		SeCloseSocket(socket);
		SeNetSocketMgrDel(&pkNetCore->kSocketMgr, kHSocket);
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP CLIENT] epoll_ctl ERROR, errno=%d IP=%s port=%d", iErrorno, pcIP, usPort);
		return 0;
	}
	
	pkNetSocket = SeNetSocketMgrGet(&pkNetCore->kSocketMgr, kHSocket);
	if(iResult == 0) { pkNetSocket->usStatus = SOCKET_STATUS_CONNECTED; }
	else { pkNetSocket->usStatus = SOCKET_STATUS_CONNECTING; }
	SeNetSocketMgrAddSendOrRecvInList(&pkNetCore->kSocketMgr, pkNetSocket, true);
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
	int iErrorno;
	SOCKET socket;
	struct epoll_event kEvent;
	struct SESOCKET *pkNetSocket;

	pkNetSocket = SeNetSocketMgrGet(&pkNetCore->kSocketMgr, kHSocket);
	if(!pkNetSocket) return;
	if(pkNetSocket->iTypeSocket != CLIENT_TCP_TYPE_SOCKET && pkNetSocket->iTypeSocket != ACCEPT_TCP_TYPE_SOCKET) return;
	if(pkNetSocket->usStatus != SOCKET_STATUS_ACTIVECONNECT) return;
	pkNetSocket->usStatus = SOCKET_STATUS_DISCONNECTING;
	socket = SeGetSocketByHScoket(kHSocket);
	if(epoll_ctl(pkNetCore->kHandle, EPOLL_CTL_DEL, socket, &kEvent) != 0)
	{
		iErrorno = SeErrno();
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[DISCONNECT] epoll_ctl del ERROR, errno=%d", iErrorno);
	}
	SeShutDown(socket);
	SeCloseSocket(socket);
	SeNetSocketMgrAddSendOrRecvInList(&pkNetCore->kSocketMgr, pkNetSocket, true);
}

void SeNetCoreAccept(struct SENETCORE *pkNetCore, struct SESOCKET *pkNetSocketListen)
{
	int iErrorno;
	HSOCKET kHSocket;
	SOCKET kSocket, kListenSocket;
	struct sockaddr ksockaddr;
	struct SESOCKET *pkNetSocketAccept;
	
	kListenSocket = SeGetSocketByHScoket(pkNetSocketListen->kHSocket);
	while(true)
	{
		kSocket = SeAccept(kListenSocket, &ksockaddr);
		if(kSocket == SE_INVALID_SOCKET)
		{
			iErrorno = SeErrno();
			if(iErrorno == SE_EINTR)
			{
				continue;
			}
			if(iErrorno != SE_EWOULDBLOCK)
			{
				SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[ACCEPT SOCKET] SeAccept ERROR, errno=%d", iErrorno);
			}
			break;
		}
		if(SeSetNoBlock(kSocket) != 0)
		{
			iErrorno = SeErrno();
			SeCloseSocket(kSocket);
			SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[ACCEPT SOCKET] SeSetNoBlock ERROR, errno=%d", iErrorno);
			continue;
		}
		kHSocket = SeNetSocketMgrAdd(&pkNetCore->kSocketMgr, kSocket, ACCEPT_TCP_TYPE_SOCKET, \
			pkNetSocketListen->iHeaderLen, pkNetSocketListen->pkGetHeaderLenFun, pkNetSocketListen->pkSetHeaderLenFun);
		if(kHSocket <= 0)
		{
			SeCloseSocket(kSocket);
			SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[ACCEPT SOCKET] SocketMgr is full");
			continue;
		}
		pkNetSocketAccept = SeNetSocketMgrGet(&pkNetCore->kSocketMgr, kHSocket);
		pkNetSocketAccept->usStatus = SOCKET_STATUS_ACCEPT;
		pkNetSocketAccept->kBelongListenHSocket = pkNetSocketListen->kHSocket;
		SeNetSocketMgrAddSendOrRecvInList(&pkNetCore->kSocketMgr, pkNetSocketAccept, true);
	}
}

bool SeNetCoreRead(struct SENETCORE *pkNetCore, int *riEvent, HSOCKET *rkHSocket, char *pcBuf, int *riLen)
{
	int i;
	int iNum;
	HSOCKET kHSocket;
	struct SESOCKET *pkNetSocket;

	memset(pkNetCore->akEvents, 0, sizeof(pkNetCore->akEvents)/sizeof(struct epoll_event));
	iNum = epoll_wait(pkNetCore->kHandle, pkNetCore->akEvents, sizeof(pkNetCore->akEvents)/sizeof(struct epoll_event), 0);
	if(iNum <= 0) return false;

	for(int i = 0; i < iNum; i++)
	{
		kHSocket = (HSOCKET)pkNetCore->akEvents[i].data.u64;
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

		if(pkNetCore->akEvents[i].events & EPOLLIN)
		{
			rkTransmit.AddEvent(SE_READ);
		}

		if(pkNetCore->akEvents[i].events & EPOLLOUT)
		{
			rkTransmit.AddEvent(SE_WRITE);
		}

		if((pkNetCore->akEvents[i].events & EPOLLRDHUP) || (pkNetCore->akEvents[i].events &  EPOLLERR) || (pkNetCore->akEvents[i].events &  EPOLLHUP))
		{
			rkTransmit.SetState(STS_READY_CLOSE);
		}
	}

	return true;
}

#endif
