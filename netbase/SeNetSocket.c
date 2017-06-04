#include "SeNetSocket.h"
#include "SeTool.h"
#include "SeTime.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#define MAX_BUF_LEN 3904
#define MAX_CONNECT_TIME_OUT 5*1000

void SeNetSocketReset(struct SESOCKET *pkNetSocket)
{
	pkNetSocket->kHSocket = SeGetHSocket(0, 0, 0);
	pkNetSocket->kBelongListenHSocket = SeGetHSocket(0, 0, 0);
	pkNetSocket->usStatus = SOCKET_STATUS_INIT;
	pkNetSocket->iHeaderLen = 0;
	pkNetSocket->iTypeSocket = 0;
	pkNetSocket->iEventSocket = 0;
	pkNetSocket->llFlag = 0;
	pkNetSocket->llTime = 0;
	pkNetSocket->pkGetHeaderLenFun = 0;
	pkNetSocket->pkSetHeaderLenFun = 0;
	pkNetSocket->llActiveTimeOut = 0;
	memset(pkNetSocket->acBindSvrName, 0, sizeof(pkNetSocket->acBindSvrName));
}

void SeNetSocketInit(struct SESOCKET *pkNetSocket, unsigned short usIndex)
{
	SeHashNodeInit(&pkNetSocket->kMainNode);
	SeHashNodeInit(&pkNetSocket->kSendNode);
	SeHashNodeInit(&pkNetSocket->kRecvNode);
	pkNetSocket->usIndex = usIndex;
	SeNetSocketReset(pkNetSocket);
	SeNetSreamInit(&pkNetSocket->kSendNetStream);
	SeNetSreamInit(&pkNetSocket->kRecvNetStream);
}

void SeNetSocketMgrInit(struct SESOCKETMGR *pkNetSocketMgr, unsigned short usMax)
{
	int i;

	assert(usMax > 0);
	pkNetSocketMgr->iCounter = 0;
	pkNetSocketMgr->iMax = usMax;
	pkNetSocketMgr->pkMainList = (struct SEHASH*)SeMallocMem(sizeof(struct SEHASH));
	pkNetSocketMgr->pkActiveMainList = (struct SEHASH*)SeMallocMem(sizeof(struct SEHASH));
	pkNetSocketMgr->pkSendList = (struct SEHASH*)SeMallocMem(sizeof(struct SEHASH));
	pkNetSocketMgr->pkRecvList = (struct SEHASH*)SeMallocMem(sizeof(struct SEHASH));
	pkNetSocketMgr->pkNetStreamIdle = (struct SENETSTREAM*)SeMallocMem(sizeof(struct SENETSTREAM));
	pkNetSocketMgr->pkSeSocket = (struct SESOCKET *)SeMallocMem(sizeof(struct SESOCKET)*pkNetSocketMgr->iMax);
	SeNetSreamInit(pkNetSocketMgr->pkNetStreamIdle);
	SeHashInit(pkNetSocketMgr->pkMainList, pkNetSocketMgr->iMax);
	SeHashInit(pkNetSocketMgr->pkActiveMainList, pkNetSocketMgr->iMax);
	SeHashInit(pkNetSocketMgr->pkSendList, pkNetSocketMgr->iMax);
	SeHashInit(pkNetSocketMgr->pkRecvList, pkNetSocketMgr->iMax);

	for(i = 0; i < pkNetSocketMgr->iMax; i++)
	{
		SeNetSocketInit(&pkNetSocketMgr->pkSeSocket[i], (unsigned short)i);
		SeNetSocketReset(&pkNetSocketMgr->pkSeSocket[i]);
		SeHashAdd(pkNetSocketMgr->pkMainList, pkNetSocketMgr->pkSeSocket[i].usIndex, &((pkNetSocketMgr->pkSeSocket[i]).kMainNode));
	}
}

void SeNetSocketMgrEnd(struct SESOCKETMGR *pkNetSocketMgr, struct SESOCKET *pkNetSocket)
{
	struct SENETSTREAMNODE *pkNetStreamNode;

	pkNetStreamNode = SeNetSreamHeadPop(&pkNetSocket->kSendNetStream);
	while(pkNetStreamNode)
	{
		SeNetSreamNodeZero(pkNetStreamNode);
		SeNetSreamHeadAdd(pkNetSocketMgr->pkNetStreamIdle, pkNetStreamNode);
		pkNetStreamNode = SeNetSreamHeadPop(&pkNetSocket->kSendNetStream);
	}

	pkNetStreamNode = SeNetSreamHeadPop(&pkNetSocket->kRecvNetStream);
	while(pkNetStreamNode)
	{
		SeNetSreamNodeZero(pkNetStreamNode);
		SeNetSreamHeadAdd(pkNetSocketMgr->pkNetStreamIdle, pkNetStreamNode);
		pkNetStreamNode = SeNetSreamHeadPop(&pkNetSocket->kRecvNetStream);
	}
}

