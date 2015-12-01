#include "SeNetSocket.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "SeTool.h"

#define MAX_BUF_LEN 1024*4

void SeNetSocketReset(struct SESOCKET *pkNetSocket)
{
	pkNetSocket->kHSocket = SeGetHSocket(0, 0, 0);
	pkNetSocket->kBelongListenHSocket = SeGetHSocket(0, 0, 0);
	pkNetSocket->usStatus = SOCKET_STATUS_INIT;
	pkNetSocket->iHeaderLen = 0;
	pkNetSocket->iTypeSocket = 0;
	pkNetSocket->iEventSocket = 0;
	pkNetSocket->llFlag = -1;
	pkNetSocket->pkGetHeaderLenFun = 0;
	pkNetSocket->pkSetHeaderLenFun = 0;
}

void SeNetSocketInit(struct SESOCKET *pkNetSocket, unsigned short usIndex)
{
	pkNetSocket->usIndex = usIndex;
	SeNetSocketReset(pkNetSocket);
	SeNetSreamInit(&pkNetSocket->kSendNetStream);
	SeNetSreamInit(&pkNetSocket->kRecvNetStream);
	SeHashNodeInit(&pkNetSocket->kMainNode);
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
	SeHashInit(&pkNetSocketMgr->kMainList, pkNetSocketMgr->iMax);
	SeHashInit(&pkNetSocketMgr->kSendList, pkNetSocketMgr->iMax);
	SeHashInit(&pkNetSocketMgr->kRecvList, pkNetSocketMgr->iMax);

	for(i = 0; i < pkNetSocketMgr->iMax; i++)
	{
		SeNetSocketInit(&pkNetSocketMgr->pkSeSocket[i], (unsigned short)i);
		SeNetSocketReset(&pkNetSocketMgr->pkSeSocket[i]);
		SeHashAdd(&pkNetSocketMgr->kMainList, pkNetSocketMgr->pkSeSocket[i].usIndex, &((pkNetSocketMgr->pkSeSocket[i]).kMainNode));
	}
}

void SeNetSocketMgrEnd(struct SESOCKETMGR *pkNetSocketMgr, struct SESOCKET *pkNetSocket)
{
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
}

void SeNetSocketMgrFin(struct SESOCKETMGR *pkNetSocketMgr)
{
	int i;
	struct SENETSTREAMNODE *pkNetStreamNode;

	for(i = 0; i < pkNetSocketMgr->iMax; i++) SeNetSocketMgrEnd(pkNetSocketMgr, &pkNetSocketMgr->pkSeSocket[i]);
	pkNetStreamNode = SeNetSreamHeadPop(&pkNetSocketMgr->kNetStreamIdle);
	while(pkNetStreamNode) {free(pkNetStreamNode);pkNetStreamNode = SeNetSreamHeadPop(&pkNetSocketMgr->kNetStreamIdle);}

	free(pkNetSocketMgr->pkSeSocket);
	SeHashFin(&pkNetSocketMgr->kMainList);
	SeHashFin(&pkNetSocketMgr->kSendList);
	SeHashFin(&pkNetSocketMgr->kRecvList);
}

HSOCKET SeNetSocketMgrAdd(struct SESOCKETMGR *pkNetSocketMgr, SOCKET socket, int iTypeSocket, int iHeaderLen, \
			SEGETHEADERLENFUN pkGetHeaderLenFun, SESETHEADERLENFUN pkSetHeaderLenFun)
{
	struct SEHASHNODE *pkHashNode;
	struct SESOCKET *pkNetSocket;
	
	assert(socket > 0);
	pkHashNode = SeHashPop(&pkNetSocketMgr->kMainList);
	if(!pkHashNode) return 0;
	pkNetSocket = SE_CONTAINING_RECORD(pkHashNode, struct SESOCKET, kMainNode);
	SeNetSocketReset(pkNetSocket);
	pkNetSocketMgr->iCounter++;

	pkNetSocket->kHSocket = SeGetHSocket((unsigned short)pkNetSocketMgr->iCounter, pkNetSocket->usIndex, socket);
	pkNetSocket->usStatus = SOCKET_STATUS_INIT;
	pkNetSocket->iHeaderLen = iHeaderLen;
	pkNetSocket->iTypeSocket = iTypeSocket;
	pkNetSocket->pkGetHeaderLenFun = pkGetHeaderLenFun;
	pkNetSocket->pkSetHeaderLenFun = pkSetHeaderLenFun;

	return pkNetSocket->kHSocket;
}

struct SESOCKET *SeNetSocketMgrGet(struct SESOCKETMGR *pkNetSocketMgr, HSOCKET kHSocket)
{
	unsigned short usIndex;
	struct SESOCKET *pkNetSocket;
	struct SEHASHNODE *pkHashNode;
	
	usIndex = SeGetIndexByHScoket(kHSocket);
	assert(usIndex >= 0 && usIndex < pkNetSocketMgr->iMax);
	if(usIndex < 0 || usIndex >= pkNetSocketMgr->iMax) return 0;
	pkHashNode = SeHashGet(&pkNetSocketMgr->kMainList, usIndex);
	if(pkHashNode) return 0;
	pkNetSocket = &pkNetSocketMgr->pkSeSocket[usIndex];
	if(kHSocket != pkNetSocket->kHSocket) return 0;
	return pkNetSocket;
}

