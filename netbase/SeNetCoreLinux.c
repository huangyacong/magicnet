#include "SeNetCore.h"

#if defined(__linux)

bool SeNetCoreSendBuf(struct SENETCORE *pkNetCore, struct SESOCKET *pkNetSocket);
bool SeNetCoreRecvBuf(struct SENETCORE *pkNetCore, struct SESOCKET *pkNetSocket);

HSOCKET SeGetTimerID()
{
	return SeGetValidHSocket();
}

bool SeTimerRoutine(struct SENETCORE *pkNetCore)
{
	uint64_t exp = 0;
	return read(pkNetCore->kTimerHandle, &exp, sizeof(exp)) == sizeof(exp);
}

void SeCreateTimer(struct SENETCORE *pkNetCore, int iMillSec)
{
	struct epoll_event ev;
	static struct itimerspec timeValue;

	pkNetCore->kTimerHandle = timerfd_create(CLOCK_MONOTONIC, EFD_NONBLOCK | EFD_CLOEXEC);

	timeValue.it_value.tv_sec = 1;
	timeValue.it_value.tv_nsec = 0;

	timeValue.it_interval.tv_sec = iMillSec / 1000;
	timeValue.it_interval.tv_nsec = abs(timeValue.it_interval.tv_sec * 1000 - iMillSec) * 1000 * 1000;

	timerfd_settime(pkNetCore->kTimerHandle, 0, &timeValue, NULL);

	ev.events = EPOLLIN;
	ev.data.u64 = SeGetTimerID();
	epoll_ctl(pkNetCore->kHandle, EPOLL_CTL_ADD, pkNetCore->kTimerHandle, &ev);
}

void SeDeleteTimer(struct SENETCORE *pkNetCore)
{
	SeCloseHandle(pkNetCore->kTimerHandle);
}

void SeNetCoreInit(struct SENETCORE *pkNetCore, const char *pcLogName, unsigned short usMax, int iTimerCnt, int iLogLV)
{
	SeNetBaseInit();
	pkNetCore->iTag = 0;
	pkNetCore->iWaitTime = NET_CORE_WAIT_TIME;
	pkNetCore->iFlag = 0;
	pkNetCore->kHandle = epoll_create(usMax);
	pkNetCore->kTimerHandle = SE_INVALID_HANDLE;
	SeNetSocketMgrInit(&pkNetCore->kSocketMgr, usMax);
	SeInitLog(&pkNetCore->kLog, pcLogName);
	SeAddLogLV(&pkNetCore->kLog, iLogLV);
	SeCreateTimer(pkNetCore, iTimerCnt);
}

void SeNetCoreFin(struct SENETCORE *pkNetCore)
{
	SeDeleteTimer(pkNetCore);
	pkNetCore->kTimerHandle = SE_INVALID_HANDLE;
	SeNetSocketMgrFin(&pkNetCore->kSocketMgr);
	SeCloseHandle(pkNetCore->kHandle);
	SeFinLog(&pkNetCore->kLog);
	SeNetBaseEnd();
}

void SeNetCoreSetLogContextFunc(struct SENETCORE *pkNetCore, SELOGCONTEXT pkLogContextFunc, void *pkLogContect)
{
	SeLogSetLogContextFunc(&pkNetCore->kLog, pkLogContextFunc, pkLogContect);
}

void SeNetCoreSetWaitTime(struct SENETCORE *pkNetCore, int iWaitTime)
{
	pkNetCore->iWaitTime = iWaitTime;
}

HSOCKET SeNetCoreTCPListen(struct SENETCORE *pkNetCore, int iDomain, bool bReusePort, const char *pcIP, unsigned short usPort,\
	int iHeaderLen, bool bNoDelay, int iTimeOut, SEGETHEADERLENFUN pkGetHeaderLenFun, SESETHEADERLENFUN pkSetHeaderLenFun)
{
	int domain;
	int backlog;
	void* pkAddr;
	int iErrorno;
	SOCKET socket;
	SOCK_LEN iLen;
	HSOCKET kHSocket;
	struct sockaddr kAddr;
	struct linger so_linger;
	struct epoll_event kEvent;
	struct sockaddr_un servaddr;
	struct SESOCKET *pkNetSocket;
	
	pkAddr = 0;
	domain = iDomain;
	socket = SeSocket(domain, SOCK_STREAM);
	if(socket == SE_INVALID_SOCKET)
	{
		iErrorno = SeErrno();
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP LISTEN] Create Socket ERROR, errno=%d IP=%s port=%d", iErrorno, pcIP, usPort);
		return 0;
	}
	if(SeSetNoBlock(socket, true) != 0)
	{
		iErrorno = SeErrno();
		SeShutDown(socket);
		SeCloseSocket(socket);
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP LISTEN] SeSetNoBlock ERROR, errno=%d IP=%s port=%d", iErrorno, pcIP, usPort);
		return 0;
	}
	so_linger.l_onoff = 1;
	so_linger.l_linger = 0;
	if(SeSetSockOpt(socket, SOL_SOCKET, SO_LINGER, (const char *)&so_linger, sizeof(so_linger)) != 0)
	{
		iErrorno = SeErrno();
		SeShutDown(socket);
		SeCloseSocket(socket);
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP LISTEN] SeSetSockOpt ERROR, errno=%d IP=%s port=%d", iErrorno, pcIP, usPort);
		return 0;
	}
	if(SeSetReuseAddr(socket) != 0)
	{
		iErrorno = SeErrno();
		SeShutDown(socket);
		SeCloseSocket(socket);
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP LISTEN] SeSetReuseAddr ERROR, errno=%d IP=%s port=%d", iErrorno, pcIP, usPort);
		return 0;
	}
	if(bReusePort)
	{
		if(SeSetReusePort(socket) != 0)
		{
			iErrorno = SeErrno();
			SeShutDown(socket);
			SeCloseSocket(socket);
			SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP LISTEN] SeSetReusePort ERROR, errno=%d IP=%s port=%d", iErrorno, pcIP, usPort);
			return 0;
		}
	}

	if(domain == SE_DOMAIN_UNIX)
	{
		pkAddr = &servaddr;
		iLen = sizeof(struct sockaddr_un);
		SeSetSockAddr(domain, &servaddr, pcIP, usPort);
	}
	else
	{
		pkAddr = &kAddr;
		iLen = sizeof(struct sockaddr);
		SeSetSockAddr(domain, &kAddr, pcIP, usPort);
	}
	
	if(SeBind(socket, (struct sockaddr *)pkAddr, iLen) != 0)
	{
		iErrorno = SeErrno();
		SeShutDown(socket);
		SeCloseSocket(socket);
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP LISTEN] SeBind ERROR, errno=%d IP=%s port=%d", iErrorno, pcIP, usPort);
		return 0;
	}

	backlog = SENETCORE_SOCKET_BACKLOG;
	if(SeListen(socket, backlog) != 0)
	{
		iErrorno = SeErrno();
		SeShutDown(socket);
		SeCloseSocket(socket);
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP LISTEN] SeListen ERROR, errno=%d backlog=%d IP=%s port=%d", iErrorno, backlog, pcIP, usPort);
		return 0;
	}

	kHSocket = SeNetSocketMgrAdd(&pkNetCore->kSocketMgr, socket, LISTEN_TCP_TYPE_SOCKET, iHeaderLen, pkGetHeaderLenFun, pkSetHeaderLenFun);
	if(kHSocket <= 0)
	{
		SeShutDown(socket);
		SeCloseSocket(socket);
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP LISTEN] SocketMgr is full, IP=%s port=%d", pcIP, usPort);
		return 0;
	}

	kEvent.data.u64 = kHSocket;
	kEvent.events = EPOLLIN;
	if(epoll_ctl(pkNetCore->kHandle, EPOLL_CTL_ADD, socket, &kEvent) != 0)
	{
		iErrorno = SeErrno();
		SeShutDown(socket);
		SeCloseSocket(socket);
		SeNetSocketMgrDel(&pkNetCore->kSocketMgr, kHSocket);
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP LISTEN] epoll_ctl ERROR, errno=%d IP=%s port=%d", iErrorno, pcIP, usPort);
		return 0;
	}

	SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP LISTEN] SocketMgr listen ok, IP=%s port=%d", pcIP, usPort);
	pkNetSocket = SeNetSocketMgrGet(&pkNetCore->kSocketMgr, kHSocket);
	pkNetSocket->iDomain = domain;
	pkNetSocket->iNoDelay = bNoDelay ? 1 : 0;
	pkNetSocket->usStatus = SOCKET_STATUS_ACTIVECONNECT;
	pkNetSocket->iActiveTimeOut = iTimeOut;
	SeSetAddrToBuf(domain, pkAddr, pkNetSocket->acIPAddr, (int)sizeof(pkNetSocket->acIPAddr), &pkNetSocket->iIPPort);

	return kHSocket;
}

