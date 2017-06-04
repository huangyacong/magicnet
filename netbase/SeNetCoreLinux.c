#include "SeNetCore.h"

#if defined(__linux)

void SeNetCoreInit(struct SENETCORE *pkNetCore, char *pcLogName, unsigned short usMax, int iLogLV)
{
	pkNetCore->iWaitTime = NET_CORE_WAIT_TIME;
	SeNetBaseInit();
	pkNetCore->kHandle = epoll_create(usMax);
	SeInitLog(&pkNetCore->kLog, pcLogName);
	SeAddLogLV(&pkNetCore->kLog, iLogLV);
	SeNetSocketMgrInit(&pkNetCore->kSocketMgr, usMax);
}

void SeNetCoreFin(struct SENETCORE *pkNetCore)
{
	SeCloseHandle(pkNetCore->kHandle);
	SeFinLog(&pkNetCore->kLog);
	SeNetSocketMgrFin(&pkNetCore->kSocketMgr);
	SeNetBaseEnd();
}

void SeNetCoreSetLogContextFunc(struct SENETCORE *pkNetCore, SELOGCONTEXT pkLogContextFunc, void *pkLogContect)
{
	SeLogSetLogContextFunc(&pkNetCore->kLog, pkLogContextFunc, pkLogContect);
}

void SeNetCoreSetWaitTime(struct SENETCORE *pkNetCore, unsigned int uiWaitTime)
{
	pkNetCore->iWaitTime = uiWaitTime;
}

HSOCKET SeNetCoreTCPListen(struct SENETCORE *pkNetCore, const char *pcIP, unsigned short usPort,\
	int iHeaderLen, int iTimeOut, SEGETHEADERLENFUN pkGetHeaderLenFun, SESETHEADERLENFUN pkSetHeaderLenFun)
{
	int backlog;
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
	if(SeSetExclusiveAddruse(socket) != 0)
	{
		iErrorno = SeErrno();
		SeCloseSocket(socket);
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP LISTEN] SeSetExclusiveAddruse ERROR, errno=%d IP=%s port=%d", iErrorno, pcIP, usPort);
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
	backlog = SENETCORE_SOCKET_BACKLOG;
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

	SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP LISTEN] SocketMgr listen ok, IP=%s port=%d", pcIP, usPort);
	pkNetSocket = SeNetSocketMgrGet(&pkNetCore->kSocketMgr, kHSocket);
	pkNetSocket->usStatus = SOCKET_STATUS_ACTIVECONNECT;
	pkNetSocket->iActiveTimeOut = iTimeOut;
	memcpy(&pkNetSocket->kRemoteAddr, &kAddr, sizeof(struct sockaddr));

	return kHSocket;
}

