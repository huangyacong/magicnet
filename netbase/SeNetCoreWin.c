#include "SeNetCore.h"

#if (defined(_WIN32) || defined(WIN32))

#define OP_TYPE_SEND 1
#define OP_TYPE_RECV 2
#define OP_TYPE_ACCEPT 3
#define OP_TYPE_CONNECT 4

#if defined(IO_DATA_LEN)
#define MAX_IO_DATA_LEN 1024*512
#else
#define MAX_IO_DATA_LEN 1024*32
#endif

struct IODATA
{
	OVERLAPPED			overlapped;
	struct SENODE		kNode;
	HSOCKET				kHScoket;
	SOCKET				kSocket;
	WSABUF				kBuf;
	int					iOPType;
	char				acData[MAX_IO_DATA_LEN];
};

struct ListenSocket
{
	int					iBacklog;
	HSOCKET				kHScoket;
	struct SENODE		kNode;
};

struct IODATA* SeNewIOData(struct SENETCORE *pkNetCore)
{
	struct SENODE* pkNode;
	struct IODATA* pkIOData;

	pkNode = SeListHeadPop(&pkNetCore->kList);
	if(pkNode) { return SE_CONTAINING_RECORD(pkNode, struct IODATA, kNode); }
	
	pkIOData = (struct IODATA*)SeMallocMem(sizeof(struct IODATA));
	if(!pkIOData) { return 0; }
	memset(pkIOData, 0, sizeof(struct IODATA));
	SeListInitNode(&pkIOData->kNode);
	return pkIOData;
}

void SeDelIOData(struct SENETCORE *pkNetCore, struct IODATA* pkIOData)
{
	SeListTailAddList(&pkNetCore->kList, &pkIOData->kNode);
}

void SeFreeIOData(struct SENETCORE *pkNetCore)
{
	struct SENODE* pkNode;
	struct IODATA* pkIOData;

	pkNode = SeListHeadPop(&pkNetCore->kList);
	while(pkNode)
	{
		pkIOData = SE_CONTAINING_RECORD(pkNode, struct IODATA, kNode);
		SeFreeMem(pkIOData);
		pkNode = SeListHeadPop(&pkNetCore->kList);
	}
}

bool SeAddListenSocket(struct SENETCORE *pkNetCore, HSOCKET kHScoket)
{
	struct ListenSocket* pkListenSocket;

	pkListenSocket = (struct ListenSocket*)SeMallocMem(sizeof(struct ListenSocket));
	if(!pkListenSocket) { return false; }
	memset(pkListenSocket, 0, sizeof(struct ListenSocket));
	SeListInitNode(&pkListenSocket->kNode);
	pkListenSocket->kHScoket = kHScoket;
	pkListenSocket->iBacklog = 0;
	SeListTailAddList(&pkNetCore->kListenList, &pkListenSocket->kNode);
	return true;
}

struct ListenSocket* SeGetListenSocket(struct SENETCORE *pkNetCore)
{
	struct SENODE* pkNode;
	struct ListenSocket* pkListenSocket;

	pkNode = SeListHeadPop(&pkNetCore->kListenList);
	if(!pkNode) { return 0; }
	pkListenSocket = SE_CONTAINING_RECORD(pkNode, struct ListenSocket, kNode);
	SeListTailAddList(&pkNetCore->kListenList, &pkListenSocket->kNode);
	return pkListenSocket;
}

struct ListenSocket* SeGetListenSocketByHSocket(struct SENETCORE *pkNetCore, HSOCKET kHScoket)
{
	struct SENODE* pkNode;
	struct ListenSocket* pkListenSocket;

	pkNode = pkNetCore->kListenList.head;
	while(pkNode)
	{
		pkListenSocket = SE_CONTAINING_RECORD(pkNode, struct ListenSocket, kNode);
		if(pkListenSocket->kHScoket == kHScoket) { return pkListenSocket; }
		pkNode = pkNode->next;
	}

	return 0;
}

void SeFreeListenSocket(struct SENETCORE *pkNetCore)
{
	struct SENODE* pkNode;
	struct ListenSocket* pkListenSocket;

	pkNode = SeListHeadPop(&pkNetCore->kListenList);
	while(pkNode)
	{
		pkListenSocket = SE_CONTAINING_RECORD(pkNode, struct ListenSocket, kNode);
		SeFreeMem(pkListenSocket);
		pkNode = SeListHeadPop(&pkNetCore->kListenList);
	}
}