HSOCKET SeNetCoreTCPClient(struct SENETCORE *pkNetCore, int iDomain, const char *pcIP, unsigned short usPort,\
	int iHeaderLen, bool bNoDelay, int iTimeOut, int iConnectTimeOut, SEGETHEADERLENFUN pkGetHeaderLenFun, SESETHEADERLENFUN pkSetHeaderLenFun)
{
	int domain;
	int iResult;
	void* pkAddr;
	int iErrorno;
	SOCKET socket;
	SOCK_LEN kLen;
	HSOCKET kHSocket;
	struct sockaddr kAddr;
	struct linger so_linger;
	struct epoll_event kEvent;
	struct sockaddr_un servaddr;
	struct SESOCKET *pkNetSocket;
	
	pkAddr = 0;
	domain = iDomain;
	socket = SeSocket(domain, SOCK_STREAM);
	if(socket == SE_INVALID_SOCKET)
	{
		iErrorno = SeErrno();
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP CLIENT] Create Socket ERROR, errno=%d IP=%s port=%d", iErrorno, pcIP, usPort);
		return 0;
	}
	if(SeSetNoBlock(socket, true) != 0)
	{
		iErrorno = SeErrno();
		SeShutDown(socket);
		SeCloseSocket(socket);
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP CLIENT] SeSetNoBlock ERROR, errno=%d IP=%s port=%d", iErrorno, pcIP, usPort);
		return 0;
	}

	so_linger.l_onoff = 1;
	so_linger.l_linger = 0;
	if(SeSetSockOpt(socket, SOL_SOCKET, SO_LINGER, (const char *)&so_linger, sizeof(so_linger)) != 0)
	{
		iErrorno = SeErrno();
		SeShutDown(socket);
		SeCloseSocket(socket);
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP CLIENT] SeSetSockOpt ERROR, errno=%d IP=%s port=%d", iErrorno, pcIP, usPort);
		return 0;
	}

	if(bNoDelay)
	{
		if(SeSetNoDelay(socket) != 0)
		{
			iErrorno = SeErrno();
			SeShutDown(socket);
			SeCloseSocket(socket);
			SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP CLIENT] SeSetNoDelay ERROR, errno=%d IP=%s port=%d", iErrorno, pcIP, usPort);
			return 0;
		}
	}

	if(SeSetReuseAddr(socket) != 0)
	{
		iErrorno = SeErrno();
		SeShutDown(socket);
		SeCloseSocket(socket);
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP CLIENT] SeSetReuseAddr ERROR, errno=%d IP=%s port=%d", iErrorno, pcIP, usPort);
		return 0;
	}

	kHSocket = SeNetSocketMgrAdd(&pkNetCore->kSocketMgr, socket, CLIENT_TCP_TYPE_SOCKET, iHeaderLen, pkGetHeaderLenFun, pkSetHeaderLenFun);
	if(kHSocket <= 0)
	{
		SeShutDown(socket);
		SeCloseSocket(socket);
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP CLIENT] SocketMgr is full, IP=%s port=%d", pcIP, usPort);
		return 0;
	}
	pkNetSocket = SeNetSocketMgrGet(&pkNetCore->kSocketMgr, kHSocket);
	pkNetSocket->iDomain = domain;
	
	if(domain == SE_DOMAIN_UNIX)
	{
		pkAddr = &servaddr;
		kLen = sizeof(struct sockaddr_un);
		SeSetSockAddr(domain, &servaddr, pcIP, usPort);
	}
	else
	{
		pkAddr = &kAddr;
		kLen = sizeof(struct sockaddr);
		SeSetSockAddr(domain, &kAddr, pcIP, usPort);
	}

	iResult = SeConnect(socket, (struct sockaddr *)pkAddr, kLen);
	iErrorno = SeErrno();
	if(iResult != 0 && iErrorno != SE_EINPROGRESS && iErrorno != SE_EINTR)
	{
		pkNetSocket = 0;
		SeShutDown(socket);
		SeCloseSocket(socket);
		SeNetSocketMgrDel(&pkNetCore->kSocketMgr, kHSocket);
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP CLIENT] SeConnect ERROR, errno=%d IP=%s port=%d socket=0x%llx", iErrorno, pcIP, usPort, kHSocket);
		return 0;
	}

	if(iResult == 0)
	{
		pkNetSocket->usStatus = SOCKET_STATUS_CONNECTED;
		pkNetSocket->llTime = SeTimeGetTickCount();
		pkNetSocket->iActiveTimeOut = iTimeOut;
		pkNetSocket->iConnectTimeOut = iConnectTimeOut;
		SeSetAddrToBuf(domain, pkAddr, pkNetSocket->acIPAddr, (int)sizeof(pkNetSocket->acIPAddr), &pkNetSocket->iIPPort);
		SeNetSocketMgrAddSendOrRecvInList(&pkNetCore->kSocketMgr, pkNetSocket, true);
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP CLIENT] ConnectEx to svr, ip=%s port=%d socket=0x%llx", pkNetSocket->acIPAddr, pkNetSocket->iIPPort, kHSocket);
		return kHSocket; 
	}

	kEvent.data.u64 = kHSocket;
	kEvent.events = EPOLLOUT | EPOLLRDHUP | EPOLLERR | EPOLLHUP | EPOLLONESHOT;
	if(epoll_ctl(pkNetCore->kHandle, EPOLL_CTL_ADD, socket, &kEvent) != 0)
	{
		pkNetSocket = 0;
		iErrorno = SeErrno();
		SeShutDown(socket);
		SeCloseSocket(socket);
		SeNetSocketMgrDel(&pkNetCore->kSocketMgr, kHSocket);
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP CLIENT] epoll_ctl ERROR, errno=%d IP=%s port=%d socket=0x%llx", iErrorno, pcIP, usPort, kHSocket);
		return 0;
	}
	
	pkNetSocket->usStatus = SOCKET_STATUS_CONNECTING;
	pkNetSocket->llTime = SeTimeGetTickCount();
	pkNetSocket->iActiveTimeOut = iTimeOut;
	pkNetSocket->iConnectTimeOut = iConnectTimeOut;
	SeSetAddrToBuf(domain, pkAddr, pkNetSocket->acIPAddr, (int)sizeof(pkNetSocket->acIPAddr), &pkNetSocket->iIPPort);
	SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP CLIENT] ConnectEx to svr, ip=%s port=%d socket=0x%llx", pkNetSocket->acIPAddr, pkNetSocket->iIPPort, kHSocket);
	return kHSocket;
}

