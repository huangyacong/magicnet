#include "SeNetCore.h"

#if (defined(_WIN32) || defined(WIN32))

#define OP_TYPE_ACCEPT 0
#define OP_TYPE_SEND 1
#define OP_TYPE_RECV 2

struct IODATA{
	OVERLAPPED overlapped;
	HSOCKET kHScoket;
	SOCKET kSocket;
	WSABUF kBuf;
	int iLen;
	int iOPType;
	char acData[1024*4];
};

void SeNetCoreInit(struct SENETCORE *pkNetCore, char *pcLogName, unsigned short usMax)
{
	SeNetBaseInit();
	pkNetCore->kHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
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

void SeNetCoreAcceptEx(struct SENETCORE *pkNetCore, HSOCKET kListenHSocket, int iNum)
{
	int i;
	BOOL bRet;
	int iErrorno;
	DWORD dwBytes;
	SOCKET socket;
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	struct IODATA *pkIOData;
	LPFN_ACCEPTEX lpfnAcceptEx;
	
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
		if(socket == SE_INVALID_SOCKET) { break; }
		pkIOData = (struct IODATA*)GlobalAlloc(GPTR, sizeof(struct IODATA));
		if(!pkIOData) { SeCloseSocket(socket); break; }

		memset(pkIOData, 0, sizeof(struct IODATA));
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

	SeNetCoreAcceptEx(pkNetCore, kHSocket, backlog);
	
	return kHSocket;
}

#endif
