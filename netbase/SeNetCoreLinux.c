#include "SeNetCore.h"

#if defined(__linux)

#define SENETCORE_MAX_SOCKET_BUF_LEN 1024*1024*4

void SeNetCoreInit(struct SENETCORE *pkNetCore, char *pcLogName, unsigned short usMax)
{
	SeNetBaseInit();
	pkNetCore->kHandle = epoll_create(usMax);
	SeInitLog(&pkNetCore->kLog, pcLogName);
	SeNetSocketMgrInit(&pkNetCore->kSocketMgr, usMax);
	SeAddLogLV(&pkNetCore->kLog, LT_PRINT);
	SeAddLogLV(&pkNetCore->kLog, LT_SOCKET);
	pkNetCore->pcBuf = (char*)malloc(SENETCORE_MAX_SOCKET_BUF_LEN);
}

void SeNetCoreFin(struct SENETCORE *pkNetCore)
{
	SeCloseHandle(pkNetCore->kHandle);
	SeFinLog(&pkNetCore->kLog);
	SeNetSocketMgrFin(&pkNetCore->kSocketMgr);
	SeNetBaseEnd();
	free(pkNetCore->pcBuf);
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
	if(SeSetNoBlock(socket, true) != 0)
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
	if(SeSetNoBlock(socket, true) != 0)
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

	pkNetSocket = SeNetSocketMgrGet(&pkNetCore->kSocketMgr, kHSocket);
	if(iResult == 0)
	{
		pkNetSocket->usStatus = SOCKET_STATUS_CONNECTED;
		SeNetSocketMgrAddSendOrRecvInList(&pkNetCore->kSocketMgr, pkNetSocket, true);
		return kHSocket; 
	}

	kEvent.data.u64 = kHSocket;
	kEvent.events = EPOLLOUT | EPOLLRDHUP | EPOLLERR | EPOLLONESHOT;
	if(epoll_ctl(pkNetCore->kHandle, EPOLL_CTL_ADD, socket, &kEvent) != 0)
	{
		iErrorno = SeErrno();
		SeCloseSocket(socket);
		SeNetSocketMgrDel(&pkNetCore->kSocketMgr, kHSocket);
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP CLIENT] epoll_ctl ERROR, errno=%d IP=%s port=%d", iErrorno, pcIP, usPort);
		return 0;
	}
	
	memcpy(&pkNetSocket->kRemoteAddr, &kAddr, sizeof(struct sockaddr));
	pkNetSocket->usStatus = SOCKET_STATUS_CONNECTING;
	return kHSocket;
}

