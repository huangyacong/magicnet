#include "SeNetSocket.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "SeTool.h"
#include "SeTime.h"

#define MAX_BUF_LEN 1024*4

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
}

void SeNetSocketInit(struct SESOCKET *pkNetSocket, unsigned short usIndex)
{
	pkNetSocket->usIndex = usIndex;
	SeNetSocketReset(pkNetSocket);
	pkNetSocket->pkMainNode = (struct SEHASHNODE*)SeMallocMem(sizeof(struct SEHASHNODE));
	pkNetSocket->pkSendNode = (struct SEHASHNODE*)SeMallocMem(sizeof(struct SEHASHNODE));
	pkNetSocket->pkRecvNode = (struct SEHASHNODE*)SeMallocMem(sizeof(struct SEHASHNODE));
	pkNetSocket->pkSendNetStream = (struct SENETSTREAM*)SeMallocMem(sizeof(struct SENETSTREAM));
	pkNetSocket->pkRecvNetStream = (struct SENETSTREAM*)SeMallocMem(sizeof(struct SENETSTREAM));
	SeNetSreamInit(pkNetSocket->pkSendNetStream);
	SeNetSreamInit(pkNetSocket->pkRecvNetStream);
	SeHashNodeInit(pkNetSocket->pkMainNode);
	SeHashNodeInit(pkNetSocket->pkSendNode);
	SeHashNodeInit(pkNetSocket->pkRecvNode);
}

void SeNetSocketMgrInit(struct SESOCKETMGR *pkNetSocketMgr, int iTimeOut, unsigned short usMax)
{
	int i;

	assert(usMax > 0);
	pkNetSocketMgr->iCounter = 0;
	pkNetSocketMgr->llTimeOut = iTimeOut;
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
		SeHashAdd(pkNetSocketMgr->pkMainList, pkNetSocketMgr->pkSeSocket[i].usIndex, ((pkNetSocketMgr->pkSeSocket[i]).pkMainNode));
	}
}

void SeNetSocketMgrEnd(struct SESOCKETMGR *pkNetSocketMgr, struct SESOCKET *pkNetSocket, bool bDel)
{
	struct SENETSTREAMNODE *pkNetStreamNode;

	pkNetStreamNode = SeNetSreamHeadPop(pkNetSocket->pkSendNetStream);
	while(pkNetStreamNode)
	{
		SeNetSreamNodeZero(pkNetStreamNode);
		SeNetSreamHeadAdd(pkNetSocketMgr->pkNetStreamIdle, pkNetStreamNode);
		pkNetStreamNode = SeNetSreamHeadPop(pkNetSocket->pkSendNetStream);
	}

	pkNetStreamNode = SeNetSreamHeadPop(pkNetSocket->pkRecvNetStream);
	while(pkNetStreamNode)
	{
		SeNetSreamNodeZero(pkNetStreamNode);
		SeNetSreamHeadAdd(pkNetSocketMgr->pkNetStreamIdle, pkNetStreamNode);
		pkNetStreamNode = SeNetSreamHeadPop(pkNetSocket->pkRecvNetStream);
	}

	if(bDel == true)
	{
		SeFreeMem(pkNetSocket->pkMainNode);
		SeFreeMem(pkNetSocket->pkSendNode);
		SeFreeMem(pkNetSocket->pkRecvNode);
		SeFreeMem(pkNetSocket->pkSendNetStream);
		SeFreeMem(pkNetSocket->pkRecvNetStream);
		pkNetSocket->pkMainNode = 0;
		pkNetSocket->pkSendNode = 0;
		pkNetSocket->pkRecvNode = 0;
		pkNetSocket->pkSendNetStream = 0;
		pkNetSocket->pkRecvNetStream = 0;
	}
}

void SeNetSocketMgrFin(struct SESOCKETMGR *pkNetSocketMgr)
{
	int i;
	struct SENETSTREAMNODE *pkNetStreamNode;

	for(i = 0; i < pkNetSocketMgr->iMax; i++) SeNetSocketMgrEnd(pkNetSocketMgr, &pkNetSocketMgr->pkSeSocket[i], true);
	pkNetStreamNode = SeNetSreamHeadPop(pkNetSocketMgr->pkNetStreamIdle);
	while(pkNetStreamNode) {SeFreeMem(pkNetStreamNode);pkNetStreamNode = SeNetSreamHeadPop(pkNetSocketMgr->pkNetStreamIdle);}

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

	pkNetSocketMgr->pkMainList = 0;
	pkNetSocketMgr->pkActiveMainList = 0;
	pkNetSocketMgr->pkSendList = 0;
	pkNetSocketMgr->pkRecvList = 0;
	pkNetSocketMgr->pkNetStreamIdle = 0;
	pkNetSocketMgr->pkSeSocket = 0;
}

