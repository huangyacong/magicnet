#include "SeNetSocket.h"

void SeNetCSocketNodeInit(struct SECSOCKETNODE *pkNetCSocketNode)
{
	pkNetCSocketNode->kHSocket = SE_INVALID_SOCKET;
	pkNetCSocketNode->kBelongToListenSocket = SE_INVALID_SOCKET;
	pkNetCSocketNode->iStatus = CSOCKET_STATUS_INIT;
	pkNetCSocketNode->iFlag = 0;
	SeNetSreamInit(&pkNetCSocketNode->kSendNetStream);
	SeNetSreamInit(&pkNetCSocketNode->kRecvNetStream);
	SeListInitNode(&pkNetCSocketNode->kNode);
}

void SeNetCSocketInit(struct SENETCSOCKET *pkNetCSocket)
{
	pkNetCSocket->iListCount = 0;
	SeListInit(&pkNetCSocket->kList);
}

void SeNetCSocketHeadAdd(struct SENETCSOCKET *pkNetCSocket, struct SECSOCKETNODE *pkNetCSocketNode)
{
	SeListHeadAdd(&pkNetCSocket->kList, &pkNetCSocketNode->kNode);
	pkNetCSocket->iListCount++;
}

void SeNetCSocketTailAdd(struct SENETCSOCKET *pkNetCSocket, struct SECSOCKETNODE *pkNetCSocketNode)
{
	SeListTailAdd(&pkNetCSocket->kList, &pkNetCSocketNode->kNode);
	pkNetCSocket->iListCount++;
}

struct SECSOCKETNODE *SeNetCSocketRemove(struct SENETCSOCKET *pkNetCSocket, struct SECSOCKETNODE *pkNetCSocketNode)
{
	struct SENODE *pkNode = 0;
	struct SECSOCKETNODE *pkNetCScoketNode = 0;

	pkNode = SeListRemove(&pkNetCSocket->kList, &pkNetCSocketNode->kNode);
	if(!pkNode) return 0;
	pkNetCScoketNode = SE_CONTAINING_RECORD(pkNode, struct SECSOCKETNODE, kNode);
	pkNetCSocket->iListCount--;
	return pkNetCScoketNode;
}


void SeNetSSocketNodeInit(struct SESSOCKETNODE *pkNetSSocketNode)
{
	pkNetSSocketNode->kListenSocket = SE_INVALID_SOCKET;
	SeListInitNode(&pkNetSSocketNode->kNode);
}

void SeNetSSocketInit(struct SENETSSOCKET *pkNetSSocket)
{
	pkNetSSocket->iListCount = 0;
	SeListInit(&pkNetSSocket->kList);
}

void SeNetSSocketAdd(struct SENETSSOCKET *pkNetSSocket, struct SESSOCKETNODE *pkNetSSocketNode)
{
	SeListHeadAdd(&pkNetSSocket->kList, &pkNetSSocketNode->kNode);
	pkNetSSocket->iListCount++;
}

struct SESSOCKETNODE *SeNetSSocketPop(struct SENETSSOCKET *pkNetSSocket)
{
	struct SENODE *pkNode = 0;
	struct SESSOCKETNODE *pkNetSScoketNode = 0;
	
	pkNode = SeListHeadPop(&pkNetSSocket->kList);
	if(!pkNode) return 0;
	pkNetSScoketNode = SE_CONTAINING_RECORD(pkNode, struct SESSOCKETNODE, kNode);
	pkNetSSocket->iListCount--;
	return pkNetSScoketNode;
}