void SeNetCoreInit(struct SENETCORE *pkNetCore, char *pcLogName, unsigned short usMax, int iLogLV)
{
	pkNetCore->iWaitTime = NET_CORE_WAIT_TIME;
	SeNetBaseInit();
	SeInitLog(&pkNetCore->kLog, pcLogName);
	SeListInit(&pkNetCore->kList);
	SeListInit(&pkNetCore->kListenList);
	SeAddLogLV(&pkNetCore->kLog, iLogLV);
	pkNetCore->kHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	SeNetSocketMgrInit(&pkNetCore->kSocketMgr, usMax);
}

void SeNetCoreFin(struct SENETCORE *pkNetCore)
{
	SeCloseHandle(pkNetCore->kHandle);
	SeFinLog(&pkNetCore->kLog);
	SeNetSocketMgrFin(&pkNetCore->kSocketMgr);
	SeFreeIOData(pkNetCore);
	SeFreeListenSocket(pkNetCore);
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

bool SeNetCoreAcceptEx(struct SENETCORE *pkNetCore, HSOCKET kListenHSocket)
{
	BOOL bRet;
	int iErrorno;
	DWORD dwBytes;
	SOCKET socket;
	struct IODATA *pkIOData;
	LPFN_ACCEPTEX lpfnAcceptEx;
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	
	lpfnAcceptEx = NULL;

	if(WSAIoctl(SeGetSocketByHScoket(kListenHSocket), SIO_GET_EXTENSION_FUNCTION_POINTER,
			&GuidAcceptEx, sizeof(GuidAcceptEx),
			&lpfnAcceptEx, sizeof(lpfnAcceptEx),
			&dwBytes, NULL, NULL) == SE_SOCKET_ERROR)
	{
		iErrorno = SeErrno();
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[GET ACCEPT POINT] get AcceptEx point failed, errno=%d", iErrorno);
		return false;
	}
	
	socket = SeSocket(SOCK_STREAM);
	if(socket == SE_INVALID_SOCKET)
	{
		iErrorno = SeErrno();
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[GET ACCEPT POINT] new client socket failed, errno=%d", iErrorno);
		return false;
	}
	
	pkIOData = SeNewIOData(pkNetCore);
	if(!pkIOData)
	{
		SeCloseSocket(socket);
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[GET ACCEPT POINT] new mem failed");
		return false;
	}

	memset(&pkIOData->overlapped, 0, sizeof(OVERLAPPED));
	pkIOData->kHScoket = kListenHSocket;
	pkIOData->kSocket = socket;
	pkIOData->iOPType = OP_TYPE_ACCEPT;

	bRet = lpfnAcceptEx(SeGetSocketByHScoket(kListenHSocket), socket,
			pkIOData->acData, 0,
			sizeof(struct sockaddr_in) + 16, sizeof(struct sockaddr_in) + 16,
			&dwBytes, &pkIOData->overlapped);

	if(bRet)
	{
		return true;
	}

	iErrorno = SeErrno();
	if(iErrorno != ERROR_IO_PENDING)
	{
		SeCloseSocket(socket);
		SeDelIOData(pkNetCore, pkIOData);
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[GET ACCEPT POINT] add socket failed, errno=%d", iErrorno);
		return false;
	}
	return true;
}

void SeSetAcceptSocket(struct SENETCORE *pkNetCore)
{
	struct ListenSocket* pkListenSocket;

	pkListenSocket = SeGetListenSocket(pkNetCore);
	if(!pkListenSocket)
	{
		return;
	}
	if(pkListenSocket->iBacklog >= SENETCORE_SOCKET_BACKLOG) { return; }
	if(SeNetCoreAcceptEx(pkNetCore, pkListenSocket->kHScoket))
	{
		pkListenSocket->iBacklog++;
		return;
	}
	SeLogWrite(&pkNetCore->kLog, LT_ERROR, true, "[ACCEPT SOCKET] Send Accept Socket Failed. Backlog=%d", pkListenSocket->iBacklog);
}

HSOCKET SeNetCoreTCPListen(struct SENETCORE *pkNetCore, const char *pcIP, unsigned short usPort,\
	int iHeaderLen, int iTimeOut, SEGETHEADERLENFUN pkGetHeaderLenFun, SESETHEADERLENFUN pkSetHeaderLenFun)
{
	int backlog;
	int iErrorno;
	SOCKET socket;
	HSOCKET kHSocket;
	struct sockaddr kAddr;
	struct SESOCKET *pkNetSocket;
	
	SeSetSockAddr(&kAddr, pcIP, usPort);
	socket = SeSocket(SOCK_STREAM);
	if(socket == SE_INVALID_SOCKET)
	{
		iErrorno = SeErrno();
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP LISTEN] Create Socket ERROR, errno=%d IP=%s port=%d", iErrorno, pcIP, usPort);
		return 0;
	}
	if(!CreateIoCompletionPort((HANDLE)socket, pkNetCore->kHandle, 0, 0))
	{
		iErrorno = SeErrno();
		SeCloseSocket(socket);
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP LISTEN] epoll_ctl ERROR, errno=%d IP=%s port=%d", iErrorno, pcIP, usPort);
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
	/*
	if(SeSetReuseAddr(socket) != 0)
	{
		iErrorno = SeErrno();
		SeCloseSocket(socket);
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP LISTEN] SeSetReuseAddr ERROR, errno=%d IP=%s port=%d", iErrorno, pcIP, usPort);
		return 0;
	}
	*/
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

	SeAddListenSocket(pkNetCore, kHSocket);
	
	SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP LISTEN] SocketMgr listen ok, IP=%s port=%d", pcIP, usPort);
	pkNetSocket = SeNetSocketMgrGet(&pkNetCore->kSocketMgr, kHSocket);
	pkNetSocket->usStatus = SOCKET_STATUS_ACTIVECONNECT;
	pkNetSocket->llActiveTimeOut = iTimeOut;
	memcpy(&pkNetSocket->kRemoteAddr, &kAddr, sizeof(struct sockaddr));
	
	return kHSocket;
}

