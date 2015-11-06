#include "SeNetSocket.h"

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

void SeNetSocketReset(struct SESOCKET *pkNetSocket)
{
	pkNetSocket->kHSocket = SeGetHSocket(0, 0, SE_INVALID_SOCKET);
	pkNetSocket->usStatus = SOCKET_STATUS_INIT;
	pkNetSocket->iFlag = 0;
}