bool SeNetCoreSend(struct SENETCORE *pkNetCore, HSOCKET kHSocket, char* pcBuf, int iSize)
{
	bool bRet;
	struct SESOCKET *pkNetSocket;
	
	if(iSize > (0xFFFF*2) || iSize < 0) { return false; }
	pkNetSocket = SeNetSocketMgrGet(&pkNetCore->kSocketMgr, kHSocket);
	if(!pkNetSocket) return false;
	if(pkNetSocket->iTypeSocket != CLIENT_TCP_TYPE_SOCKET && pkNetSocket->iTypeSocket != ACCEPT_TCP_TYPE_SOCKET) return false;
	if(pkNetSocket->usStatus != SOCKET_STATUS_ACTIVECONNECT) return false;
	SeNetSocketMgrUpdateNetStreamIdle(&pkNetCore->kSocketMgr, pkNetSocket->iHeaderLen, iSize);
	bRet = SeNetSreamWrite(&pkNetSocket->kSendNetStream, &pkNetCore->kSocketMgr.kNetStreamIdle, pkNetSocket->pkSetHeaderLenFun, pkNetSocket->iHeaderLen, pcBuf, iSize);
	if(bRet) { SeNetSocketMgrAddSendOrRecvInList(&pkNetCore->kSocketMgr, pkNetSocket, true); }
	else { SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[CORE SEND] send data ERROR"); }
	return bRet;
}

void SeNetCoreDisconnect(struct SENETCORE *pkNetCore, HSOCKET kHSocket)
{
	SOCKET socket;
	struct epoll_event kEvent;
	struct SESOCKET *pkNetSocket;

	pkNetSocket = SeNetSocketMgrGet(&pkNetCore->kSocketMgr, kHSocket);
	if(!pkNetSocket) return;
	if(pkNetSocket->iTypeSocket != CLIENT_TCP_TYPE_SOCKET && pkNetSocket->iTypeSocket != ACCEPT_TCP_TYPE_SOCKET) return;

	if(pkNetSocket->usStatus == SOCKET_STATUS_CONNECTING)
	{
		pkNetSocket->usStatus = SOCKET_STATUS_CONNECTED_FAILED;
	}
	else if(pkNetSocket->usStatus == SOCKET_STATUS_ACTIVECONNECT)
	{
		pkNetSocket->usStatus = SOCKET_STATUS_DISCONNECT;
	}
	else
	{
		return;
	}

	socket = SeGetSocketByHScoket(kHSocket);
	epoll_ctl(pkNetCore->kHandle, EPOLL_CTL_DEL, socket, &kEvent);
	SeCloseSocket(socket);
	SeNetSocketMgrAddSendOrRecvInList(&pkNetCore->kSocketMgr, pkNetSocket, true);
}

bool SeNetCoreRecvBuf(struct SENETCORE *pkNetCore, struct SESOCKET *pkNetSocket)
{
	int iLen;
	bool bOk;
	int iErrorno;
	SOCKET socket;

	socket = SeGetSocketByHScoket(pkNetSocket->kHSocket);

	while(true)
	{
		iLen = SeRecv(socket, pkNetCore->pcBuf, SENETCORE_MAX_SOCKET_BUF_LEN, 0);
		if(iLen == 0)
		{
			return false;
		}
		else if(iLen < 0)
		{
			iErrorno = SeErrno();
			if(iErrorno == SE_EINTR) continue;
			else if(iErrorno == SE_EWOULDBLOCK) { break; }
			else { return false; }
		}
		else
		{
			SeNetSocketMgrUpdateNetStreamIdle(&pkNetCore->kSocketMgr, pkNetSocket->iHeaderLen, iLen);
			bOk = SeNetSreamWrite(&pkNetSocket->kRecvNetStream, &pkNetCore->kSocketMgr.kNetStreamIdle, pkNetSocket->pkSetHeaderLenFun, 0, pkNetCore->pcBuf, iLen);
			if(!bOk) { SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[CORE RECV] recv data ERROR"); return false; }
		}
	}

	return true;
}

bool SeNetCoreSendBuf(struct SENETCORE *pkNetCore, struct SESOCKET *pkNetSocket)
{
	int iLen;
	int iCount;
	int iErrorno;
	SOCKET socket;
	struct epoll_event kEvent;
	struct SENETSTREAMNODE *pkNetStreamNode;

	socket = SeGetSocketByHScoket(pkNetSocket->kHSocket);

	while(true)
	{
		pkNetStreamNode = SeNetSreamHeadPop(&pkNetSocket->kSendNetStream);
		if(!pkNetStreamNode) { break; }
		iLen = SeSend(socket, pkNetStreamNode->pkBuf + pkNetStreamNode->iReadPos, pkNetStreamNode->iWritePos - pkNetStreamNode->iReadPos, MSG_NOSIGNAL);
		if(iLen == 0)
		{
			SeNetSreamHeadAdd(&pkNetSocket->kSendNetStream, pkNetStreamNode);
			return false;
		}
		else if(iLen < 0)
		{
			iErrorno = SeErrno();
			SeNetSreamHeadAdd(&pkNetSocket->kSendNetStream, pkNetStreamNode);
			if(iErrorno == SE_EINTR) continue;
			else if(iErrorno == SE_EWOULDBLOCK) { break; }
			else { return false;}
		}
		else
		{
			pkNetStreamNode->iReadPos += iLen;
			if(pkNetStreamNode->iWritePos - pkNetStreamNode->iReadPos <= 0) { SeNetSreamHeadAdd(&pkNetCore->kSocketMgr.kNetStreamIdle, pkNetStreamNode); }
			else { SeNetSreamHeadAdd(&pkNetSocket->kSendNetStream, pkNetStreamNode); break; }
		}
	}

	iCount = SeNetSreamCount(&pkNetSocket->kSendNetStream);

	if(iCount <= 0 && SeNetSocketMgrHasEvent(pkNetSocket, WRITE_EVENT_SOCKET))
	{
		SeNetSocketMgrClearEvent(pkNetSocket, WRITE_EVENT_SOCKET);
		kEvent.data.u64 = pkNetSocket->kHSocket;
		kEvent.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLET;
		epoll_ctl(pkNetCore->kHandle, EPOLL_CTL_MOD, socket, &kEvent);
	}
	else if(iCount > 0 && !SeNetSocketMgrHasEvent(pkNetSocket, WRITE_EVENT_SOCKET))
	{
		SeNetSocketMgrAddEvent(pkNetSocket, WRITE_EVENT_SOCKET);
		kEvent.data.u64 = pkNetSocket->kHSocket;
		kEvent.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLERR | EPOLLET;
		epoll_ctl(pkNetCore->kHandle, EPOLL_CTL_MOD, socket, &kEvent);
	}

	return true;
}

void SeNetCoreListenSocket(struct SENETCORE *pkNetCore, struct SESOCKET *pkNetSocketListen)
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
		if(SeSetNoBlock(kSocket, true) != 0)
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
		memcpy(&pkNetSocketAccept->kRemoteAddr, &ksockaddr, sizeof(struct sockaddr));
		pkNetSocketAccept->usStatus = SOCKET_STATUS_CONNECTED;
		pkNetSocketAccept->kBelongListenHSocket = pkNetSocketListen->kHSocket;
		SeNetSocketMgrAddSendOrRecvInList(&pkNetCore->kSocketMgr, pkNetSocketAccept, true);
	}
}