HSOCKET SeNetCoreTCPClient(struct SENETCORE *pkNetCore, const char *pcIP, unsigned short usPort,\
	int iHeaderLen, int iTimeOut, SEGETHEADERLENFUN pkGetHeaderLenFun, SESETHEADERLENFUN pkSetHeaderLenFun)
{
	int iErrorno;
	SOCKET socket;
	HSOCKET kHSocket;
	DWORD dwSend,dwBytes;
	struct sockaddr kAddr;
	struct sockaddr local;
	struct linger so_linger;
	struct IODATA *pkIOData;
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
	
	LPFN_CONNECTEX ConnectEx;
    GUID guidConnectEx = WSAID_CONNECTEX;
    if(SOCKET_ERROR == WSAIoctl(socket, SIO_GET_EXTENSION_FUNCTION_POINTER,
						&guidConnectEx, sizeof(guidConnectEx), &ConnectEx, sizeof(ConnectEx), &dwBytes, NULL, NULL))
    {
		iErrorno = SeErrno();
		SeCloseSocket(socket);
		SeNetSocketMgrDel(&pkNetCore->kSocketMgr, kHSocket);
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP CLIENT] get ConnectEx failed, errno=%d", iErrorno);
		return 0;
    }

	SeSetSockAddr(&local, "0.0.0.0", 0);
    if(SOCKET_ERROR == SeBind(socket, &local))
    {
        iErrorno = SeErrno();
		SeCloseSocket(socket);
		SeNetSocketMgrDel(&pkNetCore->kSocketMgr, kHSocket);
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP CLIENT] SeBind failed, errno=%d", iErrorno);
		return 0;
    }
	if(!CreateIoCompletionPort((HANDLE)socket, pkNetCore->kHandle, 0, 0))
	{
		iErrorno = SeErrno();
		SeCloseSocket(socket);
		SeNetSocketMgrDel(&pkNetCore->kSocketMgr, kHSocket);
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP CLIENT] CreateIoCompletionPort failed, errno=%d", iErrorno);
		return 0;
	}

	pkIOData = SeNewIOData(pkNetCore);
	if(!pkIOData)
	{
		SeCloseSocket(socket);
		SeNetSocketMgrDel(&pkNetCore->kSocketMgr, kHSocket);
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP CLIENT] new mem failed");
		return 0;
	}

	memset(&pkIOData->overlapped, 0, sizeof(OVERLAPPED));
	pkIOData->kHScoket = kHSocket;
	pkIOData->iOPType = OP_TYPE_CONNECT;
 
	if(!ConnectEx(socket, &kAddr, sizeof(struct sockaddr), NULL, 0, &dwSend, &pkIOData->overlapped))
	{
		iErrorno = SeErrno();
		if(ERROR_IO_PENDING != iErrorno)
		{
			SeDelIOData(pkNetCore, pkIOData);
			SeCloseSocket(socket);
			SeNetSocketMgrDel(&pkNetCore->kSocketMgr, kHSocket);
			SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP CLIENT] ConnectEx failed, errno=%d", iErrorno);
			return 0;
		}
	}
	
	pkNetSocket = SeNetSocketMgrGet(&pkNetCore->kSocketMgr, kHSocket);
	memcpy(&pkNetSocket->kRemoteAddr, &kAddr, sizeof(struct sockaddr));
	pkNetSocket->usStatus = SOCKET_STATUS_CONNECTING;
	pkNetSocket->llActiveTimeOut = iTimeOut;
	SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP CLIENT] ConnectEx to svr, ip=%s port=%d socket=%llx", inet_ntoa(pkNetSocket->kRemoteAddr.sin_addr), ntohs(pkNetSocket->kRemoteAddr.sin_port), kHSocket);
	return kHSocket;
}

