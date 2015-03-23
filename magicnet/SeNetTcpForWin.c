#include "SeNetTcp.h"

#if (defined(_WIN32) || defined(WIN32))

void SeNetTcpInit(struct SENETTCP *pkNetTcp, char *pcLogName, SENETTCPONCONNECT pkOnConnectFunc, SENETTCPDISCONNECT pkOnDisconnectFunc, SENETTCPRECV pkOnRecvDataFunc)
{
	SeNetTcpCreate(pkNetTcp, pcLogName);
	pkNetTcp->pkOnConnectFunc = pkOnConnectFunc;
	pkNetTcp->pkOnDisconnectFunc = pkOnDisconnectFunc;
	pkNetTcp->pkOnRecvDataFunc = pkOnRecvDataFunc;
	pkNetTcp->kHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
}

void SeNetTcpFin(struct SENETTCP *pkNetTcp)
{
	struct SENETSTREAM kMemCache;
	struct SECSOCKETNODE *pkNetCSocketNode;
	struct SENETSTREAMNODE *pkNetStreamNode;
	
	SeNetSreamInit(&kMemCache);

	pkNetCSocketNode = SeNetCSocketPop(&pkNetTcp->kAcceptCSocketList);
	while(pkNetCSocketNode) {
		// È±ÉÙÊÍ·ÅÄÚ´æ

		SeShutDown(pkNetCSocketNode->kSvrSocket);
		SeCloseSocket(pkNetCSocketNode->kSvrSocket);
		SeNetCSocketNodeFin(pkNetCSocketNode, &kMemCache);
		pkNetCSocketNode = SeNetCSocketPop(&pkNetTcp->kAcceptCSocketList);
	}

	pkNetStreamNode = SeNetSreamHeadPop(&kMemCache);
	while(pkNetStreamNode) {
		SeFreeMem((void*)pkNetStreamNode);
		pkNetStreamNode = SeNetSreamHeadPop(&kMemCache);
	}

	SeNetTcpFree(pkNetTcp);
}

SOCKET SeNetTcpCreateSvrSocket(struct SENETTCP *pkNetTcp, const char *pcIP, unsigned short usPort, int iMemSize, int iProtoFormat)
{
	struct SECSOCKETNODE *pkNetCSocketNode;

	pkNetCSocketNode = SeNetTcpAddSvr(pkNetTcp, pcIP, usPort, iMemSize, iProtoFormat);
	return pkNetCSocketNode->kSvrSocket;
}

void SeNetTcpProcess(struct SENETTCP *pkNetTcp)
{
}

#endif