HSOCKET SeNetCoreTCPClient(struct SENETCORE *pkNetCore, const char *pcIP, unsigned short usPort,\
	int iHeaderLen, int iTimeOut, int iConnectTimeOut, SEGETHEADERLENFUN pkGetHeaderLenFun, SESETHEADERLENFUN pkSetHeaderLenFun)
{
	int iResult;
	int iErrorno;
	SOCKET socket;
	HSOCKET kHSocket;
	struct sockaddr kAddr;
	struct linger so_linger;
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

	so_linger.l_onoff = 1;
	so_linger.l_linger = 0;
	if(SeSetSockOpt(socket, SOL_SOCKET, SO_LINGER, (const char *)&so_linger, sizeof(so_linger)) != 0)
	{
		iErrorno = SeErrno();
		SeCloseSocket(socket);
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP CLIENT] SeSetSockOpt ERROR, errno=%d IP=%s port=%d", iErrorno, pcIP, usPort);
		return 0;
	}

	if(SeSetNoDelay(socket) != 0)
	{
		iErrorno = SeErrno();
		SeCloseSocket(socket);
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP CLIENT] SeSetNoDelay ERROR, errno=%d IP=%s port=%d", iErrorno, pcIP, usPort);
		return 0;
	}

	if(SeSetReuseAddr(socket) != 0)
	{
		iErrorno = SeErrno();
		SeCloseSocket(socket);
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP CLIENT] SeSetReuseAddr ERROR, errno=%d IP=%s port=%d", iErrorno, pcIP, usPort);
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
	if(iResult != 0 && iErrorno != SE_EINPROGRESS && iErrorno != SE_EINTR)
	{
		SeCloseSocket(socket);
		SeNetSocketMgrDel(&pkNetCore->kSocketMgr, kHSocket);
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP CLIENT] SeConnect ERROR, errno=%d IP=%s port=%d socket=%llx", iErrorno, pcIP, usPort, kHSocket);
		return 0;
	}

	pkNetSocket = SeNetSocketMgrGet(&pkNetCore->kSocketMgr, kHSocket);
	if(iResult == 0)
	{
		memcpy(&pkNetSocket->kRemoteAddr, &kAddr, sizeof(struct sockaddr));
		pkNetSocket->usStatus = SOCKET_STATUS_CONNECTED;
		pkNetSocket->llTime = SeTimeGetTickCount();
		SeNetSocketMgrAddSendOrRecvInList(&pkNetCore->kSocketMgr, pkNetSocket, true);
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP CLIENT] ConnectEx to svr, ip=%s port=%d socket=%llx", inet_ntoa(pkNetSocket->kRemoteAddr.sin_addr), ntohs(pkNetSocket->kRemoteAddr.sin_port), kHSocket);
		return kHSocket; 
	}

	kEvent.data.u64 = kHSocket;
	kEvent.events = EPOLLOUT | EPOLLRDHUP | EPOLLERR | EPOLLONESHOT;
	if(epoll_ctl(pkNetCore->kHandle, EPOLL_CTL_ADD, socket, &kEvent) != 0)
	{
		iErrorno = SeErrno();
		SeCloseSocket(socket);
		SeNetSocketMgrDel(&pkNetCore->kSocketMgr, kHSocket);
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP CLIENT] epoll_ctl ERROR, errno=%d IP=%s port=%d socket=%llx", iErrorno, pcIP, usPort, kHSocket);
		return 0;
	}
	
	memcpy(&pkNetSocket->kRemoteAddr, &kAddr, sizeof(struct sockaddr));
	pkNetSocket->usStatus = SOCKET_STATUS_CONNECTING;
	pkNetSocket->iActiveTimeOut = iTimeOut;
	pkNetSocket->iConnectTimeOut = iConnectTimeOut;
	SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP CLIENT] ConnectEx to svr, ip=%s port=%d socket=%llx", inet_ntoa(pkNetSocket->kRemoteAddr.sin_addr), ntohs(pkNetSocket->kRemoteAddr.sin_port), kHSocket);
	return kHSocket;
}