bool SeNetCoreSend(struct SENETCORE *pkNetCore, HSOCKET kHSocket, const char* pcBuf, int iSize)
{
	bool bRet;
	struct SESOCKET *pkNetSocket;
	
	if(iSize < 0) { SeLogWrite(&pkNetCore->kLog, LT_ERROR, true, "[CORE SEND] Send Data Error. Socket=%llx.Size=%d", kHSocket, iSize); return false; }
	pkNetSocket = SeNetSocketMgrGet(&pkNetCore->kSocketMgr, kHSocket);
	if(!pkNetSocket) return false;
	if(pkNetSocket->iTypeSocket != CLIENT_TCP_TYPE_SOCKET && pkNetSocket->iTypeSocket != ACCEPT_TCP_TYPE_SOCKET) return false;
	if(pkNetSocket->usStatus != SOCKET_STATUS_ACTIVECONNECT) return false;
	if(!SeNetSocketMgrUpdateNetStreamIdle(&pkNetCore->kSocketMgr, pkNetSocket->iHeaderLen, iSize)) { SeLogWrite(&pkNetCore->kLog, LT_ERROR, true, "[CORE SEND] no more memcahce.%llx", kHSocket); return false; }
	if(!SeNetSreamCanWrite(&pkNetSocket->kSendNetStream, pkNetSocket->pkSetHeaderLenFun, pkNetSocket->iHeaderLen, iSize)) { SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[CORE SEND] send data ERROR.socket=%llx.size=%d", kHSocket, iSize); return false; }
	bRet = SeNetSreamWrite(&pkNetSocket->kSendNetStream, pkNetCore->kSocketMgr.pkNetStreamIdle, pkNetSocket->pkSetHeaderLenFun, pkNetSocket->iHeaderLen, pcBuf, iSize);
	if(bRet) { SeNetSocketMgrAddSendOrRecvInList(&pkNetCore->kSocketMgr, pkNetSocket, true); }
	else { SeLogWrite(&pkNetCore->kLog, LT_ERROR, true, "[CORE SEND] Send Data Error, Now Close The Socket=%llx.Size=%d", kHSocket, iSize); SeNetCoreDisconnect(pkNetCore, pkNetSocket->kHSocket); }

	return bRet;
}

bool SeNetCoreSendExtend(struct SENETCORE *pkNetCore, HSOCKET kHSocket, const struct SENETSTREAMBUF *pkBufList, int iNum)
{
	int i;
	bool bRet;
	int iSize;
	struct SESOCKET *pkNetSocket;

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
	if(pkNetSocket->iTypeSocket != CLIENT_TCP_TYPE_SOCKET && pkNetSocket->iTypeSocket != ACCEPT_TCP_TYPE_SOCKET) return false;
	if(pkNetSocket->usStatus != SOCKET_STATUS_ACTIVECONNECT) return false;
	if(!SeNetSocketMgrUpdateNetStreamIdle(&pkNetCore->kSocketMgr, pkNetSocket->iHeaderLen, iSize)) { SeLogWrite(&pkNetCore->kLog, LT_ERROR, true, "[CORE SEND] no more memcahce.%llx", kHSocket); return false; }
	if(!SeNetSreamCanWrite(&pkNetSocket->kSendNetStream, pkNetSocket->pkSetHeaderLenFun, pkNetSocket->iHeaderLen, iSize)) { SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[CORE SEND] send data ERROR.socket=%llx.size=%d", kHSocket, iSize); return false; }
	bRet = SeNetSreamWriteExtend(&pkNetSocket->kSendNetStream, pkNetCore->kSocketMgr.pkNetStreamIdle, pkNetSocket->pkSetHeaderLenFun, pkNetSocket->iHeaderLen, pkBufList, iNum);
	if(bRet) { SeNetSocketMgrAddSendOrRecvInList(&pkNetCore->kSocketMgr, pkNetSocket, true); }
	else { SeLogWrite(&pkNetCore->kLog, LT_ERROR, true, "[CORE SEND] Send Data Error, Now Close The Socket=%llx.Size=%d", kHSocket, iSize); SeNetCoreDisconnect(pkNetCore, pkNetSocket->kHSocket); }

	return bRet;
}

void SeNetCoreDisconnect(struct SENETCORE *pkNetCore, HSOCKET kHSocket)
{
	SOCKET socket;
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
	SeCloseSocket(socket);
	SeNetSocketMgrAddSendOrRecvInList(&pkNetCore->kSocketMgr, pkNetSocket, true);
}

bool SeNetCoreSendBuf(struct SENETCORE *pkNetCore, struct SESOCKET *pkNetSocket)
{
	DWORD dwLen;
	int iSize;
	int iErrorno;
	SOCKET socket;
	struct IODATA *pkIOData;
	
	socket = SeGetSocketByHScoket(pkNetSocket->kHSocket);
	if(SeNetSocketMgrHasEvent(pkNetSocket, WRITE_EVENT_SOCKET)) { return true; }

	if(SeNetSreamCount(&pkNetSocket->kSendNetStream) <= 0)
	{
		return true;
	}

	pkIOData = SeNewIOData(pkNetCore);
	if(!pkIOData)
	{
		SeLogWrite(&pkNetCore->kLog, LT_ERROR, true, "[SEND DATA] new mem failed.socket=%llx", pkNetSocket->kHSocket);
		return false;
	}

	memset(&pkIOData->overlapped, 0, sizeof(OVERLAPPED));
	pkIOData->kHScoket = pkNetSocket->kHSocket;
	pkIOData->iOPType = OP_TYPE_SEND;
	pkIOData->kBuf.buf = pkIOData->acData;
	pkIOData->kBuf.len = (int)sizeof(pkIOData->acData);

	iSize = pkIOData->kBuf.len;
	if(!SeNetSreamRead(&pkNetSocket->kSendNetStream, pkNetCore->kSocketMgr.pkNetStreamIdle, pkNetSocket->pkGetHeaderLenFun, 0, pkIOData->kBuf.buf, &iSize))
	{
		SeLogWrite(&pkNetCore->kLog, LT_ERROR, true, "[SeNetCoreSendBuf] mem error.socket=%llx", pkNetSocket->kHSocket);
		SeDelIOData(pkNetCore, pkIOData);
		return false;
	}
	pkIOData->kBuf.len = iSize;

	dwLen = 0;
	if(WSASend(socket, &pkIOData->kBuf, 1, &dwLen, 0, &pkIOData->overlapped, 0) == SE_SOCKET_ERROR)
	{
		iErrorno = SeErrno();
		if(iErrorno != WSA_IO_PENDING)
		{
			SeDelIOData(pkNetCore, pkIOData);
			SeLogWrite(&pkNetCore->kLog, LT_DEBUG, true, "[SEND DATA] SeNetCoreSendBuf WSASend failed, socket=%llx errno=%d", pkNetSocket->kHSocket, iErrorno);
			return false;
		}
	}
	
	SeNetSocketMgrAddEvent(pkNetSocket, WRITE_EVENT_SOCKET);

	if(SeGetNetSreamLen(&pkNetSocket->kSendNetStream) >= SENETCORE_SOCKET_RS_BUF_LEN)
	{
		SeLogWrite(&pkNetCore->kLog, LT_ERROR, true, "[SEND MORE BUF] SendBuf Too More And Close It.socket=%llx.size>%d", pkNetSocket->kHSocket, SENETCORE_SOCKET_RS_BUF_LEN);
		return false;
	}

	return true;
}

bool SeNetCoreRecvBuf(struct SENETCORE *pkNetCore, struct SESOCKET *pkNetSocket)
{
	int iFlags;
	DWORD dwLen;
	int iErrorno;
	SOCKET socket;
	struct IODATA *pkIOData;
	
	socket = SeGetSocketByHScoket(pkNetSocket->kHSocket);
	if(SeNetSocketMgrHasEvent(pkNetSocket, READ_EVENT_SOCKET)) { return true; }

	pkIOData = SeNewIOData(pkNetCore);
	if(!pkIOData)
	{
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[RECV DATA] new mem failed.socket=%llx", pkNetSocket->kHSocket);
		return false;
	}

	memset(&pkIOData->overlapped, 0, sizeof(OVERLAPPED));
	pkIOData->kHScoket = pkNetSocket->kHSocket;
	pkIOData->iOPType = OP_TYPE_RECV;
	pkIOData->kBuf.buf = pkIOData->acData;
	pkIOData->kBuf.len = sizeof(pkIOData->acData);

	iFlags = 0;
	dwLen = 0;
	if(WSARecv(socket, &pkIOData->kBuf, 1, &dwLen, (LPDWORD)&iFlags, &pkIOData->overlapped, 0) == SE_SOCKET_ERROR)
	{
		iErrorno = SeErrno();
		if(iErrorno != WSA_IO_PENDING)
		{
			SeDelIOData(pkNetCore, pkIOData);
			SeLogWrite(&pkNetCore->kLog, LT_DEBUG, true, "[RECV DATA] SeNetCoreRecvBuf WSARecv failed, socket=%llx errno=%d", pkNetSocket->kHSocket, iErrorno);
			return false;
		}
	}
	
	SeNetSocketMgrAddEvent(pkNetSocket, READ_EVENT_SOCKET);

	if(SeGetNetSreamLen(&pkNetSocket->kRecvNetStream) >= SENETCORE_SOCKET_RS_BUF_LEN) 
	{
		SeLogWrite(&pkNetCore->kLog, LT_ERROR, true, "[RECV MORE BUF] RecvBuf Too More And Close It.socket=%llx.size>%d", pkNetSocket->kHSocket, SENETCORE_SOCKET_RS_BUF_LEN);
		return false;
	}

	return true;
}

void SeNetCoreListenSocket(struct SENETCORE *pkNetCore, struct SESOCKET *pkNetSocketListen, SOCKET kSocket, struct IODATA *pkIOData)
{
	int							iErrorno;
	HSOCKET						kHSocket;
	struct linger				so_linger;
	char						*pcAddrIP;
	char						acLocalIP[128];
	struct ListenSocket			*pkListenSocket;
	struct SESOCKET				*pkNetSocketAccept;

	LPFN_GETACCEPTEXSOCKADDRS	lpfnGetAcceptExSockaddrs = NULL;
	GUID						tGuidGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;
	DWORD						dwBytes = 0;
	int							tResult = 0;

	struct	sockaddr_in			*local_addr = NULL;
	struct  sockaddr_in			*remote_addr = NULL;
	int							local_addr_len = sizeof(struct sockaddr_in);
	int							remote_addr_len = sizeof(struct sockaddr_in);

	pkListenSocket = SeGetListenSocketByHSocket(pkNetCore, pkNetSocketListen->kHSocket);
	if(pkListenSocket){ pkListenSocket->iBacklog--; }

	if(!pkListenSocket)
	{
		SeLogWrite(&pkNetCore->kLog, LT_ERROR, true, "[ACCEPT SOCKET] Can't Get Listen Obj.");
	}

	SeSetAcceptSocket(pkNetCore);

	tResult = WSAIoctl(SeGetSocketByHScoket(pkNetSocketListen->kHSocket), SIO_GET_EXTENSION_FUNCTION_POINTER,
						&tGuidGetAcceptExSockaddrs, sizeof(tGuidGetAcceptExSockaddrs),
						&lpfnGetAcceptExSockaddrs, sizeof(lpfnGetAcceptExSockaddrs),
						&dwBytes, NULL, NULL);
	
	

	if(tResult != SE_SOCKET_ERROR)
	{
		lpfnGetAcceptExSockaddrs(pkIOData->acData, 0, 
							sizeof(struct sockaddr_in)+16, sizeof(struct sockaddr_in)+16,
							(struct sockaddr**)&local_addr, &local_addr_len,
							(struct sockaddr**)&remote_addr, &remote_addr_len);
	}
	else
	{
		iErrorno = SeErrno();
		SeCloseSocket(kSocket);
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[ACCEPT SOCKET] addr is error, %d", iErrorno);
		return;
	}

	if(SeSetNoBlock(kSocket, true) != 0)
	{
		iErrorno = SeErrno();
		SeCloseSocket(kSocket);
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[ACCEPT SOCKET] SeSetNoBlock ERROR, errno=%d", iErrorno);
		return;
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
		return;
	}
	pkNetSocketAccept = SeNetSocketMgrGet(&pkNetCore->kSocketMgr, kHSocket);
	memcpy(&pkNetSocketAccept->kRemoteAddr, remote_addr, sizeof(struct sockaddr_in));
	pkNetSocketAccept->usStatus = SOCKET_STATUS_CONNECTED;
	pkNetSocketAccept->llActiveTimeOut = pkNetSocketListen->llActiveTimeOut;
	pkNetSocketAccept->llTime = SeTimeGetTickCount();
	pkNetSocketAccept->kBelongListenHSocket = pkNetSocketListen->kHSocket;
	SeNetSocketMgrAddSendOrRecvInList(&pkNetCore->kSocketMgr, pkNetSocketAccept, true);
	pcAddrIP = inet_ntoa(pkNetSocketListen->kRemoteAddr.sin_addr);
	SeStrNcpy(acLocalIP, (int)sizeof(acLocalIP), pcAddrIP ? pcAddrIP : "");
	SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP CLIENT] Accept client, ip=%s port=%d localsvrip=%s localsvrport=%d", \
		inet_ntoa(pkNetSocketAccept->kRemoteAddr.sin_addr), ntohs(pkNetSocketAccept->kRemoteAddr.sin_port), acLocalIP, ntohs(pkNetSocketListen->kRemoteAddr.sin_port));
}

void SeNetCoreAcceptSocket(struct SENETCORE *pkNetCore, struct SESOCKET *pkNetSocket, const struct IODATA *pkIOData, DWORD dwLen)
{
	bool bOK;
	int iErrorno;
	SOCKET socket;
	DWORD dwSendLen;
	struct IODATA *pkSendIOData;

	socket = SeGetSocketByHScoket(pkNetSocket->kHSocket);

	if(dwLen <= 0)
	{
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[ACCEPT SOCKET] SeNetCoreAcceptSocket socket close by client.socket=%llx", pkNetSocket->kHSocket);
		SeNetCoreDisconnect(pkNetCore, pkNetSocket->kHSocket);
		return;
	}
	
	if(pkIOData->iOPType == OP_TYPE_SEND)
	{
		if(dwLen >= pkIOData->kBuf.len)
		{
			SeNetSocketMgrClearEvent(pkNetSocket, WRITE_EVENT_SOCKET);
			bOK = SeNetCoreSendBuf(pkNetCore, pkNetSocket);
			if(!bOK) { SeNetCoreDisconnect(pkNetCore, pkNetSocket->kHSocket); return; }
		}
		else
		{
			pkSendIOData = SeNewIOData(pkNetCore);
			if(!pkSendIOData)
			{
				SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[SEND DATA] new mem failed. socket=%llx", pkNetSocket->kHSocket);
				SeNetCoreDisconnect(pkNetCore, pkNetSocket->kHSocket);
				return;
			}

			memset(&pkSendIOData->overlapped, 0, sizeof(OVERLAPPED));
			pkSendIOData->kHScoket = pkNetSocket->kHSocket;
			pkSendIOData->iOPType = OP_TYPE_SEND;
			pkSendIOData->kBuf.buf = pkSendIOData->acData;
			pkSendIOData->kBuf.len = pkIOData->kBuf.len - dwLen;
			memcpy(pkSendIOData->kBuf.buf, pkIOData->kBuf.buf + dwLen, pkIOData->kBuf.len - dwLen);
			SeLogWrite(&pkNetCore->kLog, LT_WARNING, true, "[SEND DATA] wsasend buf leave some data,now send again.socket=%llx", pkNetSocket->kHSocket);

			dwSendLen = 0;
			if(WSASend(socket, &pkSendIOData->kBuf, 1, &dwSendLen, 0, &pkSendIOData->overlapped, 0) == SE_SOCKET_ERROR)
			{
				iErrorno = SeErrno();
				if(iErrorno != WSA_IO_PENDING)
				{
					SeDelIOData(pkNetCore, pkSendIOData);
					SeNetCoreDisconnect(pkNetCore, pkNetSocket->kHSocket);
					SeLogWrite(&pkNetCore->kLog, LT_DEBUG, true, "[SEND DATA] SeNetCoreAcceptSocket WSASend failed, socket=%llx errno=%d", pkNetSocket->kHSocket, iErrorno);
					return;
				}
			}
		}
	}
	else if(pkIOData->iOPType == OP_TYPE_RECV)
	{
		SeNetSocketMgrActive(&pkNetCore->kSocketMgr, pkNetSocket);
		if(!SeNetSocketMgrUpdateNetStreamIdle(&pkNetCore->kSocketMgr, pkNetSocket->iHeaderLen, dwLen))
		{
			SeLogWrite(&pkNetCore->kLog, LT_ERROR, true, "[CORE RECV] no more memcahce.%llx", pkNetSocket->kHSocket);
			SeNetCoreDisconnect(pkNetCore, pkNetSocket->kHSocket);
			return;
		}
		bOK = SeNetSreamWrite(&pkNetSocket->kRecvNetStream, pkNetCore->kSocketMgr.pkNetStreamIdle, pkNetSocket->pkSetHeaderLenFun, 0, pkIOData->kBuf.buf, dwLen);
		if(!bOK) { SeNetCoreDisconnect(pkNetCore, pkNetSocket->kHSocket); SeLogWrite(&pkNetCore->kLog, LT_ERROR, true, "[CORE RECV] recv data ERROR. %llx", pkNetSocket->kHSocket); return; }
		SeNetSocketMgrClearEvent(pkNetSocket, READ_EVENT_SOCKET);
		bOK = SeNetCoreRecvBuf(pkNetCore, pkNetSocket);
		if(!bOK) { SeNetCoreDisconnect(pkNetCore, pkNetSocket->kHSocket); return; }
		SeNetSocketMgrAddSendOrRecvInList(&pkNetCore->kSocketMgr, pkNetSocket, false);
	}
	else
	{
		SeLogWrite(&pkNetCore->kLog, LT_ERROR, true, "[CORE RECV] SeNetCoreAcceptSocket OPType Error. type=%d socket=%llx", pkIOData->iOPType, pkNetSocket->kHSocket);
	}
}

void SeNetCoreClientSocket(struct SENETCORE *pkNetCore, struct SESOCKET *pkNetSocket, const struct IODATA *pkIOData, DWORD dwLen, BOOL bConnectOK)
{
	if(pkIOData->iOPType == OP_TYPE_CONNECT)
	{
		assert(pkNetSocket->usStatus == SOCKET_STATUS_CONNECTING);

		if(pkNetSocket->usStatus != SOCKET_STATUS_CONNECTING)
		{
			SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "SeNetCoreClientSocket status=%d error.socket=%llx", pkNetSocket->usStatus, pkNetSocket->kHSocket);
		}

		if(bConnectOK)
		{
			pkNetSocket->usStatus = SOCKET_STATUS_CONNECTED;
			pkNetSocket->llTime = SeTimeGetTickCount();
			SeNetSocketMgrAddSendOrRecvInList(&pkNetCore->kSocketMgr, pkNetSocket, true);
		}
		else
		{
			SeNetCoreDisconnect(pkNetCore, pkNetSocket->kHSocket);
		}

		return;
	}

	SeNetCoreAcceptSocket(pkNetCore, pkNetSocket, pkIOData, dwLen);
}

