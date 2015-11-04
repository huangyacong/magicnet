#include "SeNetSocket.h"

void SeNetSocketInit(struct SESOCKET *pkNetSocket)
{
	SeNetSocketReset(pkNetSocket);
	SeNetSreamInit(&pkNetSocket->kSendNetStream);
	SeNetSreamInit(&pkNetSocket->kRecvNetStream);
	SeListInitNode(&pkNetSocket->kMainNode);
	SeListInitNode(&pkNetSocket->kSendNode);
	SeListInitNode(&pkNetSocket->kRecvNode);
}

void SeNetSocketReset(struct SESOCKET *pkNetSocket)
{
	pkNetSocket->kHSocket = SeGetHSocket(0, 0, SE_INVALID_SOCKET);
	pkNetSocket->iStatus = SOCKET_STATUS_INIT;
	pkNetSocket->iFlag = 0;
}

struct SESOCKET *SeNetSocketMainHeadPop(struct SELIST *pkSeList)
{
	struct SENODE *pkNode;
	struct SESOCKET *pkSeSocket;

	pkNode = SeListHeadPop(pkSeList);
	if(!pkNode) return 0;
	pkSeSocket = SE_CONTAINING_RECORD(pkNode, struct SESOCKET, kMainNode);
	return pkSeSocket;
}

void SeNetSocketMainTailAdd(struct SELIST *pkSeList, struct SESOCKET *pkSeSocket)
{
	SeListTailAdd(pkSeList, &pkSeSocket->kMainNode);
}

struct SESOCKET *SeNetSocketSendHeadPop(struct SELIST *pkSeList)
{
	struct SENODE *pkNode;
	struct SESOCKET *pkSeSocket;

	pkNode = SeListHeadPop(pkSeList);
	if(!pkNode) return 0;
	pkSeSocket = SE_CONTAINING_RECORD(pkNode, struct SESOCKET, kSendNode);
	return pkSeSocket;
}

void SeNetSocketSendTailAdd(struct SELIST *pkSeList, struct SESOCKET *pkSeSocket)
{
	SeListTailAdd(pkSeList, &pkSeSocket->kSendNode);
}

struct SESOCKET *SeNetSocketRecvHeadPop(struct SELIST *pkSeList)
{
	struct SENODE *pkNode;
	struct SESOCKET *pkSeSocket;

	pkNode = SeListHeadPop(pkSeList);
	if(!pkNode) return 0;
	pkSeSocket = SE_CONTAINING_RECORD(pkNode, struct SESOCKET, kRecvNode);
	return pkSeSocket;
}

void SeNetSocketRecvTailAdd(struct SELIST *pkSeList, struct SESOCKET *pkSeSocket)
{
	SeListTailAdd(pkSeList, &pkSeSocket->kRecvNode);
}
