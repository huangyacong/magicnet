#include "SeNetCore.h"

#if (defined(_WIN32) || defined(WIN32))

#define OP_TYPE_SEND 1
#define OP_TYPE_RECV 2
#define OP_TYPE_ACCEPT 3
#define OP_TYPE_CONNECT 4

struct IODATA
{
	OVERLAPPED	overlapped;
	HSOCKET		kHScoket;
	SOCKET		kSocket;
	WSABUF		kBuf;
	int			iOPType;
	char		acData[1024*4];
};

void SeNetCoreInit(struct SENETCORE *pkNetCore, char *pcLogName, int iTimeOut, unsigned short usMax)
{
	SeNetBaseInit();
	pkNetCore->kHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	SeInitLog(&pkNetCore->kLog, pcLogName);
	SeNetSocketMgrInit(&pkNetCore->kSocketMgr, iTimeOut, usMax);
	SeAddLogLV(&pkNetCore->kLog, LT_PRINT);
	SeAddLogLV(&pkNetCore->kLog, LT_SOCKET);
}

void SeNetCoreFin(struct SENETCORE *pkNetCore)
{
	SeCloseHandle(pkNetCore->kHandle);
	SeFinLog(&pkNetCore->kLog);
	SeNetSocketMgrFin(&pkNetCore->kSocketMgr);
	SeNetBaseEnd();
}

void SeNetCoreAcceptEx(struct SENETCORE *pkNetCore, HSOCKET kListenHSocket, int iNum)
{
	int i;
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
		return;
	}

	for(i = 0; i < iNum; i++)
	{
		socket = SeSocket(SOCK_STREAM);
		if(socket == SE_INVALID_SOCKET)
		{
			iErrorno = SeErrno();
			SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[GET ACCEPT POINT] new client socket failed, errno=%d", iErrorno);
			continue;
		}

		pkIOData = (struct IODATA*)GlobalAlloc(GPTR, sizeof(struct IODATA));
		if(!pkIOData)
		{
			iErrorno = SeErrno();
			SeCloseSocket(socket);
			SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[GET ACCEPT POINT] new mem failed, errno=%d", iErrorno);
			continue;
		}

		memset(&pkIOData->overlapped, 0, sizeof(OVERLAPPED));
		pkIOData->kHScoket = kListenHSocket;
		pkIOData->kSocket = socket;
		pkIOData->iOPType = OP_TYPE_ACCEPT;

		bRet = lpfnAcceptEx(SeGetSocketByHScoket(kListenHSocket), socket,
				pkIOData->acData, 0,
				sizeof(struct sockaddr_in) + 16, sizeof(struct sockaddr_in) + 16,
				&dwBytes, &pkIOData->overlapped);
		if(bRet) { continue; }
	
		iErrorno = SeErrno();
		if(iErrorno != ERROR_IO_PENDING)
		{
			SeCloseSocket(socket);
			GlobalFree(pkIOData);
			SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[GET ACCEPT POINT] add socket failed, errno=%d", iErrorno);
			continue;
		}
	}
}

HSOCKET SeNetCoreTCPListen(struct SENETCORE *pkNetCore, const char *pcIP, unsigned short usPort,\
				int iHeaderLen, SEGETHEADERLENFUN pkGetHeaderLenFun, SESETHEADERLENFUN pkSetHeaderLenFun)
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
	
	pkNetSocket = SeNetSocketMgrGet(&pkNetCore->kSocketMgr, kHSocket);
	pkNetSocket->usStatus = SOCKET_STATUS_ACTIVECONNECT;
	SeNetCoreAcceptEx(pkNetCore, kHSocket, backlog);
	
	return kHSocket;
}

HSOCKET SeNetCoreTCPClient(struct SENETCORE *pkNetCore, const char *pcIP, unsigned short usPort,\
				int iHeaderLen, SEGETHEADERLENFUN pkGetHeaderLenFun, SESETHEADERLENFUN pkSetHeaderLenFun)
{
	int iErrorno;
	SOCKET socket;
	HSOCKET kHSocket;
	DWORD dwSend,dwBytes;
	struct sockaddr kAddr;
	struct sockaddr local;
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

	pkIOData = (struct IODATA*)GlobalAlloc(GPTR, sizeof(struct IODATA));
	if(!pkIOData)
	{
		iErrorno = SeErrno();
		SeCloseSocket(socket);
		SeNetSocketMgrDel(&pkNetCore->kSocketMgr, kHSocket);
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP CLIENT] new mem failed, errno=%d", iErrorno);
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
			GlobalFree(pkIOData);
			SeCloseSocket(socket);
			SeNetSocketMgrDel(&pkNetCore->kSocketMgr, kHSocket);
			SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TCP CLIENT] ConnectEx failed, errno=%d", iErrorno);
			return 0;
		}
	}
	
	pkNetSocket = SeNetSocketMgrGet(&pkNetCore->kSocketMgr, kHSocket);
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
	SeCloseSocket(socket);
	SeNetSocketMgrAddSendOrRecvInList(&pkNetCore->kSocketMgr, pkNetSocket, true);
}