bool SeNetCoreProcess(struct SENETCORE *pkNetCore, int *riEventSocket, HSOCKET *rkListenHSocket, HSOCKET *rkHSocket, char *pcBuf, int *riLen, int *rSSize, int *rRSize)
{
	bool bOK;
	SOCKET socket;
	char *pcAddrIP;
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
			SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TIME OUT] Socket time out. socket=%llx", pkConstNetSocket->kHSocket);
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
			socket = SeGetSocketByHScoket(pkNetSocket->kHSocket);
			CreateIoCompletionPort((HANDLE)socket, pkNetCore->kHandle, 0, 0);
			*riLen = 0;
			pcAddrIP = inet_ntoa(pkNetSocket->kRemoteAddr.sin_addr);
			if(pcAddrIP) { strcpy(pcBuf, pcAddrIP); *riLen = (int)strlen(pcAddrIP); pcBuf[*riLen] = '\0'; }
			*riEventSocket = SENETCORE_EVENT_SOCKET_CONNECT;
			pkNetSocket->usStatus = SOCKET_STATUS_ACTIVECONNECT;
			pkNetSocket->llTime = SeTimeGetTickCount();
			SeNetSocketMgrClearEvent(pkNetSocket, READ_EVENT_SOCKET);
			SeNetSocketMgrClearEvent(pkNetSocket, WRITE_EVENT_SOCKET);
			SeNetCoreRecvBuf(pkNetCore, pkNetSocket);
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
	bool bResult;
	DWORD dwLen;
	ULONG_PTR ulKey;
	struct IODATA *pkIOData;
	OVERLAPPED* pkOverlapped;
	struct SESOCKET *pkNetSocket;
	
	bWork = false;
	pkOverlapped = NULL;

	SeSetAcceptSocket(pkNetCore);

	bResult = GetQueuedCompletionStatus(pkNetCore->kHandle, &dwLen, &ulKey, &pkOverlapped, pkNetCore->iWaitTime) == TRUE ? true : false;
	if(pkOverlapped)
	{
		bWork = true;
		pkIOData = SE_CONTAINING_RECORD(pkOverlapped, struct IODATA, overlapped);
		pkNetSocket = SeNetSocketMgrGet(&pkNetCore->kSocketMgr, pkIOData->kHScoket);

		if(pkNetSocket)
		{
			if(pkNetSocket->iTypeSocket == LISTEN_TCP_TYPE_SOCKET && pkNetSocket->usStatus == SOCKET_STATUS_ACTIVECONNECT) { SeNetCoreListenSocket(pkNetCore, pkNetSocket, pkIOData->kSocket, pkIOData); }
			else if(pkNetSocket->iTypeSocket == ACCEPT_TCP_TYPE_SOCKET && pkNetSocket->usStatus == SOCKET_STATUS_ACTIVECONNECT) { SeNetCoreAcceptSocket(pkNetCore, pkNetSocket, pkIOData, dwLen); }
			else if(pkNetSocket->iTypeSocket == CLIENT_TCP_TYPE_SOCKET && (pkNetSocket->usStatus == SOCKET_STATUS_ACTIVECONNECT || pkNetSocket->usStatus == SOCKET_STATUS_CONNECTING)) { SeNetCoreClientSocket(pkNetCore, pkNetSocket, pkIOData, dwLen, bResult); }
			else
			{
				SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[EPOLL WAIT] SeNetCoreRead Error. typesocket=%d status=%d socket=%llx. OPType=%d OPocket=%llx", \
					pkNetSocket->iTypeSocket, pkNetSocket->usStatus, pkNetSocket->kHSocket, pkIOData->iOPType, pkIOData->kHScoket);
			}
		}
		//else //{ SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[EPOLL WAIT] socket not found"); }
		
		SeDelIOData(pkNetCore, pkIOData);
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
