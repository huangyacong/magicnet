#include "SeNetTcp.h"

void SeNetTcpCreate(struct SENETTCP *pkNetTcp, char *pcLogName)
{
	int iBegin;

	pkNetTcp->kHandle = SE_INVALID_HANDLE;
	SeNetSreamInit(&pkNetTcp->kMemCCache);
	SeNetSSocketInit(&pkNetTcp->kSvrSocketList);
	
	SeNetCSocketInit(&pkNetTcp->kFreeCSocketList);
	SeNetCSocketInit(&pkNetTcp->kActiveCSocketList);
	SeNetCSocketInit(&pkNetTcp->kConnectCSocketList);
	SeNetCSocketInit(&pkNetTcp->kDisConnectCSocketList);

	for(iBegin = 0; iBegin < MAX_SOCKET_LEN; iBegin++) {
		SeNetCSocketNodeInit(&pkNetTcp->kClientSocket[iBegin]);
		pkNetTcp->kClientSocket[iBegin].iFlag = iBegin;
		SeNetCSocketHeadAdd(&pkNetTcp->kFreeCSocketList, &pkNetTcp->kClientSocket[iBegin]);
	}

	pkNetTcp->pkOnConnectFunc = 0;
	pkNetTcp->pkOnDisconnectFunc = 0;
	pkNetTcp->pkOnRecvDataFunc = 0;

	SeInitLog(&pkNetTcp->kLog, pcLogName);
}

void SeNetTcpFree(struct SENETTCP *pkNetTcp)
{
	struct SENETSTREAM kMemCache;
	struct SESSOCKETNODE *pkNetSSocketNode;
	struct SECSOCKETNODE *pkNetCSocketNode;
	struct SENETSTREAMNODE *pkNetStreamNode;
	
	SeNetSreamInit(&kMemCache);
	SeFinLog(&pkNetTcp->kLog);
	
	pkNetCSocketNode = SeNetCSocketPop(&pkNetTcp->kFreeCSocketList);
	while(pkNetCSocketNode) {
		SeNetCSocketNodeFin(pkNetCSocketNode, &kMemCache);
		pkNetCSocketNode = SeNetCSocketPop(&pkNetTcp->kFreeCSocketList);
	}

	pkNetCSocketNode = SeNetCSocketPop(&pkNetTcp->kActiveCSocketList);
	while(pkNetCSocketNode) {
		SeCloseSocket(SeGetSocketByHScoket(pkNetCSocketNode->kHSocket));
		SeNetCSocketNodeFin(pkNetCSocketNode, &kMemCache);
		pkNetCSocketNode = SeNetCSocketPop(&pkNetTcp->kActiveCSocketList);
	}

	pkNetCSocketNode = SeNetCSocketPop(&pkNetTcp->kConnectCSocketList);
	while(pkNetCSocketNode) {
		SeCloseSocket(SeGetSocketByHScoket(pkNetCSocketNode->kHSocket));
		SeNetCSocketNodeFin(pkNetCSocketNode, &kMemCache);
		pkNetCSocketNode = SeNetCSocketPop(&pkNetTcp->kConnectCSocketList);
	}

	pkNetCSocketNode = SeNetCSocketPop(&pkNetTcp->kDisConnectCSocketList);
	while(pkNetCSocketNode) {
		SeCloseSocket(SeGetSocketByHScoket(pkNetCSocketNode->kHSocket));
		SeNetCSocketNodeFin(pkNetCSocketNode, &kMemCache);
		pkNetCSocketNode = SeNetCSocketPop(&pkNetTcp->kDisConnectCSocketList);
	}

	pkNetSSocketNode = SeNetSSocketPop(&pkNetTcp->kSvrSocketList);
	while(pkNetSSocketNode) {
		SeShutDown(pkNetSSocketNode->kListenSocket);
		SeCloseSocket(pkNetSSocketNode->kListenSocket);
		SeNetSSocketNodeFin(pkNetSSocketNode, &kMemCache);
		SeFreeMem((void*)pkNetSSocketNode);
		pkNetSSocketNode = SeNetSSocketPop(&pkNetTcp->kSvrSocketList);
	}

	pkNetStreamNode = SeNetSreamHeadPop(&kMemCache);
	while(pkNetStreamNode) {
		SeFreeMem((void*)pkNetStreamNode);
		pkNetStreamNode = SeNetSreamHeadPop(&kMemCache);
	}

	pkNetStreamNode = SeNetSreamHeadPop(&pkNetTcp->kMemCCache);
	while(pkNetStreamNode) {
		SeFreeMem((void*)pkNetStreamNode);
		pkNetStreamNode = SeNetSreamHeadPop(&pkNetTcp->kMemCCache);
	}

	SeCloseHandle(pkNetTcp->kHandle);
	pkNetTcp->kHandle = SE_INVALID_HANDLE;

	pkNetTcp->pkOnConnectFunc = 0;
	pkNetTcp->pkOnDisconnectFunc = 0;
	pkNetTcp->pkOnRecvDataFunc = 0;
}
