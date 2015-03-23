#include "SeNetSocket.h"

void SeNetCSocketNodeInit(struct SECSOCKETNODE *pkNetCSocketNode)
{
	pkNetCSocketNode->kHSocket = 0;
	pkNetCSocketNode->kBelongToListenSocket = SE_INVALID_SOCKET;
	pkNetCSocketNode->iStatus = CSOCKET_STATUS_INIT;
	pkNetCSocketNode->iEvent = 0;
	pkNetCSocketNode->iProtoFormat = 0;
	pkNetCSocketNode->iFlag = 0;
	SeNetSreamInit(&pkNetCSocketNode->kSendNetStream);
	SeNetSreamInit(&pkNetCSocketNode->kRecvNetStream);
	SeListInitNode(&pkNetCSocketNode->kNode);
}

void SeNetCSocketNodeFin(struct SECSOCKETNODE *pkNetCSocketNode, struct SENETSTREAM	*pkMemCache)
{
	struct SENETSTREAMNODE *pkNetStreamNode;

	pkNetStreamNode = SeNetSreamHeadPop(&pkNetCSocketNode->kSendNetStream);
	while(pkNetStreamNode) {
		SeNetSreamTailAdd(pkMemCache, pkNetStreamNode);
		pkNetStreamNode = SeNetSreamHeadPop(&pkNetCSocketNode->kSendNetStream);
	}
	pkNetStreamNode = SeNetSreamHeadPop(&pkNetCSocketNode->kRecvNetStream);
	while(pkNetStreamNode) {
		SeNetSreamTailAdd(pkMemCache, pkNetStreamNode);
		pkNetStreamNode = SeNetSreamHeadPop(&pkNetCSocketNode->kRecvNetStream);
	}

	pkNetCSocketNode->kHSocket = 0;
	pkNetCSocketNode->kBelongToListenSocket = SE_INVALID_SOCKET;
	pkNetCSocketNode->iStatus = CSOCKET_STATUS_INIT;
	pkNetCSocketNode->iEvent = 0;
	pkNetCSocketNode->iProtoFormat = 0;
}

void SeNetCSocketInit(struct SENETCSOCKET *pkNetCSocket)
{
	pkNetCSocket->llListCount = 0;
	SeListInit(&pkNetCSocket->kList);
}

struct SECSOCKETNODE *SeNetCSocketPop(struct SENETCSOCKET *pkNetCSocket)
{
	struct SENODE *pkNode = 0;
	struct SECSOCKETNODE *pkNetCScoketNode = 0;
	
	pkNode = SeListHeadPop(&pkNetCSocket->kList);
	if(!pkNode) return 0;
	pkNetCScoketNode = SE_CONTAINING_RECORD(pkNode, struct SECSOCKETNODE, kNode);
	pkNetCSocket->llListCount--;
	return pkNetCScoketNode;
}

void SeNetCSocketHeadAdd(struct SENETCSOCKET *pkNetCSocket, struct SECSOCKETNODE *pkNetCSocketNode)
{
	SeListHeadAdd(&pkNetCSocket->kList, &pkNetCSocketNode->kNode);
	pkNetCSocket->llListCount++;
}

void SeNetCSocketTailAdd(struct SENETCSOCKET *pkNetCSocket, struct SECSOCKETNODE *pkNetCSocketNode)
{
	SeListTailAdd(&pkNetCSocket->kList, &pkNetCSocketNode->kNode);
	pkNetCSocket->llListCount++;
}

struct SECSOCKETNODE *SeNetCSocketRemove(struct SENETCSOCKET *pkNetCSocket, struct SECSOCKETNODE *pkNetCSocketNode)
{
	struct SENODE *pkNode = 0;
	struct SECSOCKETNODE *pkNetCScoketNode = 0;

	pkNode = SeListRemove(&pkNetCSocket->kList, &pkNetCSocketNode->kNode);
	if(!pkNode) return 0;
	pkNetCScoketNode = SE_CONTAINING_RECORD(pkNode, struct SECSOCKETNODE, kNode);
	pkNetCSocket->llListCount--;
	return pkNetCScoketNode;
}


void SeNetSSocketNodeInit(struct SESSOCKETNODE *pkNetSSocketNode)
{
	pkNetSSocketNode->kListenSocket = SE_INVALID_SOCKET;
	pkNetSSocketNode->iProtoFormat = 0;
	pkNetSSocketNode->llMemSize = 0;
	SeNetSreamInit(&pkNetSSocketNode->kMemSCache);
	SeListInitNode(&pkNetSSocketNode->kNode);
}

void SeNetSSocketNodeFin(struct SESSOCKETNODE *pkNetSSocketNode, struct SENETSTREAM	*pkMemCache)
{
	struct SENETSTREAMNODE *pkNetStreamNode;

	pkNetStreamNode = SeNetSreamHeadPop(&pkNetSSocketNode->kMemSCache);
	while(pkNetStreamNode) {
		SeNetSreamTailAdd(pkMemCache, pkNetStreamNode);
		pkNetStreamNode = SeNetSreamHeadPop(&pkNetSSocketNode->kMemSCache);
	}

	pkNetSSocketNode->kListenSocket = SE_INVALID_SOCKET;
	pkNetSSocketNode->iProtoFormat = 0;
	pkNetSSocketNode->llMemSize = 0;
}

void SeNetSSocketInit(struct SENETSSOCKET *pkNetSSocket)
{
	pkNetSSocket->llListCount = 0;
	SeListInit(&pkNetSSocket->kList);
}

void SeNetSSocketAdd(struct SENETSSOCKET *pkNetSSocket, struct SESSOCKETNODE *pkNetSSocketNode)
{
	SeListHeadAdd(&pkNetSSocket->kList, &pkNetSSocketNode->kNode);
	pkNetSSocket->llListCount++;
}

struct SESSOCKETNODE *SeNetSSocketPop(struct SENETSSOCKET *pkNetSSocket)
{
	struct SENODE *pkNode = 0;
	struct SESSOCKETNODE *pkNetSScoketNode = 0;
	
	pkNode = SeListHeadPop(&pkNetSSocket->kList);
	if(!pkNode) return 0;
	pkNetSScoketNode = SE_CONTAINING_RECORD(pkNode, struct SESSOCKETNODE, kNode);
	pkNetSSocket->llListCount--;
	return pkNetSScoketNode;
}