bool SeNetCoreSend(struct SENETCORE *pkNetCore, HSOCKET kHSocket, const char* pcBuf, int iSize)
{
	bool bRet;
	struct SESOCKET *pkNetSocket;
	
	if (iSize < 0 || !pcBuf) 
	{
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[CORE SEND] Send Data Error. Socket=0x%llx.Size=%d pkBuf is %s", kHSocket, iSize, pcBuf ? "true" : "false");
		return false;
	}

	pkNetSocket = SeNetSocketMgrGet(&pkNetCore->kSocketMgr, kHSocket);
	if (!pkNetSocket)
	{
		return false;
	}

	if (pkNetSocket->iTypeSocket != CLIENT_TCP_TYPE_SOCKET && pkNetSocket->iTypeSocket != ACCEPT_TCP_TYPE_SOCKET) 
	{
		return false;
	}
	if (pkNetSocket->usStatus != SOCKET_STATUS_ACTIVECONNECT)
	{
		return false;
	}

	if (SeNetSocketMgrHasEvent(pkNetSocket, WRITE_EVENT_SOCKET))
	{
		SeNetSocketMgrAddSendOrRecvInList(&pkNetCore->kSocketMgr, pkNetSocket, true);
	}

	if (!SeNetSocketMgrUpdateNetStreamIdle(&pkNetCore->kSocketMgr, pkNetSocket->iHeaderLen, iSize))
	{
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[CORE SEND] no more memcahce.socket=0x%llx", kHSocket);
		return false;
	}
	if (!SeNetSreamCanWrite(&pkNetSocket->kSendNetStream, pkNetSocket->pkSetHeaderLenFun, pkNetSocket->iHeaderLen, iSize))
	{
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[CORE SEND] send data ERROR.socket=0x%llx.size=%d", kHSocket, iSize);
		return false;
	}
	bRet = SeNetSreamWrite(&pkNetSocket->kSendNetStream, &(pkNetCore->kSocketMgr.kNetStreamIdle), pkNetSocket->pkSetHeaderLenFun, pkNetSocket->iHeaderLen, pcBuf, iSize);
	if (!bRet)
	{
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[CORE SEND] Send Data Error, Now Close The Socket=0x%llx.Size=%d", kHSocket, iSize);
		SeNetCoreDisconnect(pkNetCore, pkNetSocket->kHSocket);
		return false;
	}
	
	return bRet;
}