HSOCKET SeNetSocketMgrAdd(struct SESOCKETMGR *pkNetSocketMgr, SOCKET socket, int iTypeSocket, int iHeaderLen, \
			SEGETHEADERLENFUN pkGetHeaderLenFun, SESETHEADERLENFUN pkSetHeaderLenFun)
{
	struct SEHASHNODE *pkHashNode;
	struct SESOCKET *pkNetSocket;
	
	assert(socket > 0);
	pkHashNode = SeHashPop(pkNetSocketMgr->pkMainList);
	if(!pkHashNode) return 0;
	pkNetSocket = SE_CONTAINING_RECORD(pkHashNode, struct SESOCKET, pkMainNode);
	SeNetSocketReset(pkNetSocket);
	pkNetSocketMgr->iCounter++;

	if(iTypeSocket == CLIENT_TCP_TYPE_SOCKET || iTypeSocket == ACCEPT_TCP_TYPE_SOCKET )
	{
		SeHashAdd(pkNetSocketMgr->pkActiveMainList, pkNetSocket->usIndex, pkNetSocket->pkMainNode);
	}

	pkNetSocket->kHSocket = SeGetHSocket((unsigned short)pkNetSocketMgr->iCounter, pkNetSocket->usIndex, socket);
	pkNetSocket->usStatus = SOCKET_STATUS_INIT;
	pkNetSocket->iHeaderLen = iHeaderLen;
	pkNetSocket->iTypeSocket = iTypeSocket;
	pkNetSocket->llTime = SeTimeGetTickCount();
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
	pkHashNode = SeHashGet(pkNetSocketMgr->pkMainList, usIndex);
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
	SeNetSocketMgrEnd(pkNetSocketMgr, pkNetSocket, false);
	pkHashNode = SeHashGet(pkNetSocketMgr->pkSendList, pkNetSocket->usIndex);
	if(pkHashNode) { assert(pkNetSocket->pkSendNode == pkHashNode); SeHashRemove(pkNetSocketMgr->pkSendList, pkHashNode); }
	pkHashNode = SeHashGet(pkNetSocketMgr->pkRecvList, pkNetSocket->usIndex);
	if(pkHashNode) { assert(pkNetSocket->pkRecvNode == pkHashNode); SeHashRemove(pkNetSocketMgr->pkRecvList, pkHashNode); }
	SeNetSocketReset(pkNetSocket);
	if(SeHashGet(pkNetSocketMgr->pkActiveMainList, pkNetSocket->usIndex)) { SeHashRemove(pkNetSocketMgr->pkActiveMainList, pkNetSocket->pkMainNode); }
	SeHashAdd(pkNetSocketMgr->pkMainList, pkNetSocket->usIndex, pkNetSocket->pkMainNode);
}

void SeNetSocketMgrAddSendOrRecvInList(struct SESOCKETMGR *pkNetSocketMgr, struct SESOCKET *pkNetSocket, bool bSendOrRecv)
{
	struct SEHASHNODE *pkNetSocketTmp;

	if(bSendOrRecv == true)
	{
		pkNetSocketTmp = SeHashGet(pkNetSocketMgr->pkSendList, pkNetSocket->usIndex);
		if(!pkNetSocketTmp) SeHashAdd(pkNetSocketMgr->pkSendList, pkNetSocket->usIndex, pkNetSocket->pkSendNode);
	}
	else
	{
		pkNetSocketTmp = SeHashGet(pkNetSocketMgr->pkRecvList, pkNetSocket->usIndex);
		if(!pkNetSocketTmp)SeHashAdd(pkNetSocketMgr->pkRecvList, pkNetSocket->usIndex, pkNetSocket->pkRecvNode);
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
		pkNetSocket = SE_CONTAINING_RECORD(pkNetSocketTmp, struct SESOCKET, pkSendNode);
		return pkNetSocket;
	}
	else
	{
		pkNetSocketTmp = SeHashPop(pkNetSocketMgr->pkRecvList);
		if(!pkNetSocketTmp) return 0;
		pkNetSocket = SE_CONTAINING_RECORD(pkNetSocketTmp, struct SESOCKET, pkRecvNode);
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
	iBufLen = iBufLen <= 0 ? 1024 : iBufLen;
	
	assert(iHeaderLen >= 0);
	assert(iSize > (int)(sizeof(struct SENETSTREAMNODE) + iHeaderLen));

	iNode = iSize - (int)(sizeof(struct SENETSTREAMNODE) + iHeaderLen);
	iCount = ((iBufLen + (iNode - 1))/iNode)*2;
	
	if(SeNetSreamCount(pkNetSocketMgr->pkNetStreamIdle) >= iCount) return;

	for(i = 0; i < iCount; i++)
	{
		pcBuf = (char *)SeMallocMem(iSize);
		assert(pcBuf);
		if(!pcBuf) { continue; }
		pkNetStreamNode = SeNetSreamNodeFormat(pcBuf, iSize);
		SeNetSreamNodeZero(pkNetStreamNode);
		SeNetSreamHeadAdd(pkNetSocketMgr->pkNetStreamIdle, pkNetStreamNode);
	}
}

void SeNetSocketMgrActive(struct SESOCKETMGR *pkNetSocketMgr, struct SESOCKET *pkNetSocket)
{
	pkNetSocket->llTime = SeTimeGetTickCount();
	if(!SeHashGet(pkNetSocketMgr->pkActiveMainList, pkNetSocket->usIndex)) { return; }
	SeHashMoveToEnd(pkNetSocketMgr->pkActiveMainList, pkNetSocket->pkMainNode);
}

const struct SESOCKET *SeNetSocketMgrTimeOut(struct SESOCKETMGR *pkNetSocketMgr)
{
	struct SESOCKET *pkNetSocket;
	struct SEHASHNODE *pkHashNode;
	unsigned long long llTimeOut;

	pkHashNode = SeHashGetHead(pkNetSocketMgr->pkActiveMainList);
	if(!pkHashNode) { return 0; }
	llTimeOut = pkNetSocketMgr->llTimeOut;
	pkNetSocket = SE_CONTAINING_RECORD(pkHashNode, struct SESOCKET, pkMainNode);
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
