#include "SeNetSocket.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "SeTool.h"

void SeNetSocketReset(struct SESOCKET *pkNetSocket)
{
	pkNetSocket->kHSocket = SeGetHSocket(0, 0, 0);
	pkNetSocket->usStatus = SOCKET_STATUS_INIT;
	pkNetSocket->iHeaderLen = 0;
	pkNetSocket->iTypeSocket = 0;
}

void SeNetSocketInit(struct SESOCKET *pkNetSocket, unsigned short usIndex)
{
	pkNetSocket->usIndex = usIndex;
	SeNetSocketReset(pkNetSocket);
	SeNetSreamInit(&pkNetSocket->kSendNetStream);
	SeNetSreamInit(&pkNetSocket->kRecvNetStream);
	SeListInitNode(&pkNetSocket->kMainNode);
	SeHashNodeInit(&pkNetSocket->kSendNode);
	SeHashNodeInit(&pkNetSocket->kRecvNode);
}

void SeNetSocketMgrInit(struct SESOCKETMGR *pkNetSocketMgr, unsigned short usMax)
{
	int i;

	assert(usMax > 0);
	pkNetSocketMgr->iMax = usMax;
	SeNetSreamInit(&pkNetSocketMgr->kNetStreamIdle);
	pkNetSocketMgr->pkSeSocket = (struct SESOCKET *)malloc(sizeof(struct SESOCKET)*pkNetSocketMgr->iMax);
	SeListInit(&pkNetSocketMgr->kMainList);
	SeHashInit(&pkNetSocketMgr->kSendList, pkNetSocketMgr->iMax);
	SeHashInit(&pkNetSocketMgr->kRecvList, pkNetSocketMgr->iMax);

	for(i = 0; i < pkNetSocketMgr->iMax; i++)
	{
		SeNetSocketInit(&pkNetSocketMgr->pkSeSocket[i], (unsigned short)i);
		SeNetSocketReset(&pkNetSocketMgr->pkSeSocket[i]);
		SeListHeadAdd(&pkNetSocketMgr->kMainList, &((pkNetSocketMgr->pkSeSocket[i]).kMainNode));
	}
}

void SeNetSocketMgrFin(struct SESOCKETMGR *pkNetSocketMgr)
{
	int i;
	SOCKET kSocket;
	struct SENETSTREAMNODE *pkNetStreamNode;

	for(i = 0; i < pkNetSocketMgr->iMax; i++)
	{
		kSocket = SeGetSocketByHScoket(pkNetSocketMgr->pkSeSocket[i].kHSocket);
		if(kSocket > 0) SeCloseSocket(kSocket);

		pkNetStreamNode = SeNetSreamHeadPop(&(pkNetSocketMgr->pkSeSocket[i].kSendNetStream));
		while(pkNetStreamNode)
		{
			SeNetSreamNodeZero(pkNetStreamNode);
			SeNetSreamHeadAdd(&pkNetSocketMgr->kNetStreamIdle, pkNetStreamNode);
			pkNetStreamNode = SeNetSreamHeadPop(&(pkNetSocketMgr->pkSeSocket[i].kSendNetStream));
		}

		pkNetStreamNode = SeNetSreamHeadPop(&(pkNetSocketMgr->pkSeSocket[i].kRecvNetStream));
		while(pkNetStreamNode)
		{
			SeNetSreamNodeZero(pkNetStreamNode);
			SeNetSreamHeadAdd(&pkNetSocketMgr->kNetStreamIdle, pkNetStreamNode);
			pkNetStreamNode = SeNetSreamHeadPop(&(pkNetSocketMgr->pkSeSocket[i].kRecvNetStream));
		}
	}

	pkNetStreamNode = SeNetSreamHeadPop(&pkNetSocketMgr->kNetStreamIdle);
	while(pkNetStreamNode) {free(pkNetStreamNode);pkNetStreamNode = SeNetSreamHeadPop(&pkNetSocketMgr->kNetStreamIdle);}

	free(pkNetSocketMgr->pkSeSocket);
	SeHashFin(&pkNetSocketMgr->kSendList);
	SeHashFin(&pkNetSocketMgr->kRecvList);
}