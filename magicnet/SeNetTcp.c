#include "SeNetTcp.h"

void SeNetTcpCreate(struct SENETTCP *pkNetTcp, char *pcLogName)
{
	int iBegin;

	pkNetTcp->kHandle = SE_INVALID_HANDLE;
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
	struct SESSOCKETNODE *pkNetSSocketNode;
	struct SECSOCKETNODE *pkNetCSocketNode;
	struct SENETSTREAMNODE *pkNetStreamNode;

	SeFinLog(&pkNetTcp->kLog);

	pkNetSSocketNode = SeNetSSocketPop(&pkNetTcp->kSvrSocketList);
	while(pkNetSSocketNode) {
		pkNetStreamNode = SeNetSreamHeadPop(&pkNetSSocketNode->kMemSCache);
		while(pkNetStreamNode) {
			SeFreeMem((void*)pkNetStreamNode);
			pkNetStreamNode = SeNetSreamHeadPop(&pkNetSSocketNode->kMemSCache);
		}
		SeShutDown(pkNetSSocketNode->kListenSocket);
		SeCloseSocket(pkNetSSocketNode->kListenSocket);
		SeFreeMem((void*)pkNetSSocketNode);
		pkNetSSocketNode = SeNetSSocketPop(&pkNetTcp->kSvrSocketList);
	}

	pkNetCSocketNode = SeNetCSocketPop(&pkNetTcp->kActiveCSocketList);
	while(pkNetCSocketNode) {
		pkNetStreamNode = SeNetSreamHeadPop(&pkNetCSocketNode->kSendNetStream);
		while(pkNetStreamNode) {
			SeFreeMem((void*)pkNetStreamNode);
			pkNetStreamNode = SeNetSreamHeadPop(&pkNetCSocketNode->kSendNetStream);
		}
		pkNetStreamNode = SeNetSreamHeadPop(&pkNetCSocketNode->kRecvNetStream);
		while(pkNetStreamNode) {
			SeFreeMem((void*)pkNetStreamNode);
			pkNetStreamNode = SeNetSreamHeadPop(&pkNetCSocketNode->kRecvNetStream);
		}
		SeCloseSocket(SeGetSocketByHScoket(pkNetCSocketNode->kHSocket));
		pkNetCSocketNode = SeNetCSocketPop(&pkNetTcp->kActiveCSocketList);
	}

	pkNetCSocketNode = SeNetCSocketPop(&pkNetTcp->kConnectCSocketList);
	while(pkNetCSocketNode) {
		pkNetStreamNode = SeNetSreamHeadPop(&pkNetCSocketNode->kSendNetStream);
		while(pkNetStreamNode) {
			SeFreeMem((void*)pkNetStreamNode);
			pkNetStreamNode = SeNetSreamHeadPop(&pkNetCSocketNode->kSendNetStream);
		}
		pkNetStreamNode = SeNetSreamHeadPop(&pkNetCSocketNode->kRecvNetStream);
		while(pkNetStreamNode) {
			SeFreeMem((void*)pkNetStreamNode);
			pkNetStreamNode = SeNetSreamHeadPop(&pkNetCSocketNode->kRecvNetStream);
		}
		SeCloseSocket(SeGetSocketByHScoket(pkNetCSocketNode->kHSocket));
		pkNetCSocketNode = SeNetCSocketPop(&pkNetTcp->kConnectCSocketList);
	}

	pkNetCSocketNode = SeNetCSocketPop(&pkNetTcp->kDisConnectCSocketList);
	while(pkNetCSocketNode) {
		pkNetStreamNode = SeNetSreamHeadPop(&pkNetCSocketNode->kSendNetStream);
		while(pkNetStreamNode) {
			SeFreeMem((void*)pkNetStreamNode);
			pkNetStreamNode = SeNetSreamHeadPop(&pkNetCSocketNode->kSendNetStream);
		}
		pkNetStreamNode = SeNetSreamHeadPop(&pkNetCSocketNode->kRecvNetStream);
		while(pkNetStreamNode) {
			SeFreeMem((void*)pkNetStreamNode);
			pkNetStreamNode = SeNetSreamHeadPop(&pkNetCSocketNode->kRecvNetStream);
		}
		SeCloseSocket(SeGetSocketByHScoket(pkNetCSocketNode->kHSocket));
		pkNetCSocketNode = SeNetCSocketPop(&pkNetTcp->kDisConnectCSocketList);
	}

	SeCloseHandle(pkNetTcp->kHandle);
	pkNetTcp->kHandle = SE_INVALID_HANDLE;

	pkNetTcp->pkOnConnectFunc = 0;
	pkNetTcp->pkOnDisconnectFunc = 0;
	pkNetTcp->pkOnRecvDataFunc = 0;
}