bool SeNetCoreSendExtend(struct SENETCORE *pkNetCore, HSOCKET kHSocket, const struct SENETSTREAMBUF *pkBufList, int iNum)
{
	int i;
	bool bRet;
	int iSize;
	struct SESOCKET *pkNetSocket;

	if (!pkBufList || iNum <= 0)
	{
		return false;
	}

	iSize = 0;
	for (i = 0; i < iNum; i++)
	{
		if (!pkBufList[i].pcBuf)
		{
			SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[CORE SEND] Send Data Error, Buf Is NULL. Socket=0x%llx.", kHSocket);
			return false;
		}
		if (pkBufList[i].iBufLen < 0)
		{
			SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[CORE SEND] Send Data Error, Len Is Error. Socket=0x%llx.", kHSocket);
			return false;
		}
		iSize += pkBufList[i].iBufLen;
	}
	
	if (iSize < 0)
	{
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[CORE SEND] Send Data Error. Socket=0x%llx.Size=%d", kHSocket, iSize);
		return false;
	}

	pkNetSocket = SeNetSocketMgrGet(&pkNetCore->kSocketMgr, kHSocket);
	if (!pkNetSocket)
	{
		return false;
	}

	if (pkNetSocket->iTypeSocket != CLIENT_TCP_TYPE_SOCKET && pkNetSocket->iTypeSocket != ACCEPT_TCP_TYPE_SOCKET)
	{
		return false;
	}
	if (pkNetSocket->usStatus != SOCKET_STATUS_ACTIVECONNECT)
	{
		return false;
	}

	if (SeNetSocketMgrHasEvent(pkNetSocket, WRITE_EVENT_SOCKET))
	{
		SeNetSocketMgrAddSendOrRecvInList(&pkNetCore->kSocketMgr, pkNetSocket, true);
	}
	
	if (!SeNetSocketMgrUpdateNetStreamIdle(&pkNetCore->kSocketMgr, pkNetSocket->iHeaderLen, iSize))
	{
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[CORE SEND] no more memcahce.socket=0x%llx", kHSocket);
		return false;
	}
	if (!SeNetSreamCanWrite(&pkNetSocket->kSendNetStream, pkNetSocket->pkSetHeaderLenFun, pkNetSocket->iHeaderLen, iSize))
	{
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[CORE SEND] send data ERROR.socket=0x%llx.size=%d", kHSocket, iSize);
		return false;
	}
	bRet = SeNetSreamWriteExtend(&pkNetSocket->kSendNetStream, &(pkNetCore->kSocketMgr.kNetStreamIdle), pkNetSocket->pkSetHeaderLenFun, pkNetSocket->iHeaderLen, pkBufList, iNum);
	if (!bRet)
	{
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[CORE SEND] Send Data Error, Now Close The Socket=0x%llx.Size=%d", kHSocket, iSize);
		SeNetCoreDisconnect(pkNetCore, pkNetSocket->kHSocket);
		return false;
	}
	
	return bRet;
}

void SeNetCoreDisconnect(struct SENETCORE *pkNetCore, HSOCKET kHSocket)
{
	bool bOK;
	SOCKET socket;
	struct epoll_event kEvent;
	unsigned short usOrgStatus;
	struct SESOCKET *pkNetSocket;

	pkNetSocket = SeNetSocketMgrGet(&pkNetCore->kSocketMgr, kHSocket);
	if(!pkNetSocket)
	{
		return;
	}
	if(pkNetSocket->iTypeSocket != CLIENT_TCP_TYPE_SOCKET && pkNetSocket->iTypeSocket != ACCEPT_TCP_TYPE_SOCKET)
	{
		return;
	}

	bOK = false;
	usOrgStatus = pkNetSocket->usStatus;

	switch (pkNetSocket->usStatus)
	{
		case SOCKET_STATUS_CONNECTING:
		{
			pkNetSocket->usStatus = SOCKET_STATUS_CONNECTED_FAILED;
			break;
		}
		case SOCKET_STATUS_ACTIVECONNECT:
		{
			pkNetSocket->usStatus = SOCKET_STATUS_DISCONNECT;
			break;
		}
		default:
		{
			bOK = true;
			SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "SeNetCoreDisconnect Failed. OrgStatus=%d Atatus=%d socket=0x%llx", usOrgStatus, pkNetSocket->usStatus, kHSocket);
			break;
		}
	}

	if (bOK)
	{
		return;
	}

	SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "SeNetCoreDisconnect. OrgStatus=%d Atatus=%d SendCount=%d RecvCount=%d socket=0x%llx port=%d", \
		usOrgStatus, pkNetSocket->usStatus, SeNetSreamCount(&pkNetSocket->kSendNetStream), SeNetSreamCount(&pkNetSocket->kRecvNetStream), kHSocket, pkNetSocket->iIPPort);

	socket = pkNetSocket->kSocketFd.kSocket;
	epoll_ctl(pkNetCore->kHandle, EPOLL_CTL_DEL, socket, &kEvent);
	SeShutDown(socket);
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
	
	bRecv = false;
	while(true)
	{
		if(!SeNetSocketMgrUpdateNetStreamIdle(&pkNetCore->kSocketMgr, pkNetSocket->iHeaderLen, 0))
		{
			SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[CORE RECV] no more memcahce.socket=0x%llx", pkNetSocket->kHSocket);
			return false;
		}
		
		bPop = false;
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
			pkNetStreamNode = SeNetSreamHeadPop(&(pkNetCore->kSocketMgr.kNetStreamIdle));
			if(pkNetStreamNode)
			{
				SeNetSreamNodeZero(pkNetStreamNode);
			}
		}

		if(!pkNetStreamNode)
		{
			SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[CORE RECV] no more ilde memcahce.socket=0x%llx", pkNetSocket->kHSocket);
			return false;
		}

		if(pkNetStreamNode->usMaxLen <= pkNetStreamNode->usWritePos)
		{
			if(bPop)
			{
				SeNetSreamTailAdd(&pkNetSocket->kRecvNetStream, pkNetStreamNode);
			}
			else
			{
				SeNetSreamHeadAdd(&(pkNetCore->kSocketMgr.kNetStreamIdle), pkNetStreamNode);
			}
			SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[CORE RECV] usMaxLen <= usWritePos.socket=0x%llx", pkNetSocket->kHSocket);
			return false;
		}
		
		iLen = SeRecv(pkNetSocket->kSocketFd.kSocket, pkNetStreamNode->pkBuf + pkNetStreamNode->usWritePos, pkNetStreamNode->usMaxLen - pkNetStreamNode->usWritePos, 0);

		if(iLen == 0)
		{
			if(bPop)
			{
				SeNetSreamTailAdd(&pkNetSocket->kRecvNetStream, pkNetStreamNode);
			}
			else
			{
				SeNetSreamHeadAdd(&(pkNetCore->kSocketMgr.kNetStreamIdle), pkNetStreamNode);
			}
			SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[SeNetCoreRecvBuf] socket is close by client.socket=0x%llx", pkNetSocket->kHSocket);
			return false;
		}
		else if(iLen < 0)
		{
			iErrorno = SeErrno();
			if(bPop)
			{
				SeNetSreamTailAdd(&pkNetSocket->kRecvNetStream, pkNetStreamNode);
			}
			else
			{
				SeNetSreamHeadAdd(&(pkNetCore->kSocketMgr.kNetStreamIdle), pkNetStreamNode);
			}
			if(iErrorno == SE_EINTR)
			{
				bRecv = true;
				SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[SENETCORERECVBUF_EINTR] socket SE_EINTR.socket=0x%llx", pkNetSocket->kHSocket);
				continue;
			}
			else if(iErrorno == SE_EWOULDBLOCK)
			{
				break;
			}
			else 
			{
				SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[SeNetCoreRecvBuf] socket recv error.socket=0x%llx", pkNetSocket->kHSocket);
				return false;
			}
		}
		else
		{
			bRecv = true;
			pkNetStreamNode->usWritePos += iLen;
			SeNetSreamTailAdd(&pkNetSocket->kRecvNetStream, pkNetStreamNode);
		}
	}

	if (bRecv)
	{
		pkNetSocket->llTime = SeTimeGetTickCount();
	}

	return true;
}