bool SeNetCoreSendBuf(struct SENETCORE *pkNetCore, struct SESOCKET *pkNetSocket)
{
	DWORD dwLen;
	int iErrorno;
	SOCKET socket;
	struct IODATA *pkIOData;
	struct SENETSTREAMNODE *pkNetStreamNode;
	
	socket = SeGetSocketByHScoket(pkNetSocket->kHSocket);
	if(SeNetSocketMgrHasEvent(pkNetSocket, WRITE_EVENT_SOCKET)) { return true; }
	
	pkNetStreamNode = SeNetSreamHeadPop(&pkNetSocket->kSendNetStream);
	if(!pkNetStreamNode) { return true; }

	pkIOData = (struct IODATA*)GlobalAlloc(GPTR, sizeof(struct IODATA));
	if(!pkIOData)
	{
		iErrorno = SeErrno();
		SeNetSreamHeadAdd(&pkNetSocket->kSendNetStream, pkNetStreamNode);
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[SEND DATA] new mem failed, errno=%d", iErrorno);
		return false;
	}

	memset(&pkIOData->overlapped, 0, sizeof(OVERLAPPED));
	pkIOData->kHScoket = pkNetSocket->kHSocket;
	pkIOData->iOPType = OP_TYPE_SEND;
	pkIOData->kBuf.buf = pkIOData->acData;
	pkIOData->kBuf.len = pkNetStreamNode->iWritePos - pkNetStreamNode->iReadPos;
	assert(pkNetStreamNode->iWritePos - pkNetStreamNode->iReadPos > 0);
	assert(pkNetStreamNode->iMaxLen <= sizeof(pkIOData->acData));
	memcpy(pkIOData->kBuf.buf, pkNetStreamNode->pkBuf, pkNetStreamNode->iWritePos - pkNetStreamNode->iReadPos);

	dwLen = 0;
	if(WSASend(socket, &pkIOData->kBuf, 1, &dwLen, 0, &pkIOData->overlapped, 0) == SE_SOCKET_ERROR)
	{
		iErrorno = SeErrno();
		if(iErrorno != WSA_IO_PENDING)
		{
			GlobalFree(pkIOData);
			SeNetSreamHeadAdd(&pkNetSocket->kSendNetStream, pkNetStreamNode);
			return false;
		}
	}
	
	SeNetSocketMgrAddEvent(pkNetSocket, WRITE_EVENT_SOCKET);
	SeNetSreamHeadAdd(&pkNetCore->kSocketMgr.kNetStreamIdle, pkNetStreamNode);

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

	pkIOData = (struct IODATA*)GlobalAlloc(GPTR, sizeof(struct IODATA));
	if(!pkIOData)
	{
		iErrorno = SeErrno();
		SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[RECV DATA] new mem failed, errno=%d", iErrorno);
		return false;
	}

	memset(&pkIOData->overlapped, 0, sizeof(OVERLAPPED));
	pkIOData->kHScoket = pkNetSocket->kHSocket;
	pkIOData->iOPType = OP_TYPE_RECV;
	pkIOData->kBuf.buf = pkIOData->acData;
	pkIOData->kBuf.len = sizeof(pkIOData->acData);
	memcpy(pkIOData->kBuf.buf, pkIOData->kBuf.buf, pkIOData->kBuf.len);

	iFlags = 0;
	dwLen = 0;
	if(WSARecv(socket, &pkIOData->kBuf, 1, &dwLen, (LPDWORD)&iFlags, &pkIOData->overlapped, 0) == SE_SOCKET_ERROR)
	{
		iErrorno = SeErrno();
		if(iErrorno != WSA_IO_PENDING)
		{
			GlobalFree(pkIOData);
			return false;
		}
	}
	
	SeNetSocketMgrAddEvent(pkNetSocket, READ_EVENT_SOCKET);

	return true;
}

void SeNetCoreListenSocket(struct SENETCORE *pkNetCore, struct SESOCKET *pkNetSocketListen, SOCKET kSocket, struct IODATA *pkIOData)
{
	int							iErrorno;
	HSOCKET						kHSocket;
	struct SESOCKET				*pkNetSocketAccept;

	LPFN_GETACCEPTEXSOCKADDRS	lpfnGetAcceptExSockaddrs = NULL;
	GUID						tGuidGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;
	DWORD						dwBytes = 0;
	int							tResult = 0;

	struct	sockaddr_in			*local_addr = NULL;
	struct  sockaddr_in			*remote_addr = NULL;
	int							local_addr_len = sizeof(struct sockaddr_in);
	int							remote_addr_len = sizeof(struct sockaddr_in);

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
	pkNetSocketAccept->kBelongListenHSocket = pkNetSocketListen->kHSocket;
	SeNetSocketMgrAddSendOrRecvInList(&pkNetCore->kSocketMgr, pkNetSocketAccept, true);
	SeNetCoreAcceptEx(pkNetCore, pkNetSocketListen->kHSocket, 1);
}

