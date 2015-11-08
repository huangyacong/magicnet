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
	pkNetSocket->pkGetHeaderLenFun = 0;
	pkNetSocket->pkSetHeaderLenFun = 0;
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
	pkNetSocketMgr->iCounter = 0;
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

	for(i = 0; i < pkNetSocketMgr->iMax; i++) SeNetSocketMgrDel(pkNetSocketMgr, &pkNetSocketMgr->pkSeSocket[i]);
	pkNetStreamNode = SeNetSreamHeadPop(&pkNetSocketMgr->kNetStreamIdle);
	while(pkNetStreamNode) {free(pkNetStreamNode);pkNetStreamNode = SeNetSreamHeadPop(&pkNetSocketMgr->kNetStreamIdle);}

	free(pkNetSocketMgr->pkSeSocket);
	SeHashFin(&pkNetSocketMgr->kSendList);
	SeHashFin(&pkNetSocketMgr->kRecvList);
}

struct SESOCKET *SeNetSocketMgrTCPAdd(struct SESOCKETMGR *pkNetSocketMgr, SOCKET socket, int iTypeSocket, int iHeaderLen, \
			SEGETHEADERLENFUN pkGetHeaderLenFun, SESETHEADERLENFUN pkSetHeaderLenFun)
{
	struct SENODE *pkNode;
	struct SESOCKET *pkNetSocket;

	pkNode = SeListHeadPop(&pkNetSocketMgr->kMainList);
	if(!pkNode) return 0;
	pkNetSocket = SE_CONTAINING_RECORD(pkNode, struct SESOCKET, kMainNode);
	SeNetSocketReset(pkNetSocket);
	pkNetSocketMgr->iCounter++;

	pkNetSocket->kHSocket = SeGetHSocket(pkNetSocketMgr->iCounter, pkNetSocket->usIndex, socket);
	pkNetSocket->usStatus = SOCKET_STATUS_INIT;
	pkNetSocket->iHeaderLen = iHeaderLen;
	pkNetSocket->iTypeSocket = iTypeSocket;
	pkNetSocket->pkGetHeaderLenFun = pkGetHeaderLenFun;
	pkNetSocket->pkSetHeaderLenFun = pkSetHeaderLenFun;

	return pkNetSocket;
}

void SeNetSocketMgrDel(struct SESOCKETMGR *pkNetSocketMgr, struct SESOCKET *pkNetSocket)
{
	struct SEHASHNODE *pkNetSocketTmp;
	struct SENETSTREAMNODE *pkNetStreamNode;

	pkNetStreamNode = SeNetSreamHeadPop(&pkNetSocket->kSendNetStream);
	while(pkNetStreamNode)
	{
		SeNetSreamNodeZero(pkNetStreamNode);
		SeNetSreamHeadAdd(&pkNetSocketMgr->kNetStreamIdle, pkNetStreamNode);
		pkNetStreamNode = SeNetSreamHeadPop(&pkNetSocket->kSendNetStream);
	}

	pkNetStreamNode = SeNetSreamHeadPop(&pkNetSocket->kRecvNetStream);
	while(pkNetStreamNode)
	{
		SeNetSreamNodeZero(pkNetStreamNode);
		SeNetSreamHeadAdd(&pkNetSocketMgr->kNetStreamIdle, pkNetStreamNode);
		pkNetStreamNode = SeNetSreamHeadPop(&pkNetSocket->kRecvNetStream);
	}
	
	SeNetSocketReset(pkNetSocket);
	SeListTailAdd(&pkNetSocketMgr->kMainList, &pkNetSocket->kMainNode);
	pkNetSocketTmp = SeHashGet(&pkNetSocketMgr->kSendList, pkNetSocket->usIndex);
	if(pkNetSocketTmp) {assert(pkNetSocket == pkNetSocketTmp); SeHashRemove(&pkNetSocketMgr->kSendList, &pkNetSocketTmp->kSendNode);}
	pkNetSocketTmp = SeHashGet(&pkNetSocketMgr->kRecvList, pkNetSocket->usIndex);
	if(pkNetSocketTmp) {assert(pkNetSocket == pkNetSocketTmp);SeHashRemove(&pkNetSocketMgr->kRecvList, &pkNetSocketTmp->kRecvNode);}
}
