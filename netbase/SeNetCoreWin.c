#include "SeNetCore.h"

#if (defined(_WIN32) || defined(WIN32))

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

HSOCKET SeNetCoreTCPListen(struct SENETCORE *pkNetCore, const char *pcIP, unsigned short usPort,\
				int iHeaderLen, SEGETHEADERLENFUN pkGetHeaderLenFun, SESETHEADERLENFUN pkSetHeaderLenFun)
{
	int backlog;
	int iErrorno;
	SOCKET socket;
	HSOCKET kHSocket;
	struct sockaddr kAddr;
	
	SeSetSockAddr(&kAddr, pcIP, usPort);
	socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL ,0 , WSA_FLAG_OVERLAPPED);
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

	return kHSocket;
}

#endif