void SeNetSocketMgrDel(struct SESOCKETMGR *pkNetSocketMgr, HSOCKET kHSocket)
{
	struct SESOCKET *pkNetSocket;
	struct SEHASHNODE *pkHashNode;
	
	pkNetSocket = SeNetSocketMgrGet(pkNetSocketMgr, kHSocket);
	if(!pkNetSocket) return;
	SeNetSocketMgrEnd(pkNetSocketMgr, pkNetSocket);
	pkHashNode = SeHashGet(&pkNetSocketMgr->kSendList, pkNetSocket->usIndex);
	if(pkHashNode) { assert(&pkNetSocket->kSendNode == pkHashNode); SeHashRemove(&pkNetSocketMgr->kSendList, pkHashNode); }
	pkHashNode = SeHashGet(&pkNetSocketMgr->kRecvList, pkNetSocket->usIndex);
	if(pkHashNode) { assert(&pkNetSocket->kRecvNode == pkHashNode); SeHashRemove(&pkNetSocketMgr->kRecvList, pkHashNode); }
	SeNetSocketReset(pkNetSocket);
	SeHashAdd(&pkNetSocketMgr->kMainList, pkNetSocket->usIndex, &pkNetSocket->kMainNode);
}

void SeNetSocketMgrAddSendOrRecvInList(struct SESOCKETMGR *pkNetSocketMgr, struct SESOCKET *pkNetSocket, bool bSendOrRecv)
{
	struct SEHASHNODE *pkNetSocketTmp;

	if(bSendOrRecv == true)
	{
		pkNetSocketTmp = SeHashGet(&pkNetSocketMgr->kSendList, pkNetSocket->usIndex);
		//if(pkNetSocketTmp) { assert(&pkNetSocket->kSendNode == pkNetSocketTmp); SeHashRemove(&pkNetSocketMgr->kSendList, &pkNetSocket->kSendNode); }
		if(!pkNetSocketTmp) SeHashAdd(&pkNetSocketMgr->kSendList, pkNetSocket->usIndex, &pkNetSocket->kSendNode);
	}
	else
	{
		pkNetSocketTmp = SeHashGet(&pkNetSocketMgr->kRecvList, pkNetSocket->usIndex);
		//if(pkNetSocketTmp) { assert(&pkNetSocket->kRecvNode == pkNetSocketTmp); SeHashRemove(&pkNetSocketMgr->kRecvList, &pkNetSocket->kRecvNode); }
		if(!pkNetSocketTmp)SeHashAdd(&pkNetSocketMgr->kRecvList, pkNetSocket->usIndex, &pkNetSocket->kRecvNode);
	}
}

struct SESOCKET *SeNetSocketMgrPopSendOrRecvOutList(struct SESOCKETMGR *pkNetSocketMgr, bool bSendOrRecv)
{
	struct SESOCKET *pkNetSocket;
	struct SEHASHNODE *pkNetSocketTmp;

	if(bSendOrRecv == true)
	{
		pkNetSocketTmp = SeHashPop(&pkNetSocketMgr->kSendList);
		if(!pkNetSocketTmp) return 0;
		pkNetSocket = SE_CONTAINING_RECORD(pkNetSocketTmp, struct SESOCKET, kSendNode);
		return pkNetSocket;
	}
	else
	{
		pkNetSocketTmp = SeHashPop(&pkNetSocketMgr->kRecvList);
		if(!pkNetSocketTmp) return 0;
		pkNetSocket = SE_CONTAINING_RECORD(pkNetSocketTmp, struct SESOCKET, kRecvNode);
		return pkNetSocket;
	}
	return 0;
}

void SeNetSocketMgrUpdateNetStreamIdle(struct SESOCKETMGR *pkNetSocketMgr, int iHeaderLen, int iBufLen)
{
	int i;
	int iNode;
	int iSize;
	int iCount;
	char *pcBuf;
	struct SENETSTREAMNODE *pkNetStreamNode;
	
	iSize = MAX_BUF_LEN;
	iBufLen = iBufLen <= 0 ? 0 : iBufLen;
	
	assert(iHeaderLen >= 0);
	assert(iSize > (sizeof(struct SENETSTREAMNODE) + iHeaderLen));

	iNode = iSize - (sizeof(struct SENETSTREAMNODE) + iHeaderLen);
	iCount = ((iBufLen + (iNode - 1))/iNode)*2;
	
	if(SeNetSreamCount(&pkNetSocketMgr->kNetStreamIdle) >= iCount) return;

	for(i = 0; i < iCount; i++)
	{
		pcBuf = (char *)malloc(iSize);
		assert(pcBuf);
		pkNetStreamNode = SeNetSreamNodeFormat(pcBuf, iSize);
		SeNetSreamNodeZero(pkNetStreamNode);
		SeNetSreamHeadAdd(&pkNetSocketMgr->kNetStreamIdle, pkNetStreamNode);
	}
}

bool SeNetSocketMgrHasEvent(struct SESOCKET *pkNetSocket, int iEventSocket)
{
	return (((pkNetSocket->iEventSocket) & iEventSocket) == iEventSocket ? true : false);
}

void SeNetSocketMgrAddEvent(struct SESOCKET *pkNetSocket, int iEventSocket)
{
	pkNetSocket->iEventSocket |= iEventSocket;
}

void SeNetSocketMgrClearEvent(struct SESOCKET *pkNetSocket, int iEventSocket)
{
	pkNetSocket->iEventSocket &= ~iEventSocket;
}