bool SeNetCoreSendBuf(struct SENETCORE *pkNetCore, struct SESOCKET *pkNetSocket)
{
	int iLen;
	int iErrorno;
	bool bBlock;
	struct epoll_event kEvent;
	struct SENETSTREAMNODE *pkNetStreamNode;

	bBlock = false;
	if (!SeNetSocketMgrHasEvent(pkNetSocket, WRITE_EVENT_SOCKET))
	{
		return true;
	}

	while(true)
	{
		pkNetStreamNode = SeNetSreamHeadPop(&pkNetSocket->kSendNetStream);
		if(!pkNetStreamNode)
		{
			break;
		}

		iLen = SeSend(pkNetSocket->kSocketFd.kSocket, pkNetStreamNode->pkBuf + pkNetStreamNode->usReadPos, pkNetStreamNode->usWritePos - pkNetStreamNode->usReadPos, MSG_NOSIGNAL);
		if(iLen == 0)
		{
			SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[SeNetCoreSendBuf] socket is close by client.socket=0x%llx", pkNetSocket->kHSocket);
			SeNetSreamHeadAdd(&pkNetSocket->kSendNetStream, pkNetStreamNode);
			return false;
		}
		else if(iLen < 0)
		{
			iErrorno = SeErrno();
			SeNetSreamHeadAdd(&pkNetSocket->kSendNetStream, pkNetStreamNode);
			if(iErrorno == SE_EINTR)
			{
				SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[SENETCORESENDBUF_EINITR] socket SE_EINTR.socket=0x%llx", pkNetSocket->kHSocket);
				continue;
			}
			else if(iErrorno == SE_EWOULDBLOCK) 
			{
				bBlock = true;
				SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[SeNetCoreSendBuf] socket send block.socket=0x%llx", pkNetSocket->kHSocket);
				break;
			}
			else 
			{
				SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[SeNetCoreSendBuf] socket send error.socket=0x%llx", pkNetSocket->kHSocket);
				return false;
			}
		}
		else
		{
			pkNetStreamNode->usReadPos += iLen;
			if(pkNetStreamNode->usWritePos <= pkNetStreamNode->usReadPos)
			{
				assert(pkNetStreamNode->usWritePos == pkNetStreamNode->usReadPos);
				SeNetSreamTailAdd(&(pkNetCore->kSocketMgr.kNetStreamIdle), pkNetStreamNode);
			}
			else
			{
				SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[SEND DATA] send buf leave some data,now send again.iLen=%d socket=0x%llx", iLen, pkNetSocket->kHSocket);
				SeNetSreamHeadAdd(&pkNetSocket->kSendNetStream, pkNetStreamNode);
			}
		}
	}

	if(bBlock)
	{
		SeNetSocketMgrClearEvent(pkNetSocket, WRITE_EVENT_SOCKET);
		if(!SeNetSocketMgrHasEvent(pkNetSocket, READ_EVENT_SOCKET))
		{
			SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[SeNetCoreSendBuf] socket open EPOLLOUT epoll_ctl.socket=0x%llx", pkNetSocket->kHSocket);
			kEvent.data.u64 = pkNetSocket->kHSocket;
			kEvent.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLERR | EPOLLHUP | EPOLLET;
			if(epoll_ctl(pkNetCore->kHandle, EPOLL_CTL_MOD, pkNetSocket->kSocketFd.kSocket, &kEvent) != 0)
			{
				SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "SeNetCoreSendBuf epoll_ctl failed.socket=0x%llx", pkNetSocket->kHSocket);
				return false;
			}
			SeNetSocketMgrAddEvent(pkNetSocket, READ_EVENT_SOCKET);
		}
		return true;
	}
	else
	{
		if(SeNetSreamCount(&pkNetSocket->kSendNetStream) <= 0)
		{
			if(SeNetSocketMgrHasEvent(pkNetSocket, READ_EVENT_SOCKET))
			{
				SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[SeNetCoreSendBuf] socket close EPOLLOUT epoll_ctl.socket=0x%llx", pkNetSocket->kHSocket);
				kEvent.data.u64 = pkNetSocket->kHSocket;
				kEvent.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP | EPOLLET;
				if(epoll_ctl(pkNetCore->kHandle, EPOLL_CTL_MOD, pkNetSocket->kSocketFd.kSocket, &kEvent) != 0)
				{
					SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "SeNetCoreSendBuf epoll_ctl failed.socket=0x%llx", pkNetSocket->kHSocket);
					return false;
				}
				SeNetSocketMgrClearEvent(pkNetSocket, READ_EVENT_SOCKET);
			}
		}
		else
		{
			assert(0 != 0); // nocall
			SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[SeNetCoreSendBuf] socket send noblock but buf > 0.socket=0x%llx", pkNetSocket->kHSocket);
		}
		return true;
	}

	return true;
}

