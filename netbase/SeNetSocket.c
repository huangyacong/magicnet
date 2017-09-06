#include "SeNetSocket.h"
#include "SeTool.h"
#include "SeTime.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#define MAX_BUF_LEN 3904

void SeNetSocketReset(struct SESOCKET *pkNetSocket)
{
	pkNetSocket->kHSocket = SeGetHSocket(0, 0, 0);
	pkNetSocket->kBelongListenHSocket = SeGetHSocket(0, 0, 0);
	pkNetSocket->usStatus = SOCKET_STATUS_INIT;
	pkNetSocket->iHeaderLen = 0;
	pkNetSocket->iTypeSocket = 0;
	pkNetSocket->iEventSocket = 0;
	pkNetSocket->llTime = 0;
	pkNetSocket->pkGetHeaderLenFun = 0;
	pkNetSocket->pkSetHeaderLenFun = 0;
	pkNetSocket->iConnectTimeOut = 5*1000;
	pkNetSocket->iActiveTimeOut = 0;
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
	pkNetSocketMgr->llCounter = 0;
	pkNetSocketMgr->llMax = usMax;
	pkNetSocketMgr->llFlag = 0;
	pkNetSocketMgr->pkSeSocket = (struct SESOCKET *)SeMallocMem(sizeof(struct SESOCKET)*(int)(pkNetSocketMgr->llMax));
	SeNetSreamInit(&(pkNetSocketMgr->kNetStreamIdle));
	SeHashInit(&(pkNetSocketMgr->kMainList), (int)(pkNetSocketMgr->llMax));
	SeHashInit(&(pkNetSocketMgr->kActiveMainList), (int)(pkNetSocketMgr->llMax));
	SeHashInit(&(pkNetSocketMgr->kSendList), (int)(pkNetSocketMgr->llMax));
	SeHashInit(&(pkNetSocketMgr->kRecvList), (int)(pkNetSocketMgr->llMax));

	for (i = 0; i < (int)(pkNetSocketMgr->llMax); i++)
	{
		SeNetSocketInit(&pkNetSocketMgr->pkSeSocket[i], (unsigned short)i);
		SeNetSocketReset(&pkNetSocketMgr->pkSeSocket[i]);
		SeHashAdd(&(pkNetSocketMgr->kMainList), pkNetSocketMgr->pkSeSocket[i].usIndex, &((pkNetSocketMgr->pkSeSocket[i]).kMainNode));
	}
}

void SeNetSocketMgrEnd(struct SESOCKETMGR *pkNetSocketMgr, struct SESOCKET *pkNetSocket)
{
	struct SENETSTREAMNODE *pkNetStreamNode;

	pkNetStreamNode = SeNetSreamHeadPop(&pkNetSocket->kSendNetStream);
	while(pkNetStreamNode)
	{
		SeNetSreamNodeZero(pkNetStreamNode);
		SeNetSreamHeadAdd(&(pkNetSocketMgr->kNetStreamIdle), pkNetStreamNode);
		pkNetStreamNode = SeNetSreamHeadPop(&pkNetSocket->kSendNetStream);
	}

	pkNetStreamNode = SeNetSreamHeadPop(&pkNetSocket->kRecvNetStream);
	while(pkNetStreamNode)
	{
		SeNetSreamNodeZero(pkNetStreamNode);
		SeNetSreamHeadAdd(&(pkNetSocketMgr->kNetStreamIdle), pkNetStreamNode);
		pkNetStreamNode = SeNetSreamHeadPop(&pkNetSocket->kRecvNetStream);
	}
}

void SeNetSocketMgrFin(struct SESOCKETMGR *pkNetSocketMgr)
{
	int i;
	struct SENETSTREAMNODE *pkNetStreamNode;

	for (i = 0; i < (int)(pkNetSocketMgr->llMax); i++)
	{
		SeNetSocketMgrEnd(pkNetSocketMgr, &pkNetSocketMgr->pkSeSocket[i]);
	}

	pkNetStreamNode = SeNetSreamHeadPop(&(pkNetSocketMgr->kNetStreamIdle));
	while(pkNetStreamNode)
	{
		SeFreeMem(pkNetStreamNode);
		pkNetStreamNode = SeNetSreamHeadPop(&(pkNetSocketMgr->kNetStreamIdle));
	}

	SeHashFin(&(pkNetSocketMgr->kMainList));
	SeHashFin(&(pkNetSocketMgr->kActiveMainList));
	SeHashFin(&(pkNetSocketMgr->kSendList));
	SeHashFin(&(pkNetSocketMgr->kRecvList));
	
	SeFreeMem(pkNetSocketMgr->pkSeSocket);
}

HSOCKET SeNetSocketMgrAdd(struct SESOCKETMGR *pkNetSocketMgr, SOCKET socket, int iTypeSocket, int iHeaderLen, \
			SEGETHEADERLENFUN pkGetHeaderLenFun, SESETHEADERLENFUN pkSetHeaderLenFun)
{
	int iCounter;
	unsigned short usIndex;
	struct SESOCKET *pkNetSocket;
	struct SEHASHNODE *pkHashNode;
	
	pkNetSocketMgr->llCounter++;
	iCounter = (int)pkNetSocketMgr->llCounter;

	assert(socket > 0);
	pkHashNode = SeHashPop(&(pkNetSocketMgr->kMainList));
	if (!pkHashNode)
	{
		return 0;
	}
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
	{
		SeHashAdd(&(pkNetSocketMgr->kActiveMainList), usIndex, &pkNetSocket->kMainNode);
	}

	return pkNetSocket->kHSocket;
}

