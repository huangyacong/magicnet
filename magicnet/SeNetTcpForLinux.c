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

#endif