void SeNetCoreListenSocket(struct SENETCORE *pkNetCore, struct SESOCKET *pkNetSocketListen)
{
	int iErrorno;
	void* pkAddr;
	SOCK_LEN iLen;
	SOCKET kSocket;
	HSOCKET kHSocket;
	struct linger so_linger;
	struct sockaddr ksockaddr;
	struct sockaddr_un servaddr;
	struct SESOCKET *pkNetSocket;
	
	while(true)
	{
		pkAddr = 0;

		if(pkNetSocketListen->iDomain == SE_DOMAIN_UNIX)
		{
			pkAddr = &servaddr;
			iLen = sizeof(struct sockaddr_un);
			SeSetSockAddr(pkNetSocketListen->iDomain, &servaddr, pkNetSocketListen->acIPAddr, 0);
		}
		else
		{
			pkAddr = &ksockaddr;
			iLen = sizeof(struct sockaddr);
		}

		kSocket = SeAccept(pkNetSocketListen->kSocketFd.kSocket, (struct sockaddr *)pkAddr, &iLen);
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
			SeShutDown(kSocket);
			SeCloseSocket(kSocket);
			SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[ACCEPT SOCKET] SeSetNoBlock ERROR, errno=%d", iErrorno);
			return;
		}
		
		so_linger.l_onoff = 1;
		so_linger.l_linger = 0;
		if(SeSetSockOpt(kSocket, SOL_SOCKET, SO_LINGER, (const char *)&so_linger, sizeof(so_linger)) != 0)
		{
			iErrorno = SeErrno();
			SeShutDown(kSocket);
			SeCloseSocket(kSocket);
			SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP CLIENT] SeSetSockOpt ERROR, errno=%d", iErrorno);
			return;
		}

		if(pkNetSocketListen->iNoDelay != 0)
		{
			if(SeSetNoDelay(kSocket) != 0)
			{
				iErrorno = SeErrno();
				SeShutDown(kSocket);
				SeCloseSocket(kSocket);
				SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP CLIENT] SeSetNoDelay ERROR, errno=%d", iErrorno);
				return;
			}
		}

		if(SeSetReuseAddr(kSocket) != 0)
		{
			iErrorno = SeErrno();
			SeShutDown(kSocket);
			SeCloseSocket(kSocket);
			SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP CLIENT] SeSetReuseAddr ERROR, errno=%d", iErrorno);
			return;
		}

		kHSocket = SeNetSocketMgrAdd(&pkNetCore->kSocketMgr, kSocket, ACCEPT_TCP_TYPE_SOCKET, \
			pkNetSocketListen->iHeaderLen, pkNetSocketListen->pkGetHeaderLenFun, pkNetSocketListen->pkSetHeaderLenFun);
		if(kHSocket <= 0)
		{
			SeShutDown(kSocket);
			SeCloseSocket(kSocket);
			SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[ACCEPT SOCKET] SocketMgr is full");
			continue;
		}
		
		pkNetSocket = SeNetSocketMgrGet(&pkNetCore->kSocketMgr, kHSocket);
		pkNetSocket->iDomain = pkNetSocketListen->iDomain;
		pkNetSocket->usStatus = SOCKET_STATUS_CONNECTED;
		pkNetSocket->iActiveTimeOut = pkNetSocketListen->iActiveTimeOut;
		pkNetSocket->llTime = SeTimeGetTickCount();
		pkNetSocket->kBelongListenHSocket = pkNetSocketListen->kHSocket;
		SeSetAddrToBuf(pkNetSocket->iDomain, pkAddr, pkNetSocket->acIPAddr, (int)sizeof(pkNetSocket->acIPAddr), &pkNetSocket->iIPPort);
		SeNetSocketMgrAddSendOrRecvInList(&pkNetCore->kSocketMgr, pkNetSocket, true);
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP CLIENT] Accept client hsocket=0x%llx, ip=%s port=%d localsvrip=%s localsvrport=%d", \
			kHSocket, pkNetSocket->acIPAddr, pkNetSocket->iIPPort, pkNetSocketListen->acIPAddr, pkNetSocketListen->iIPPort);
		pkNetSocket->iIPPort = pkNetSocketListen->iIPPort;
	}
}

void SeNetCoreAcceptSocket(struct SENETCORE *pkNetCore, struct SESOCKET *pkNetSocket, bool bRead, bool bWrite, bool bError)
{
	bool bOK;

	if(bError == true)
	{
		SeNetCoreDisconnect(pkNetCore, pkNetSocket->kHSocket);
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[SeNetCoreAcceptSocket] accept error.socket=0x%llx", pkNetSocket->kHSocket);
		return;
	}

	if(bRead == true)
	{
		bOK = SeNetCoreRecvBuf(pkNetCore, pkNetSocket);
		if(!bOK)
		{
			SeNetCoreDisconnect(pkNetCore, pkNetSocket->kHSocket);
			return;
		}
		if(SeNetSreamCanRead(&pkNetSocket->kRecvNetStream, pkNetSocket->pkGetHeaderLenFun, pkNetSocket->iHeaderLen))
		{
			SeNetSocketMgrAddSendOrRecvInList(&pkNetCore->kSocketMgr, pkNetSocket, false);
		}
	}

	if(bWrite == true)
	{
		SeNetSocketMgrAddEvent(pkNetSocket, WRITE_EVENT_SOCKET);
		bOK = SeNetCoreSendBuf(pkNetCore, pkNetSocket);
		if(!bOK)
		{
			SeNetCoreDisconnect(pkNetCore, pkNetSocket->kHSocket);
			return;
		}
	}
}