struct SESOCKET *SeNetSocketMgrGet(struct SESOCKETMGR *pkNetSocketMgr, HSOCKET kHSocket)
{
	unsigned short usIndex;
	struct SESOCKET *pkNetSocket;
	struct SEHASHNODE *pkHashNode;
	
	usIndex = SeGetIndexByHScoket(kHSocket);
	assert(usIndex >= 0 && usIndex < pkNetSocketMgr->llMax);
	if (usIndex < 0 || usIndex >= (int)pkNetSocketMgr->llMax)
	{
		return 0;
	}
	pkHashNode = SeHashGet(&(pkNetSocketMgr->kMainList), usIndex);
	if (pkHashNode)
	{
		return 0;
	}
	pkNetSocket = &pkNetSocketMgr->pkSeSocket[usIndex];
	if (kHSocket != pkNetSocket->kHSocket)
	{
		return 0;
	}
	return pkNetSocket;
}

void SeNetSocketMgrDel(struct SESOCKETMGR *pkNetSocketMgr, HSOCKET kHSocket)
{
	unsigned short usIndex;
	struct SESOCKET *pkNetSocket;
	struct SEHASHNODE *pkHashNode;
	
	pkNetSocket = SeNetSocketMgrGet(pkNetSocketMgr, kHSocket);
	if (!pkNetSocket)
	{
		return;
	}
	SeNetSocketReset(pkNetSocket);
	usIndex = pkNetSocket->usIndex;
	SeNetSocketMgrEnd(pkNetSocketMgr, pkNetSocket);

	pkHashNode = SeHashGet(&(pkNetSocketMgr->kSendList), usIndex);
	if (pkHashNode)
	{
		assert(&pkNetSocket->kSendNode == pkHashNode);
		SeHashRemove(&(pkNetSocketMgr->kSendList), pkHashNode);
	}
	pkHashNode = SeHashGet(&(pkNetSocketMgr->kRecvList), usIndex);
	if (pkHashNode)
	{
		assert(&pkNetSocket->kRecvNode == pkHashNode);
		SeHashRemove(&(pkNetSocketMgr->kRecvList), pkHashNode);
	}
	
	if (SeHashGet(&(pkNetSocketMgr->kActiveMainList), usIndex))
	{
		SeHashRemove(&(pkNetSocketMgr->kActiveMainList), &pkNetSocket->kMainNode);
	}
	SeHashAdd(&(pkNetSocketMgr->kMainList), usIndex, &pkNetSocket->kMainNode);
}

void SeNetSocketMgrAddSendOrRecvInList(struct SESOCKETMGR *pkNetSocketMgr, struct SESOCKET *pkNetSocket, bool bSendOrRecv)
{
	unsigned short usIndex;
	struct SEHASHNODE *pkNetSocketTmp;
	
	usIndex = pkNetSocket->usIndex;

	if(bSendOrRecv == true)
	{
		pkNetSocketTmp = SeHashGet(&(pkNetSocketMgr->kSendList), usIndex);
		if (!pkNetSocketTmp)
		{
			SeHashAdd(&(pkNetSocketMgr->kSendList), usIndex, &pkNetSocket->kSendNode);
		}
	}
	else
	{
		pkNetSocketTmp = SeHashGet(&(pkNetSocketMgr->kRecvList), usIndex);
		if (!pkNetSocketTmp)
		{
			SeHashAdd(&(pkNetSocketMgr->kRecvList), usIndex, &pkNetSocket->kRecvNode);
		}
	}
}

struct SESOCKET *SeNetSocketMgrPopSendOrRecvOutList(struct SESOCKETMGR *pkNetSocketMgr, bool bSendOrRecv)
{
	struct SESOCKET *pkNetSocket;
	struct SEHASHNODE *pkNetSocketTmp;

	if(bSendOrRecv == true)
	{
		pkNetSocketTmp = SeHashPop(&(pkNetSocketMgr->kSendList));
		if (!pkNetSocketTmp)
		{
			return 0;
		}
		pkNetSocket = SE_CONTAINING_RECORD(pkNetSocketTmp, struct SESOCKET, kSendNode);
		return pkNetSocket;
	}
	else
	{
		pkNetSocketTmp = SeHashPop(&(pkNetSocketMgr->kRecvList));
		if (!pkNetSocketTmp)
		{
			return 0;
		}
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
	
	if (SeNetSreamCount(&(pkNetSocketMgr->kNetStreamIdle)) >= iCount)
	{
		return true;
	}

	for(i = 0; i < iCount; i++)
	{
		pcBuf = (char *)SeMallocMem(MAX_BUF_LEN);
		assert(pcBuf);
		if(!pcBuf)
		{
			return false;
		}
		pkNetStreamNode = SeNetSreamNodeFormat(pcBuf, MAX_BUF_LEN);
		if (!pkNetStreamNode)
		{
			return false;
		}
		SeNetSreamNodeZero(pkNetStreamNode);
		SeNetSreamHeadAdd(&(pkNetSocketMgr->kNetStreamIdle), pkNetStreamNode);
	}
	return true;
}

struct SESOCKET *SeNetSocketMgrTimeOut(struct SESOCKETMGR *pkNetSocketMgr)
{
	struct SESOCKET *pkNetSocket;
	struct SEHASHNODE *pkHashNode;

	pkHashNode = SeHashGetHead(&(pkNetSocketMgr->kActiveMainList));
	if(!pkHashNode)
	{
		return 0;
	}
	pkNetSocket = SE_CONTAINING_RECORD(pkHashNode, struct SESOCKET, kMainNode);
	SeHashMoveToEnd(&(pkNetSocketMgr->kActiveMainList), &pkNetSocket->kMainNode);

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