void SeNetSocketMgrFin(struct SESOCKETMGR *pkNetSocketMgr)
{
	int i;
	struct SENETSTREAMNODE *pkNetStreamNode;

	for(i = 0; i < pkNetSocketMgr->iMax; i++)
	{
		SeNetSocketMgrEnd(pkNetSocketMgr, &pkNetSocketMgr->pkSeSocket[i]);
	}

	pkNetStreamNode = SeNetSreamHeadPop(pkNetSocketMgr->pkNetStreamIdle);
	while(pkNetStreamNode)
	{
		SeFreeMem(pkNetStreamNode);
		pkNetStreamNode = SeNetSreamHeadPop(pkNetSocketMgr->pkNetStreamIdle);
	}

	SeHashFin(pkNetSocketMgr->pkMainList);
	SeHashFin(pkNetSocketMgr->pkActiveMainList);
	SeHashFin(pkNetSocketMgr->pkSendList);
	SeHashFin(pkNetSocketMgr->pkRecvList);
	
	SeFreeMem(pkNetSocketMgr->pkMainList);
	SeFreeMem(pkNetSocketMgr->pkActiveMainList);
	SeFreeMem(pkNetSocketMgr->pkSendList);
	SeFreeMem(pkNetSocketMgr->pkRecvList);
	SeFreeMem(pkNetSocketMgr->pkNetStreamIdle);
	SeFreeMem(pkNetSocketMgr->pkSeSocket);
}

HSOCKET SeNetSocketMgrAdd(struct SESOCKETMGR *pkNetSocketMgr, SOCKET socket, int iTypeSocket, int iHeaderLen, \
			SEGETHEADERLENFUN pkGetHeaderLenFun, SESETHEADERLENFUN pkSetHeaderLenFun)
{
	int iCounter;
	unsigned short usIndex;
	struct SESOCKET *pkNetSocket;
	struct SEHASHNODE *pkHashNode;
	
	pkNetSocketMgr->iCounter++;
	iCounter = pkNetSocketMgr->iCounter;

	assert(socket > 0);
	pkHashNode = SeHashPop(pkNetSocketMgr->pkMainList);
	if(!pkHashNode) return 0;
	pkNetSocket = SE_CONTAINING_RECORD(pkHashNode, struct SESOCKET, kMainNode);
	usIndex = pkNetSocket->usIndex;

	SeNetSocketReset(pkNetSocket);
	pkNetSocket->kHSocket = SeGetHSocket((unsigned short)iCounter, usIndex, socket);
	pkNetSocket->usStatus = SOCKET_STATUS_INIT;
	pkNetSocket->iHeaderLen = iHeaderLen;
	pkNetSocket->iTypeSocket = iTypeSocket;
	pkNetSocket->llTime = SeTimeGetTickCount();
	pkNetSocket->pkGetHeaderLenFun = pkGetHeaderLenFun;
	pkNetSocket->pkSetHeaderLenFun = pkSetHeaderLenFun;

	if(iTypeSocket == CLIENT_TCP_TYPE_SOCKET || iTypeSocket == ACCEPT_TCP_TYPE_SOCKET ) 
	{ SeHashAdd(pkNetSocketMgr->pkActiveMainList, usIndex, &pkNetSocket->kMainNode); }

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
	pkHashNode = SeHashGet(pkNetSocketMgr->pkMainList, usIndex);
	if(pkHashNode) return 0;
	pkNetSocket = &pkNetSocketMgr->pkSeSocket[usIndex];
	if(kHSocket != pkNetSocket->kHSocket) return 0;
	return pkNetSocket;
}

void SeNetSocketMgrDel(struct SESOCKETMGR *pkNetSocketMgr, HSOCKET kHSocket)
{
	unsigned short usIndex;
	struct SESOCKET *pkNetSocket;
	struct SEHASHNODE *pkHashNode;
	
	pkNetSocket = SeNetSocketMgrGet(pkNetSocketMgr, kHSocket);
	if(!pkNetSocket) return;
	SeNetSocketReset(pkNetSocket);
	usIndex = pkNetSocket->usIndex;
	SeNetSocketMgrEnd(pkNetSocketMgr, pkNetSocket);

	pkHashNode = SeHashGet(pkNetSocketMgr->pkSendList, usIndex);
	if(pkHashNode) { assert(&pkNetSocket->kSendNode == pkHashNode); SeHashRemove(pkNetSocketMgr->pkSendList, pkHashNode); }
	pkHashNode = SeHashGet(pkNetSocketMgr->pkRecvList, usIndex);
	if(pkHashNode) { assert(&pkNetSocket->kRecvNode == pkHashNode); SeHashRemove(pkNetSocketMgr->pkRecvList, pkHashNode); }
	
	if(SeHashGet(pkNetSocketMgr->pkActiveMainList, usIndex)) { SeHashRemove(pkNetSocketMgr->pkActiveMainList, &pkNetSocket->kMainNode); }
	SeHashAdd(pkNetSocketMgr->pkMainList, usIndex, &pkNetSocket->kMainNode);
}