void SeNetCoreClientSocket(struct SENETCORE *pkNetCore, struct SESOCKET *pkNetSocket, bool bRead, bool bWrite, bool bError)
{
	bool bOK;
	int iLen;
	int iErrorno;
	SOCKET socket;
	struct epoll_event kEvent;

	socket = pkNetSocket->kSocketFd.kSocket;

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
				SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[SeNetCoreClientSocket] connect ok. ip=%s port=%d socket=0x%llx", pkNetSocket->acIPAddr, pkNetSocket->iIPPort, pkNetSocket->kHSocket);
				return;
			}
			bOK = false;
		}

		if(bError == true || bOK == false)
		{
			pkNetSocket->usStatus = SOCKET_STATUS_CONNECTED_FAILED;
			SeShutDown(socket);
			SeCloseSocket(socket);
			SeNetSocketMgrAddSendOrRecvInList(&pkNetCore->kSocketMgr, pkNetSocket, true);
			SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[SeNetCoreClientSocket] connect failed. ip=%s port=%d socket=0x%llx", pkNetSocket->acIPAddr, pkNetSocket->iIPPort, pkNetSocket->kHSocket);
			return;
		}

		pkNetSocket->usStatus = SOCKET_STATUS_CONNECTED_FAILED;
		SeShutDown(socket);
		SeCloseSocket(socket);
		SeNetSocketMgrAddSendOrRecvInList(&pkNetCore->kSocketMgr, pkNetSocket, true);		
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[SeNetCoreClientSocket] connect status error. ip=%s port=%d socket=0x%llx", pkNetSocket->acIPAddr, pkNetSocket->iIPPort, pkNetSocket->kHSocket);

		// no call here?
		assert(0 != 0);
		return;
	}

	SeNetCoreAcceptSocket(pkNetCore, pkNetSocket, bRead, bWrite, bError);
}

bool SeNetCoreProcess(struct SENETCORE *pkNetCore, int *riEventSocket, HSOCKET *rkListenHSocket, HSOCKET *rkHSocket, char *pcBuf, int *riLen, int *rSSize, int *rRSize)
{
	bool bOK;
	int iTimeOut;
	struct epoll_event kEvent;
	struct SESOCKET *pkNetSocket;
	struct SESOCKET *pkConstNetSocket;

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
		iTimeOut = pkConstNetSocket->usStatus == SOCKET_STATUS_CONNECTING ? pkConstNetSocket->iConnectTimeOut : pkConstNetSocket->iActiveTimeOut;
		if((pkConstNetSocket->llTime + iTimeOut) <= SeTimeGetTickCount() && (pkConstNetSocket->usStatus == SOCKET_STATUS_CONNECTING || pkConstNetSocket->usStatus == SOCKET_STATUS_ACTIVECONNECT))
		{
			SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TIME OUT] Socket time out. RecvData count=%d SendData Count=%d socket=0x%llx port=%d", \
				SeNetSreamCount(&pkConstNetSocket->kRecvNetStream), SeNetSreamCount(&pkConstNetSocket->kSendNetStream), pkConstNetSocket->kHSocket, pkConstNetSocket->iIPPort);
			SeNetCoreDisconnect(pkNetCore, pkConstNetSocket->kHSocket);
		}
	}
	pkConstNetSocket = 0;

	do
	{
		bOK = false;
		pkNetSocket = SeNetSocketMgrPopSendOrRecvOutList(&pkNetCore->kSocketMgr, true);
		if(!pkNetSocket)
		{
			break;
		}

		*rkHSocket = pkNetSocket->kHSocket;
		*rkListenHSocket = pkNetSocket->kBelongListenHSocket;

		switch (pkNetSocket->usStatus)
		{
			case SOCKET_STATUS_CONNECTED:
			{
				*riEventSocket = SENETCORE_EVENT_SOCKET_CONNECT;
				SeStrNcpy(pcBuf, *riLen, pkNetSocket->acIPAddr);
				*riLen = (int)strlen(pkNetSocket->acIPAddr);
				pcBuf[*riLen] = '\0';
				kEvent.data.u64 = pkNetSocket->kHSocket;
				kEvent.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP | EPOLLET;
				epoll_ctl(pkNetCore->kHandle, EPOLL_CTL_ADD, pkNetSocket->kSocketFd.kSocket, &kEvent);
				pkNetSocket->usStatus = SOCKET_STATUS_ACTIVECONNECT;
				pkNetSocket->llTime = SeTimeGetTickCount();
				SeNetSocketMgrClearEvent(pkNetSocket, READ_EVENT_SOCKET);
				SeNetSocketMgrAddEvent(pkNetSocket, WRITE_EVENT_SOCKET);
				bOK = true;
				break;
			}
			case SOCKET_STATUS_CONNECTED_FAILED:
			{
				assert(pkNetSocket->iTypeSocket == CLIENT_TCP_TYPE_SOCKET);
				SeNetSocketMgrDel(&pkNetCore->kSocketMgr, pkNetSocket->kHSocket);
				*riEventSocket = SENETCORE_EVENT_SOCKET_CONNECT_FAILED;
				bOK = true;
				break;
			}
			case SOCKET_STATUS_DISCONNECT:
			{
				pkNetSocket->usStatus = SOCKET_STATUS_COMM_IDLE;
				*riEventSocket = SENETCORE_EVENT_SOCKET_DISCONNECT;
				SeNetSocketMgrAddSendOrRecvInList(&pkNetCore->kSocketMgr, pkNetSocket, true);
				bOK = true;
				break;
			}
			case SOCKET_STATUS_COMM_IDLE:
			{
				SeNetSocketMgrDel(&pkNetCore->kSocketMgr, pkNetSocket->kHSocket);
				break;
			}
			case SOCKET_STATUS_ACTIVECONNECT:
			{
				if (!SeNetCoreSendBuf(pkNetCore, pkNetSocket))
				{
					SeNetCoreDisconnect(pkNetCore, pkNetSocket->kHSocket);
				}
				break;
			}
			default:
			{
				SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "SeNetCoreProcess State Error. socket=0x%llx", pkNetSocket->kHSocket);
				break;
			}
		}
		if(bOK)
		{
			return true;
		}
	}while(true);

	do
	{
		pkNetSocket = SeNetSocketMgrPopSendOrRecvOutList(&pkNetCore->kSocketMgr, false);
		if(!pkNetSocket)
		{
			break;
		}
		
		if(pkNetSocket->usStatus != SOCKET_STATUS_ACTIVECONNECT)
		{
			continue;
		}
		
		if(!SeNetSreamCanRead(&pkNetSocket->kRecvNetStream, pkNetSocket->pkGetHeaderLenFun, pkNetSocket->iHeaderLen))
		{
			continue; 
		}
		bOK = SeNetSreamRead(&pkNetSocket->kRecvNetStream, &(pkNetCore->kSocketMgr.kNetStreamIdle), pkNetSocket->pkGetHeaderLenFun, pkNetSocket->iHeaderLen, pcBuf, riLen);
		if(!bOK)
		{
			SeNetCoreDisconnect(pkNetCore, pkNetSocket->kHSocket);
			SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[READ DATA] Read Data Error. socket=0x%llx", pkNetSocket->kHSocket);
			continue;
		}

		if(SeNetSreamCanRead(&pkNetSocket->kRecvNetStream, pkNetSocket->pkGetHeaderLenFun, pkNetSocket->iHeaderLen))
		{
			SeNetSocketMgrAddSendOrRecvInList(&pkNetCore->kSocketMgr, pkNetSocket, false);
		}
		
		pcBuf[*riLen] = '\0';
		*rkHSocket = pkNetSocket->kHSocket;
		*riEventSocket = SENETCORE_EVENT_SOCKET_RECV_DATA;
		*rkListenHSocket = pkNetSocket->kBelongListenHSocket;
		*rSSize = SeNetSreamCount(&pkNetSocket->kSendNetStream);
		*rRSize = SeNetSreamCount(&pkNetSocket->kRecvNetStream);

		return true;
	}while(true);

	return false;
}