void SeNetCoreClientSocket(struct SENETCORE *pkNetCore, struct SESOCKET *pkNetSocket, bool bRead, bool bWrite, bool bError)
{
	bool bOK;
	int iLen;
	int iErrorno;
	SOCKET socket;
	struct epoll_event kEvent;

	socket = SeGetSocketByHScoket(pkNetSocket->kHSocket);

	if(pkNetSocket->usStatus == SOCKET_STATUS_CONNECTING)
	{
		bOK = true;

		if(bWrite == true)
		{
			iErrorno = -1;
			iLen = sizeof(int);
			SeGetSockOpt(socket, SOL_SOCKET, SO_ERROR, (char*)&iErrorno, (SOCK_LEN*)&iLen);
			if(iErrorno == 0)
			{
				pkNetSocket->usStatus = SOCKET_STATUS_CONNECTED;
				epoll_ctl(pkNetCore->kHandle, EPOLL_CTL_DEL, socket, &kEvent);
				SeNetSocketMgrAddSendOrRecvInList(&pkNetCore->kSocketMgr, pkNetSocket, true);
				return;
			}
			bOK = false;
		}

		if(bError == true || bOK == false)
		{
			pkNetSocket->usStatus = SOCKET_STATUS_CONNECTED_FAILED;
			epoll_ctl(pkNetCore->kHandle, EPOLL_CTL_DEL, socket, &kEvent);
			SeCloseSocket(socket);
			SeNetSocketMgrAddSendOrRecvInList(&pkNetCore->kSocketMgr, pkNetSocket, true);
			return;
		}

		// no call here?
		assert(0 != 0);
		return;
	}

	if(bError == true) { SeNetCoreDisconnect(pkNetCore, pkNetSocket->kHSocket); return; }

	if(bRead == true)
	{
		bOK = SeNetCoreRecvBuf(pkNetCore, pkNetSocket);
		if(!bOK) { SeNetCoreDisconnect(pkNetCore, pkNetSocket->kHSocket); return; }
		SeNetSocketMgrAddSendOrRecvInList(&pkNetCore->kSocketMgr, pkNetSocket, false);
	}

	if(bWrite == true)
	{
		bOK = SeNetCoreSendBuf(pkNetCore, pkNetSocket);
		if(!bOK) { SeNetCoreDisconnect(pkNetCore, pkNetSocket->kHSocket); return; }
	}
}