bool SeNetCoreSend(struct SENETCORE *pkNetCore, HSOCKET kHSocket, const char* pcBuf, int iSize)
{
	bool bRet;
	int iHeaderLen;
	struct SESOCKET *pkNetSocket;
	struct SENETSTREAM *pkSendNetStream;
	SESETHEADERLENFUN pkSetHeaderLenFun;
	
	if(iSize < 0) { SeLogWrite(&pkNetCore->kLog, LT_ERROR, true, "[CORE SEND] Send Data Error. Socket=%llx.Size=%d", kHSocket, iSize); return false; }
	pkNetSocket = SeNetSocketMgrGet(&pkNetCore->kSocketMgr, kHSocket);
	if(!pkNetSocket) return false;
	iHeaderLen = pkNetSocket->iHeaderLen;
	pkSetHeaderLenFun = pkNetSocket->pkSetHeaderLenFun;
	if(pkNetSocket->iTypeSocket != CLIENT_TCP_TYPE_SOCKET && pkNetSocket->iTypeSocket != ACCEPT_TCP_TYPE_SOCKET) return false;
	if(pkNetSocket->usStatus != SOCKET_STATUS_ACTIVECONNECT) return false;
	pkSendNetStream = &pkNetSocket->kSendNetStream;
	if(!SeNetSocketMgrUpdateNetStreamIdle(&pkNetCore->kSocketMgr, iHeaderLen, iSize)) { SeLogWrite(&pkNetCore->kLog, LT_ERROR, true, "[CORE SEND] no more memcahce.socket=%llx", kHSocket); return false; }
	if(!SeNetSreamCanWrite(&pkNetSocket->kSendNetStream, pkNetSocket->pkSetHeaderLenFun, pkNetSocket->iHeaderLen, iSize)) { SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[CORE SEND] send data ERROR.socket=%llx.size=%d", kHSocket, iSize); return false; }
	bRet = SeNetSreamWrite(pkSendNetStream, pkNetCore->kSocketMgr.pkNetStreamIdle, pkSetHeaderLenFun, iHeaderLen, pcBuf, iSize);
	if(bRet) { SeNetSocketMgrAddSendOrRecvInList(&pkNetCore->kSocketMgr, pkNetSocket, true); }
	else { SeLogWrite(&pkNetCore->kLog, LT_ERROR, true, "[CORE SEND] Send Data Error, Now Close The Socket=%llx.Size=%d", kHSocket, iSize); SeNetCoreDisconnect(pkNetCore, pkNetSocket->kHSocket); }

	return bRet;
}

bool SeNetCoreSendExtend(struct SENETCORE *pkNetCore, HSOCKET kHSocket, const struct SENETSTREAMBUF *pkBufList, int iNum)
{
	int i;
	bool bRet;
	int iSize;
	int iHeaderLen;
	struct SESOCKET *pkNetSocket;
	struct SENETSTREAM *pkSendNetStream;
	SESETHEADERLENFUN pkSetHeaderLenFun;

	if(!pkBufList || iNum <= 0) return false;

	iSize = 0;
	for(i = 0; i < iNum; i++)
	{
		if(!pkBufList[i].pcBuf) { SeLogWrite(&pkNetCore->kLog, LT_ERROR, true, "[CORE SEND] Send Data Error, Buf Is NULL. Socket=%llx.", kHSocket); return false; }
		if(pkBufList[i].iBufLen < 0) { SeLogWrite(&pkNetCore->kLog, LT_ERROR, true, "[CORE SEND] Send Data Error, Len Is Error. Socket=%llx.", kHSocket); return false; }
		iSize += pkBufList[i].iBufLen;
	}
	
	if(iSize < 0) { SeLogWrite(&pkNetCore->kLog, LT_ERROR, true, "[CORE SEND] Send Data Error. Socket=%llx.Size=%d", kHSocket, iSize); return false; }
	pkNetSocket = SeNetSocketMgrGet(&pkNetCore->kSocketMgr, kHSocket);
	if(!pkNetSocket) return false;
	iHeaderLen = pkNetSocket->iHeaderLen;
	pkSetHeaderLenFun = pkNetSocket->pkSetHeaderLenFun;
	if(pkNetSocket->iTypeSocket != CLIENT_TCP_TYPE_SOCKET && pkNetSocket->iTypeSocket != ACCEPT_TCP_TYPE_SOCKET) return false;
	if(pkNetSocket->usStatus != SOCKET_STATUS_ACTIVECONNECT) return false;
	pkSendNetStream = &pkNetSocket->kSendNetStream;
	if(!SeNetSocketMgrUpdateNetStreamIdle(&pkNetCore->kSocketMgr, iHeaderLen, iSize)) { SeLogWrite(&pkNetCore->kLog, LT_ERROR, true, "[CORE SEND] no more memcahce.socket=%llx", kHSocket); return false; }
	if(!SeNetSreamCanWrite(&pkNetSocket->kSendNetStream, pkNetSocket->pkSetHeaderLenFun, pkNetSocket->iHeaderLen, iSize)) { SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[CORE SEND] send data ERROR.socket=%llx.size=%d", kHSocket, iSize); return false; }
	bRet = SeNetSreamWriteExtend(pkSendNetStream, pkNetCore->kSocketMgr.pkNetStreamIdle, pkSetHeaderLenFun, iHeaderLen, pkBufList, iNum);
	if(bRet) { SeNetSocketMgrAddSendOrRecvInList(&pkNetCore->kSocketMgr, pkNetSocket, true); }
	else { SeLogWrite(&pkNetCore->kLog, LT_ERROR, true, "[CORE SEND] Send Data Error, Now Close The Socket=%llx.Size=%d", kHSocket, iSize); SeNetCoreDisconnect(pkNetCore, pkNetSocket->kHSocket); }

	return bRet;
}

void SeNetCoreDisconnect(struct SENETCORE *pkNetCore, HSOCKET kHSocket)
{
	SOCKET socket;
	struct epoll_event kEvent;
	unsigned short usOrgStatus;
	struct SESOCKET *pkNetSocket;

	pkNetSocket = SeNetSocketMgrGet(&pkNetCore->kSocketMgr, kHSocket);
	if(!pkNetSocket) return;
	if(pkNetSocket->iTypeSocket != CLIENT_TCP_TYPE_SOCKET && pkNetSocket->iTypeSocket != ACCEPT_TCP_TYPE_SOCKET) return;

	usOrgStatus = pkNetSocket->usStatus;

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

	SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "SeNetCoreDisconnect. OrgStatus=%d Atatus=%d socket=%llx", usOrgStatus, pkNetSocket->usStatus, kHSocket);

	socket = SeGetSocketByHScoket(kHSocket);
	epoll_ctl(pkNetCore->kHandle, EPOLL_CTL_DEL, socket, &kEvent);
	SeCloseSocket(socket);
	SeNetSocketMgrAddSendOrRecvInList(&pkNetCore->kSocketMgr, pkNetSocket, true);
}

bool SeNetCoreRecvBuf(struct SENETCORE *pkNetCore, struct SESOCKET *pkNetSocket)
{
	int iLen;
	bool bPop;
	bool bRecv;
	int iErrorno;
	struct SENETSTREAMNODE *pkNetStreamNode;
	
	bPop = false;
	bRecv = false;

	while(true)
	{
		if(!SeNetSocketMgrUpdateNetStreamIdle(&pkNetCore->kSocketMgr, pkNetSocket->iHeaderLen, 0))
		{
			SeLogWrite(&pkNetCore->kLog, LT_ERROR, true, "[CORE RECV] no more memcahce.socket=%llx", pkNetSocket->kHSocket);
			return false;
		}

		pkNetStreamNode = SeNetSreamTailPop(&pkNetSocket->kRecvNetStream);
		if(pkNetStreamNode)
		{
			bPop = true;
			if(pkNetStreamNode->usMaxLen <= pkNetStreamNode->usWritePos)
			{
				SeNetSreamTailAdd(&pkNetSocket->kRecvNetStream, pkNetStreamNode);
				pkNetStreamNode = 0;
				bPop = false;
			}
		}

		if(!pkNetStreamNode)
		{
			pkNetStreamNode = SeNetSreamTailPop(pkNetCore->kSocketMgr.pkNetStreamIdle);
			if(pkNetStreamNode) { SeNetSreamNodeZero(pkNetStreamNode); }
		}

		if(!pkNetStreamNode)
		{
			SeLogWrite(&pkNetCore->kLog, LT_ERROR, true, "[CORE RECV] no more ilde memcahce.socket=%llx", pkNetSocket->kHSocket);
			return false;
		}

		if(pkNetStreamNode->usMaxLen <= pkNetStreamNode->usWritePos)
		{
			SeNetSreamTailAdd(pkNetCore->kSocketMgr.pkNetStreamIdle, pkNetStreamNode);
			SeLogWrite(&pkNetCore->kLog, LT_ERROR, true, "[CORE RECV] usMaxLen <= usWritePos.socket=%llx", pkNetSocket->kHSocket);
			return false;
		}
		
		iLen = SeRecv(SeGetSocketByHScoket(pkNetSocket->kHSocket), pkNetStreamNode->pkBuf + pkNetStreamNode->usWritePos, pkNetStreamNode->usMaxLen - pkNetStreamNode->usWritePos, 0);

		if(iLen == 0)
		{
			if(bPop) { SeNetSreamTailAdd(&pkNetSocket->kRecvNetStream, pkNetStreamNode); }
			else { SeNetSreamTailAdd(pkNetCore->kSocketMgr.pkNetStreamIdle, pkNetStreamNode); }
			SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[SeNetCoreRecvBuf] socket is close by client.socket=%llx", pkNetSocket->kHSocket);
			return false;
		}
		else if(iLen < 0)
		{
			iErrorno = SeErrno();
			if(bPop) { SeNetSreamTailAdd(&pkNetSocket->kRecvNetStream, pkNetStreamNode); }
			else { SeNetSreamTailAdd(pkNetCore->kSocketMgr.pkNetStreamIdle, pkNetStreamNode); }
			if(iErrorno == SE_EINTR) continue;
			else if(iErrorno == SE_EWOULDBLOCK) { break; }
			else { SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[SeNetCoreRecvBuf] socket recv error.socket=%llx", pkNetSocket->kHSocket); return false; }
		}
		else
		{
			bRecv = true;
			pkNetStreamNode->usWritePos += iLen;
			SeNetSreamTailAdd(&pkNetSocket->kRecvNetStream, pkNetStreamNode);
		}
	}

	if(bRecv) { SeNetSocketMgrActive(&pkNetCore->kSocketMgr, pkNetSocket); }

	if(SeGetNetSreamLen(&pkNetSocket->kRecvNetStream) >= SENETCORE_SOCKET_RS_BUF_LEN) 
	{
		SeLogWrite(&pkNetCore->kLog, LT_ERROR, true, "[RECV MORE BUF] RecvBuf Too More And Close It.socket=%llx.size>%d", pkNetSocket->kHSocket, SENETCORE_SOCKET_RS_BUF_LEN);
		return false;
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
		iLen = SeSend(socket, pkNetStreamNode->pkBuf + pkNetStreamNode->usReadPos, pkNetStreamNode->usWritePos - pkNetStreamNode->usReadPos, MSG_NOSIGNAL);
		if(iLen == 0)
		{
			SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[SeNetCoreSendBuf] socket is close by client.socket=%llx", pkNetSocket->kHSocket);
			SeNetSreamHeadAdd(&pkNetSocket->kSendNetStream, pkNetStreamNode);
			return false;
		}
		else if(iLen < 0)
		{
			iErrorno = SeErrno();
			SeNetSreamHeadAdd(&pkNetSocket->kSendNetStream, pkNetStreamNode);
			if(iErrorno == SE_EINTR) continue;
			else if(iErrorno == SE_EWOULDBLOCK) { break; }
			else { SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[SeNetCoreSendBuf] socket send error.socket=%llx", pkNetSocket->kHSocket); return false;}
		}
		else
		{
			pkNetStreamNode->usReadPos += iLen;
			if(pkNetStreamNode->usWritePos <= pkNetStreamNode->usReadPos) { assert(pkNetStreamNode->usWritePos == pkNetStreamNode->usReadPos); SeNetSreamHeadAdd(pkNetCore->kSocketMgr.pkNetStreamIdle, pkNetStreamNode); }
			else { SeLogWrite(&pkNetCore->kLog, LT_WARNING, true, "[SEND DATA] send buf leave some data,now send again.socket=%llx", pkNetSocket->kHSocket); SeNetSreamHeadAdd(&pkNetSocket->kSendNetStream, pkNetStreamNode); }
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

	if(SeGetNetSreamLen(&pkNetSocket->kSendNetStream) >= SENETCORE_SOCKET_RS_BUF_LEN)
	{
		SeLogWrite(&pkNetCore->kLog, LT_ERROR, true, "[SEND MORE BUF] SendBuf Too More And Close It.socket=%llx.size>%d", pkNetSocket->kHSocket, SENETCORE_SOCKET_RS_BUF_LEN);
		return false;
	}

	return true;
}

void SeNetCoreListenSocket(struct SENETCORE *pkNetCore, struct SESOCKET *pkNetSocketListen)
{
	int iErrorno;
	char *pcAddrIP;
	SOCKET kSocket;
	HSOCKET kHSocket;
	char acLocalIP[128];
	struct linger so_linger;
	struct sockaddr ksockaddr;
	struct SESOCKET *pkNetSocketAccept;
	
	while(true)
	{
		kSocket = SeAccept(SeGetSocketByHScoket(pkNetSocketListen->kHSocket), &ksockaddr);
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
		
		so_linger.l_onoff = 1;
		so_linger.l_linger = 0;
		if(SeSetSockOpt(kSocket, SOL_SOCKET, SO_LINGER, (const char *)&so_linger, sizeof(so_linger)) != 0)
		{
			iErrorno = SeErrno();
			SeCloseSocket(kSocket);
			SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP CLIENT] SeSetSockOpt ERROR, errno=%d", iErrorno);
			return;
		}

		if(SeSetNoDelay(kSocket) != 0)
		{
			iErrorno = SeErrno();
			SeCloseSocket(kSocket);
			SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP CLIENT] SeSetNoDelay ERROR, errno=%d", iErrorno);
			return;
		}

		if(SeSetReuseAddr(kSocket) != 0)
		{
			iErrorno = SeErrno();
			SeCloseSocket(kSocket);
			SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP CLIENT] SeSetReuseAddr ERROR, errno=%d", iErrorno);
			return;
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
		pkNetSocketAccept->usStatus = SOCKET_STATUS_CONNECTED;
		pkNetSocketAccept->iActiveTimeOut = pkNetSocketListen->iActiveTimeOut;
		pkNetSocketAccept->llTime = SeTimeGetTickCount();
		pkNetSocketAccept->kBelongListenHSocket = pkNetSocketListen->kHSocket;
		SeNetSocketMgrAddSendOrRecvInList(&pkNetCore->kSocketMgr, pkNetSocketAccept, true);
		memcpy(&pkNetSocketAccept->kRemoteAddr, &ksockaddr, sizeof(struct sockaddr));
		pcAddrIP = inet_ntoa(pkNetSocketListen->kRemoteAddr.sin_addr);
		SeStrNcpy(acLocalIP, (int)sizeof(acLocalIP), pcAddrIP ? pcAddrIP : "");
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP CLIENT] Accept client, ip=%s port=%d localsvrip=%s localsvrport=%d socket=%llx", \
			inet_ntoa(pkNetSocketAccept->kRemoteAddr.sin_addr), ntohs(pkNetSocketAccept->kRemoteAddr.sin_port), acLocalIP, ntohs(pkNetSocketListen->kRemoteAddr.sin_port), kHSocket);
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
		epoll_ctl(pkNetCore->kHandle, EPOLL_CTL_DEL, socket, &kEvent);

		if(bWrite == true)
		{
			iErrorno = -1;
			iLen = sizeof(int);
			SeGetSockOpt(socket, SOL_SOCKET, SO_ERROR, (char*)&iErrorno, (SOCK_LEN*)&iLen);
			if(iErrorno == 0)
			{
				pkNetSocket->usStatus = SOCKET_STATUS_CONNECTED;
				pkNetSocket->llTime = SeTimeGetTickCount();
				SeNetSocketMgrAddSendOrRecvInList(&pkNetCore->kSocketMgr, pkNetSocket, true);
				SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[SeNetCoreClientSocket] connect ok.socket=%llx", pkNetSocket->kHSocket);
				return;
			}
			bOK = false;
		}

		if(bError == true || bOK == false)
		{
			pkNetSocket->usStatus = SOCKET_STATUS_CONNECTED_FAILED;
			SeCloseSocket(socket);
			SeNetSocketMgrAddSendOrRecvInList(&pkNetCore->kSocketMgr, pkNetSocket, true);
			SeLogWrite(&pkNetCore->kLog, LT_ERROR, true, "[SeNetCoreClientSocket] connect failed.socket=%llx", pkNetSocket->kHSocket);
			return;
		}

		pkNetSocket->usStatus = SOCKET_STATUS_CONNECTED_FAILED;
		SeCloseSocket(socket);
		SeNetSocketMgrAddSendOrRecvInList(&pkNetCore->kSocketMgr, pkNetSocket, true);		
		SeLogWrite(&pkNetCore->kLog, LT_ERROR, true, "[SeNetCoreClientSocket] connect status error.socket=%llx", pkNetSocket->kHSocket);

		// no call here?
		assert(0 != 0);
		return;
	}

	if(bError == true)
	{
		SeNetCoreDisconnect(pkNetCore, pkNetSocket->kHSocket);
		SeLogWrite(&pkNetCore->kLog, LT_ERROR, true, "[SeNetCoreClientSocket] connect throw error.socket=%llx", pkNetSocket->kHSocket);
		return;
	}

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

	if(bError == true)
	{
		SeNetCoreDisconnect(pkNetCore, pkNetSocket->kHSocket);
		SeLogWrite(&pkNetCore->kLog, LT_ERROR, true, "[SeNetCoreAcceptSocket] accept error.socket=%llx", pkNetSocket->kHSocket);
		return;
	}

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
	struct epoll_event kEvent;
	struct SESOCKET *pkNetSocket;
	const struct SESOCKET *pkConstNetSocket;

	*rSSize = 0;
	*rRSize = 0;

	if (*riLen <= 0 || *riLen < SENETCORE_SOCKET_RECV_BUF_LEN)
	{
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[RECV Buf] Recv Buf too small.len=%d", *riLen);
		return false;
	}

	pkConstNetSocket = SeNetSocketMgrTimeOut(&pkNetCore->kSocketMgr);
	if(pkConstNetSocket)
	{
		if(pkConstNetSocket->usStatus == SOCKET_STATUS_CONNECTING || pkConstNetSocket->usStatus == SOCKET_STATUS_ACTIVECONNECT)
		{
			SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TIME OUT] Socket time out.socket=%llx", pkConstNetSocket->kHSocket);
			SeNetCoreDisconnect(pkNetCore, pkConstNetSocket->kHSocket);
		}
	}

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
			kEvent.data.u64 = pkNetSocket->kHSocket;
			kEvent.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLERR | EPOLLHUP | EPOLLET;
			epoll_ctl(pkNetCore->kHandle, EPOLL_CTL_ADD, SeGetSocketByHScoket(pkNetSocket->kHSocket), &kEvent);
			*riLen = 0;
			pcAddrIP = inet_ntoa(pkNetSocket->kRemoteAddr.sin_addr);
			if(pcAddrIP) { strcpy(pcBuf, pcAddrIP); *riLen = (int)strlen(pcAddrIP); pcBuf[*riLen] = '\0'; }
			*riEventSocket = SENETCORE_EVENT_SOCKET_CONNECT;
			pkNetSocket->usStatus = SOCKET_STATUS_ACTIVECONNECT;
			pkNetSocket->llTime = SeTimeGetTickCount();
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
			pkNetSocket->usStatus = SOCKET_STATUS_COMM_IDLE;
			*riEventSocket = SENETCORE_EVENT_SOCKET_DISCONNECT;
			SeNetSocketMgrAddSendOrRecvInList(&pkNetCore->kSocketMgr, pkNetSocket, true);
			return true;
		}

		if(pkNetSocket->usStatus == SOCKET_STATUS_COMM_IDLE)
		{
			SeNetSocketMgrDel(&pkNetCore->kSocketMgr, pkNetSocket->kHSocket);
			continue;
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
		if(!SeNetSreamCanRead(&pkNetSocket->kRecvNetStream, pkNetSocket->pkGetHeaderLenFun, pkNetSocket->iHeaderLen)) { continue; }
		bOK = SeNetSreamRead(&pkNetSocket->kRecvNetStream, pkNetCore->kSocketMgr.pkNetStreamIdle, pkNetSocket->pkGetHeaderLenFun, pkNetSocket->iHeaderLen, pcBuf, riLen);
		if(!bOK) { SeNetCoreDisconnect(pkNetCore, pkNetSocket->kHSocket); SeLogWrite(&pkNetCore->kLog, LT_ERROR, true, "[READ DATA] Read Data Error. socket=%llx", pkConstNetSocket->kHSocket); continue; }
		
		pcBuf[*riLen] = '\0';
		*rkHSocket = pkNetSocket->kHSocket;
		*riEventSocket = SENETCORE_EVENT_SOCKET_RECV_DATA;
		*rkListenHSocket = pkNetSocket->kBelongListenHSocket;
		*rSSize = SeNetSreamCount(&pkNetSocket->kSendNetStream);
		*rRSize = SeNetSreamCount(&pkNetSocket->kRecvNetStream);
		if(SeNetSreamCount(&pkNetSocket->kRecvNetStream) > 0) { SeNetSocketMgrAddSendOrRecvInList(&pkNetCore->kSocketMgr, pkNetSocket, false); }
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
	iNum = epoll_wait(pkNetCore->kHandle, pkNetCore->akEvents, sizeof(pkNetCore->akEvents)/sizeof(struct epoll_event), pkNetCore->iWaitTime);
	bWork = iNum > 0 ? true : false;

	for(i = 0; i < iNum; i++)
	{
		pkEvent = &pkNetCore->akEvents[i];
		bRead = pkEvent->events & EPOLLIN;
		bWrite =pkEvent->events & EPOLLOUT;
		kHSocket = (HSOCKET)pkEvent->data.u64;
		bError = (pkEvent->events & EPOLLRDHUP) || (pkEvent->events &  EPOLLERR) || (pkEvent->events &  EPOLLHUP);
		pkNetSocket = SeNetSocketMgrGet(&pkNetCore->kSocketMgr, kHSocket);
		if(!pkNetSocket) { SeLogWrite(&pkNetCore->kLog, LT_CRITICAL, true, "[EPOLL WAIT] socket not found.socket=%llx", kHSocket); continue; }
		if(pkNetSocket->iTypeSocket == LISTEN_TCP_TYPE_SOCKET) { SeNetCoreListenSocket(pkNetCore, pkNetSocket); }
		else if(pkNetSocket->iTypeSocket == CLIENT_TCP_TYPE_SOCKET) { SeNetCoreClientSocket(pkNetCore, pkNetSocket, bRead, bWrite, bError); }
		else if(pkNetSocket->iTypeSocket == ACCEPT_TCP_TYPE_SOCKET) { SeNetCoreAcceptSocket(pkNetCore, pkNetSocket, bRead, bWrite, bError); }
		else { SeLogWrite(&pkNetCore->kLog, LT_CRITICAL, true, "[EPOLL WAIT] SeNetCoreRead Error. typesocket=%d status=%d socket=%llx.", pkNetSocket->iTypeSocket, pkNetSocket->usStatus, pkNetSocket->kHSocket); }
	}

	if(SeNetCoreProcess(pkNetCore, riEvent, rkListenHSocket, rkHSocket, pcBuf, riLen, rSSize, rRSize)) { return true; }
	
	*riEvent = SENETCORE_EVENT_SOCKET_IDLE;
	return !bWork;
}

struct SESOCKET *SeNetCoreGetSocket(struct SENETCORE *pkNetCore, HSOCKET kHSocket)
{
	return SeNetSocketMgrGet(&pkNetCore->kSocketMgr, kHSocket);
}

#endif