void SeNetSocketMgrAddSendOrRecvInList(struct SESOCKETMGR *pkNetSocketMgr, struct SESOCKET *pkNetSocket, bool bSendOrRecv)
{
	unsigned short usIndex;
	struct SEHASHNODE *pkNetSocketTmp;
	
	usIndex = pkNetSocket->usIndex;

	if(bSendOrRecv == true)
	{
		pkNetSocketTmp = SeHashGet(pkNetSocketMgr->pkSendList, usIndex);
		if(!pkNetSocketTmp) SeHashAdd(pkNetSocketMgr->pkSendList, usIndex, &pkNetSocket->kSendNode);
	}
	else
	{
		pkNetSocketTmp = SeHashGet(pkNetSocketMgr->pkRecvList, usIndex);
		if(!pkNetSocketTmp)SeHashAdd(pkNetSocketMgr->pkRecvList, usIndex, &pkNetSocket->kRecvNode);
	}
}

struct SESOCKET *SeNetSocketMgrPopSendOrRecvOutList(struct SESOCKETMGR *pkNetSocketMgr, bool bSendOrRecv)
{
	struct SESOCKET *pkNetSocket;
	struct SEHASHNODE *pkNetSocketTmp;

	if(bSendOrRecv == true)
	{
		pkNetSocketTmp = SeHashPop(pkNetSocketMgr->pkSendList);
		if(!pkNetSocketTmp) return 0;
		pkNetSocket = SE_CONTAINING_RECORD(pkNetSocketTmp, struct SESOCKET, kSendNode);
		return pkNetSocket;
	}
	else
	{
		pkNetSocketTmp = SeHashPop(pkNetSocketMgr->pkRecvList);
		if(!pkNetSocketTmp) return 0;
		pkNetSocket = SE_CONTAINING_RECORD(pkNetSocketTmp, struct SESOCKET, kRecvNode);
		return pkNetSocket;
	}
	return 0;
}

bool SeNetSocketMgrUpdateNetStreamIdle(struct SESOCKETMGR *pkNetSocketMgr, int iHeaderLen, int iBufLen)
{
	int i;
	int iNode;
	int iCount;
	char *pcBuf;
	struct SENETSTREAMNODE *pkNetStreamNode;
	
	iBufLen = iBufLen <= 0 ? MAX_BUF_LEN : iBufLen;
	
	assert(iHeaderLen >= 0);
	assert(MAX_BUF_LEN > (int)(sizeof(struct SENETSTREAMNODE) + iHeaderLen));

	iNode = MAX_BUF_LEN - (int)(sizeof(struct SENETSTREAMNODE) + iHeaderLen);
	iCount = ((iBufLen + (iNode - 1))/iNode)*2;
	
	if(SeNetSreamCount(pkNetSocketMgr->pkNetStreamIdle) >= iCount) return true;

	for(i = 0; i < iCount; i++)
	{
		pcBuf = (char *)SeMallocMem(MAX_BUF_LEN);
		assert(pcBuf);
		if(!pcBuf) { return false; }
		pkNetStreamNode = SeNetSreamNodeFormat(pcBuf, MAX_BUF_LEN);
		SeNetSreamNodeZero(pkNetStreamNode);
		SeNetSreamHeadAdd(pkNetSocketMgr->pkNetStreamIdle, pkNetStreamNode);
	}
	return true;
}

void SeNetSocketMgrActive(struct SESOCKETMGR *pkNetSocketMgr, struct SESOCKET *pkNetSocket)
{
	unsigned short usIndex;
	
	usIndex = pkNetSocket->usIndex;
	pkNetSocket->llTime = SeTimeGetTickCount();
	if(!SeHashGet(pkNetSocketMgr->pkActiveMainList, usIndex)) { return; }
	SeHashMoveToEnd(pkNetSocketMgr->pkActiveMainList, &pkNetSocket->kMainNode);
}

const struct SESOCKET *SeNetSocketMgrTimeOut(struct SESOCKETMGR *pkNetSocketMgr)
{
	struct SESOCKET *pkNetSocket;
	struct SEHASHNODE *pkHashNode;
	unsigned long long llTimeOut;

	pkHashNode = SeHashGetHead(pkNetSocketMgr->pkActiveMainList);
	if(!pkHashNode)
	{
		return 0;
	}
	pkNetSocket = SE_CONTAINING_RECORD(pkHashNode, struct SESOCKET, kMainNode);
	SeHashMoveToEnd(pkNetSocketMgr->pkActiveMainList, &pkNetSocket->kMainNode);

	llTimeOut = pkNetSocket->usStatus == SOCKET_STATUS_CONNECTING ? MAX_CONNECT_TIME_OUT : pkNetSocket->llActiveTimeOut;
	if((pkNetSocket->llTime + llTimeOut) > SeTimeGetTickCount()) { return 0; }

	return pkNetSocket;
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
