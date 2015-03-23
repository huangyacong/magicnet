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
	struct epoll_event kEvent;

	pkNetCSocketNode = SeNetTcpAddSvr(pkNetTcp, pcIP, usPort, iMemSize, iProtoFormat);

	kEvent.events = EPOLLIN | EPOLLET;
	kEvent.data.u64 = pkNetCSocketNode->kHSocket;
	if(epoll_ctl(pkNetTcp->kHandle, EPOLL_CTL_ADD, pkNetCSocketNode->kSvrSocket, &kEvent) != 0) {
		SeLogWrite(&pkNetTcp->kLog, LT_CRITICAL, true, "Init bind handle feaild, addr=%s, port=%d\n", pcIP, (int)usPort);
		return SE_INVALID_SOCKET;
	}

	return pkNetCSocketNode->kSvrSocket;
}

void SeNetTcpProcess(struct SENETTCP *pkNetTcp)
{
}

#endif