void SeNetCoreAcceptSocket(struct SENETCORE *pkNetCore, struct SESOCKET *pkNetSocket, bool bRead, bool bWrite, bool bError)
{
	bool bOK;

	if(bError == true) { SeNetCoreDisconnect(pkNetCore, pkNetSocket->kHSocket); return; }

	if(bRead == true)
	{
		bOK = SeNetCoreRecvBuf(pkNetCore, pkNetSocket);
		if(!bOK) { SeNetCoreDisconnect(pkNetCore, pkNetSocket->kHSocket); return; }
		SeNetSocketMgrAddSendOrRecvInList(&pkNetCore->kSocketMgr, pkNetSocket, false);
	}

	if(bWrite == true)
	{
		bOK = SeNetCoreSendBuf(pkNetCore, pkNetSocket);
		if(!bOK) { SeNetCoreDisconnect(pkNetCore, pkNetSocket->kHSocket); return; }
	}
}

bool SeNetCoreProcess(struct SENETCORE *pkNetCore, int *riEventSocket, HSOCKET *rkListenHSocket, HSOCKET *rkHSocket, char *pcBuf, int *riLen, int *rSSize, int *rRSize)
{
	bool bOK;
	char *pcAddrIP;
	SOCKET socket;
	struct epoll_event kEvent;
	struct SESOCKET *pkNetSocket;

	do
	{
		pkNetSocket = SeNetSocketMgrPopSendOrRecvOutList(&pkNetCore->kSocketMgr, true);
		if(!pkNetSocket) { break; }

		*rkHSocket = pkNetSocket->kHSocket;
		*rkListenHSocket = pkNetSocket->kBelongListenHSocket;
		
		if(pkNetSocket->usStatus == SOCKET_STATUS_CONNECTED)
		{
			SeNetSocketMgrClearEvent(pkNetSocket, READ_EVENT_SOCKET);
			SeNetSocketMgrClearEvent(pkNetSocket, WRITE_EVENT_SOCKET);
			socket = SeGetSocketByHScoket(pkNetSocket->kHSocket);
			kEvent.data.u64 = pkNetSocket->kHSocket;
			kEvent.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLERR | EPOLLHUP | EPOLLET;
			epoll_ctl(pkNetCore->kHandle, EPOLL_CTL_ADD, socket, &kEvent);
			*riLen = 0;
			pcAddrIP = inet_ntoa(pkNetSocket->kRemoteAddr.sin_addr);
			if(pcAddrIP) { strcpy(pcBuf, pcAddrIP); *riLen = (int)strlen(pcAddrIP); }
			*riEventSocket = SENETCORE_EVENT_SOCKET_CONNECT;
			pkNetSocket->usStatus = SOCKET_STATUS_ACTIVECONNECT;
			return true;
		}
		
		if(pkNetSocket->usStatus == SOCKET_STATUS_CONNECTED_FAILED)
		{
			assert(pkNetSocket->iTypeSocket == CLIENT_TCP_TYPE_SOCKET);
			SeNetSocketMgrDel(&pkNetCore->kSocketMgr, pkNetSocket->kHSocket);
			*riEventSocket = SENETCORE_EVENT_SOCKET_CONNECT_FAILED;
			return true;
		}

		if(pkNetSocket->usStatus == SOCKET_STATUS_DISCONNECT)
		{
			SeNetSocketMgrDel(&pkNetCore->kSocketMgr, pkNetSocket->kHSocket);
			*riEventSocket = SENETCORE_EVENT_SOCKET_DISCONNECT;
			return true;
		}

		if(pkNetSocket->usStatus == SOCKET_STATUS_ACTIVECONNECT && SeNetSreamCount(&pkNetSocket->kSendNetStream) > 0)
		{
			bOK = SeNetCoreSendBuf(pkNetCore, pkNetSocket);
			if(!bOK) { SeNetCoreDisconnect(pkNetCore, pkNetSocket->kHSocket); continue; }
		}
	}while(true);
	
	do
	{
		pkNetSocket = SeNetSocketMgrPopSendOrRecvOutList(&pkNetCore->kSocketMgr, false);
		if(!pkNetSocket) { break; }
		if(pkNetSocket->usStatus != SOCKET_STATUS_ACTIVECONNECT) { continue; }
		bOK = SeNetSreamRead(&pkNetSocket->kRecvNetStream, &pkNetCore->kSocketMgr.kNetStreamIdle, pkNetSocket->pkGetHeaderLenFun, pkNetSocket->iHeaderLen, pcBuf, riLen);
		if(!bOK) { continue; }
		
		*rkHSocket = pkNetSocket->kHSocket;
		*riEventSocket = SENETCORE_EVENT_SOCKET_RECV_DATA;
		*rkListenHSocket = pkNetSocket->kBelongListenHSocket;
		*rSSize = SeNetSreamCount(&pkNetSocket->kSendNetStream);
		*rRSize = SeNetSreamCount(&pkNetSocket->kRecvNetStream);
		SeNetSocketMgrAddSendOrRecvInList(&pkNetCore->kSocketMgr, pkNetSocket, false);
		return true;
	}while(true);

	return false;
}

