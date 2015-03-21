#include "SeNetTcp.h"

void SeNetTcpCreate(struct SENETTCP *pkNetTcp)
{
	int iBegin;

	pkNetTcp->kHandle = SE_INVALID_HANDLE;
	SeNetSreamInit(&pkNetTcp->kMemCache);
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
}
