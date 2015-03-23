#include "SeNetTcp.h"

#if defined(__linux)

void SeNetTcpInit(struct SENETTCP *pkNetTcp, char *pcLogName, SENETTCPONCONNECT pkOnConnectFunc, SENETTCPDISCONNECT pkOnDisconnectFunc, SENETTCPRECV pkOnRecvDataFunc)
{
	SeNetTcpCreate(pkNetTcp, pcLogName);
	pkNetTcp->pkOnConnectFunc = pkOnConnectFunc;
	pkNetTcp->pkOnDisconnectFunc = pkOnDisconnectFunc;
	pkNetTcp->pkOnRecvDataFunc = pkOnRecvDataFunc;
	pkNetTcp->kHandle = epoll_create(MAX_SOCKET_LEN);
}

void SeNetTcpFin(struct SENETTCP *pkNetTcp)
{
	SeNetTcpFree(pkNetTcp);
}

SOCKET SeNetTcpCreateSvrSocket(struct SENETTCP *pkNetTcp, const char *pcIP, unsigned short usPort, int iMemSize, int iProtoFormat)
{
	SOCKET kSocket;
	struct epoll_event kEvent;

	kSocket = SeNetTcpAddSvr(pkNetTcp, pcIP, usPort, iMemSize, iProtoFormat);

	kEvent.events = EPOLLIN | EPOLLET;
	if(epoll_ctl(pkNetTcp->kHandle, EPOLL_CTL_ADD, kSocket, &kEvent) != 0) {
		SeLogWrite(&pkNetTcp->kLog, LT_CRITICAL, true, "Init bind handle feaild, addr=%s, port=%d\n", pcIP, (int)usPort);
		return SE_INVALID_SOCKET;
	}

	return kSocket;
}

#endif