bool SeNetCoreRead(struct SENETCORE *pkNetCore, int *riEvent, HSOCKET *rkListenHSocket, HSOCKET *rkHSocket, char *pcBuf, int *riLen, int *rSSize, int *rRSize)
{
	bool bWork;
	int i, iNum;
	HSOCKET kHSocket;
	bool bRead, bWrite, bError;
	struct epoll_event *pkEvent;
	struct SESOCKET *pkNetSocket;

	memset(pkNetCore->akEvents, 0, sizeof(pkNetCore->akEvents)/sizeof(struct epoll_event));
	iNum = epoll_wait(pkNetCore->kHandle, pkNetCore->akEvents, sizeof(pkNetCore->akEvents)/sizeof(struct epoll_event), 0);
	bWork = iNum > 0 ? true : false;

	for(i = 0; i < iNum; i++)
	{
		pkEvent = &pkNetCore->akEvents[i];
		bRead = pkEvent->events & EPOLLIN;
		bWrite =pkEvent->events & EPOLLOUT;
		kHSocket = (HSOCKET)pkEvent->data.u64;
		bError = (pkEvent->events & EPOLLRDHUP) || (pkEvent->events &  EPOLLERR) || (pkEvent->events &  EPOLLHUP);
		pkNetSocket = SeNetSocketMgrGet(&pkNetCore->kSocketMgr, kHSocket);
		if(!pkNetSocket) { SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[EPOLL WAIT] socket not found"); continue; }
		if(pkNetSocket->iTypeSocket == LISTEN_TCP_TYPE_SOCKET) { SeNetCoreListenSocket(pkNetCore, pkNetSocket); }
		if(pkNetSocket->iTypeSocket == CLIENT_TCP_TYPE_SOCKET) { SeNetCoreClientSocket(pkNetCore, pkNetSocket, bRead, bWrite, bError); }
		if(pkNetSocket->iTypeSocket == ACCEPT_TCP_TYPE_SOCKET) { SeNetCoreAcceptSocket(pkNetCore, pkNetSocket, bRead, bWrite, bError); }
	}

	if(SeNetCoreProcess(pkNetCore, riEvent, rkListenHSocket, rkHSocket, pcBuf, riLen, rSSize, rRSize)) { return true; }
	
	*riEvent = SENETCORE_EVENT_SOCKET_IDLE;
	return !bWork;
}

#endif