void SeNetCoreAcceptSocket(struct SENETCORE *pkNetCore, struct SESOCKET *pkNetSocket, const struct IODATA *pkIOData, DWORD dwLen)
{
	bool bOK;
	int iErrorno;
	SOCKET socket;
	DWORD dwSendLen;
	struct IODATA *pkSendIOData;

	socket = SeGetSocketByHScoket(pkNetSocket->kHSocket);

	if(dwLen <= 0) { SeNetCoreDisconnect(pkNetCore, pkNetSocket->kHSocket); return; }
	
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
			pkSendIOData = (struct IODATA*)GlobalAlloc(GPTR, sizeof(struct IODATA));
			if(!pkSendIOData)
			{
				iErrorno = SeErrno();
				SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[SEND DATA] new mem failed, errno=%d", iErrorno);
				SeNetCoreDisconnect(pkNetCore, pkNetSocket->kHSocket);
				return;
			}

			memset(&pkSendIOData->overlapped, 0, sizeof(OVERLAPPED));
			pkSendIOData->kHScoket = pkNetSocket->kHSocket;
			pkSendIOData->iOPType = OP_TYPE_SEND;
			pkSendIOData->kBuf.buf = pkSendIOData->acData;
			pkSendIOData->kBuf.len = pkIOData->kBuf.len - dwLen;
			memcpy(pkSendIOData->kBuf.buf, pkIOData->kBuf.buf + dwLen, pkIOData->kBuf.len - dwLen);

			dwSendLen = 0;
			if(WSASend(socket, &pkSendIOData->kBuf, 1, &dwSendLen, 0, &pkSendIOData->overlapped, 0) == SE_SOCKET_ERROR)
			{
				iErrorno = SeErrno();
				if(iErrorno != WSA_IO_PENDING)
				{
					GlobalFree(pkSendIOData);
					SeNetCoreDisconnect(pkNetCore, pkNetSocket->kHSocket);
					return;
				}
			}
		}
	}

	if(pkIOData->iOPType == OP_TYPE_RECV)
	{
		SeNetSocketMgrActive(&pkNetCore->kSocketMgr, pkNetSocket);
		SeNetSocketMgrUpdateNetStreamIdle(&pkNetCore->kSocketMgr, pkNetSocket->iHeaderLen, dwLen);
		bOK = SeNetSreamWrite(&pkNetSocket->kRecvNetStream, &pkNetCore->kSocketMgr.kNetStreamIdle, pkNetSocket->pkSetHeaderLenFun, 0, pkIOData->kBuf.buf, dwLen);
		if(!bOK) { SeNetCoreDisconnect(pkNetCore, pkNetSocket->kHSocket); SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[CORE RECV] recv data ERROR"); return; }
		SeNetSocketMgrClearEvent(pkNetSocket, READ_EVENT_SOCKET);
		bOK = SeNetCoreRecvBuf(pkNetCore, pkNetSocket);
		if(!bOK) { SeNetCoreDisconnect(pkNetCore, pkNetSocket->kHSocket); return; }
		SeNetSocketMgrAddSendOrRecvInList(&pkNetCore->kSocketMgr, pkNetSocket, false);
	}
}

void SeNetCoreClientSocket(struct SENETCORE *pkNetCore, struct SESOCKET *pkNetSocket, const struct IODATA *pkIOData, DWORD dwLen, BOOL bConnectFailed)
{
	if(pkIOData->iOPType == OP_TYPE_CONNECT)
	{
		assert(pkNetSocket->usStatus == SOCKET_STATUS_CONNECTING);
		pkNetSocket->usStatus = bConnectFailed ? SOCKET_STATUS_CONNECTED : SOCKET_STATUS_CONNECTED_FAILED;
		SeNetSocketMgrAddSendOrRecvInList(&pkNetCore->kSocketMgr, pkNetSocket, true);
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

	pkConstNetSocket = SeNetSocketMgrTimeOut(&pkNetCore->kSocketMgr);
	if(pkConstNetSocket)
	{
		if(pkConstNetSocket->usStatus == SOCKET_STATUS_CONNECTING || pkConstNetSocket->usStatus == SOCKET_STATUS_ACTIVECONNECT)
		{
			SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[TIME OUT] Socket time out");
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
			if(pcAddrIP) { strcpy(pcBuf, pcAddrIP); *riLen = (int)strlen(pcAddrIP); }
			*riEventSocket = SENETCORE_EVENT_SOCKET_CONNECT;
			pkNetSocket->usStatus = SOCKET_STATUS_ACTIVECONNECT;
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
		bOK = SeNetSreamRead(&pkNetSocket->kRecvNetStream, &pkNetCore->kSocketMgr.kNetStreamIdle, pkNetSocket->pkGetHeaderLenFun, pkNetSocket->iHeaderLen, pcBuf, riLen);
		if(!bOK) { continue; }
		
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

	bResult = GetQueuedCompletionStatus(pkNetCore->kHandle, &dwLen, &ulKey, &pkOverlapped, 0);
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
		}
		else { SeLogWrite(&pkNetCore->kLog, LT_SOCKET, true, "[EPOLL WAIT] socket not found"); }
		
		GlobalFree(pkIOData);
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