bool SeNetCoreRead(struct SENETCORE *pkNetCore, int *riEvent, HSOCKET *rkListenHSocket, HSOCKET *rkHSocket, char *pcBuf, int *riLen, int *rSSize, int *rRSize)
{
	bool bWork;
	bool bHasaTimer;
	int i, iNum;
	HSOCKET kHSocket;
	bool bRead, bWrite, bError;
	struct epoll_event *pkEvent;
	struct SESOCKET *pkNetSocket;

	*rkHSocket = 0;
	*rkListenHSocket = 0;

	if(SeNetCoreProcess(pkNetCore, riEvent, rkListenHSocket, rkHSocket, pcBuf, riLen, rSSize, rRSize))
	{
		return true;
	}

	bHasaTimer = false;
	iNum = epoll_wait(pkNetCore->kHandle, pkNetCore->akEvents, sizeof(pkNetCore->akEvents)/sizeof(struct epoll_event), pkNetCore->iWaitTime);
	bWork = iNum > 0 ? true : false;

	for(i = 0; i < iNum; i++)
	{
		pkEvent = &pkNetCore->akEvents[i];
		bRead = pkEvent->events & EPOLLIN;
		bWrite =pkEvent->events & EPOLLOUT;
		kHSocket = (HSOCKET)pkEvent->data.u64;
		bError = (pkEvent->events & EPOLLRDHUP) || (pkEvent->events &  EPOLLERR) || (pkEvent->events &  EPOLLHUP);

		if(SeGetTimerID() == kHSocket)
		{
			if(!SeTimerRoutine(pkNetCore))
			{
				SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[EPOLL WAIT] socket timer read failed.");
			}
			else
			{
				bHasaTimer = true;
			}
			continue;
		}

		pkNetSocket = SeNetSocketMgrGet(&pkNetCore->kSocketMgr, kHSocket);
		if(!pkNetSocket)
		{
			SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[EPOLL WAIT] socket not found.socket=0x%llx", kHSocket);
			continue;
		}

		switch (pkNetSocket->iTypeSocket)
		{
			case LISTEN_TCP_TYPE_SOCKET:
			{
				if (pkNetSocket->usStatus == SOCKET_STATUS_ACTIVECONNECT)
				{
					SeNetCoreListenSocket(pkNetCore, pkNetSocket);
					break;
				}
				SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[EPOLL WAIT] LISTEN_TCP_TYPE_SOCKET state Error. typesocket=%d status=%d socket=0x%llx.", pkNetSocket->iTypeSocket, pkNetSocket->usStatus, pkNetSocket->kHSocket);
				break;
			}
			case CLIENT_TCP_TYPE_SOCKET:
			{
				if (pkNetSocket->usStatus == SOCKET_STATUS_ACTIVECONNECT || pkNetSocket->usStatus == SOCKET_STATUS_CONNECTING)
				{
					SeNetCoreClientSocket(pkNetCore, pkNetSocket, bRead, bWrite, bError);
					break;
				}
				SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[EPOLL WAIT] CLIENT_TCP_TYPE_SOCKET state Error. typesocket=%d status=%d socket=0x%llx.", pkNetSocket->iTypeSocket, pkNetSocket->usStatus, pkNetSocket->kHSocket);
				break;
			}
			case ACCEPT_TCP_TYPE_SOCKET:
			{
				if (pkNetSocket->usStatus == SOCKET_STATUS_ACTIVECONNECT)
				{
					SeNetCoreAcceptSocket(pkNetCore, pkNetSocket, bRead, bWrite, bError);
					break;
				}
				SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[EPOLL WAIT] ACCEPT_TCP_TYPE_SOCKET state Error. typesocket=%d status=%d socket=0x%llx.", pkNetSocket->iTypeSocket, pkNetSocket->usStatus, pkNetSocket->kHSocket);
				break;
			}
			default:
			{
				SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[EPOLL WAIT] SeNetCoreRead Error. typesocket=%d status=%d socket=0x%llx.", pkNetSocket->iTypeSocket, pkNetSocket->usStatus, pkNetSocket->kHSocket);
				break;
			}
		}
	}

	if(bHasaTimer)
	{
		*rkHSocket = 0;
		*rkListenHSocket = 0;
		*riEvent = SENETCORE_EVENT_SOCKET_TIMER;
		return true;
	}

	if(SeNetCoreProcess(pkNetCore, riEvent, rkListenHSocket, rkHSocket, pcBuf, riLen, rSSize, rRSize))
	{
		return true;
	}

	// has something todo
	if (SeHashGetHead(&(pkNetCore->kSocketMgr.kRecvList)))
	{
		bWork = true;
	}
	
	*rkHSocket = 0;
	*rkListenHSocket = 0;
	*riEvent = SENETCORE_EVENT_SOCKET_IDLE;
	return !bWork;
}

struct SESOCKET *SeNetCoreGetSocket(struct SENETCORE *pkNetCore, HSOCKET kHSocket)
{
	return SeNetSocketMgrGet(&pkNetCore->kSocketMgr, kHSocket);
}

#endif
