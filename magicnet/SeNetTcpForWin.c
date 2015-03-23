#include "SeNetTcp.h"

#if (defined(_WIN32) || defined(WIN32))

void SeNetTcpInit(struct SENETTCP *pkNetTcp, SENETTCPONCONNECT pkOnConnectFunc, SENETTCPDISCONNECT pkOnDisconnectFunc, SENETTCPRECV pkOnRecvDataFunc)
{
	SeNetTcpCreate(pkNetTcp);
	pkNetTcp->pkOnConnectFunc = pkOnConnectFunc;
	pkNetTcp->pkOnDisconnectFunc = pkOnDisconnectFunc;
	pkNetTcp->pkOnRecvDataFunc = pkOnRecvDataFunc;
	pkNetTcp->kHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
}

void SeNetTcpFin(struct SENETTCP *pkNetTcp)
{
	SeNetTcpFree(pkNetTcp);
}

#endif
