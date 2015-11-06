#ifndef __SE_NETSOCKET_H__
#define __SE_NETSOCKET_H__

#include "SeList.h"
#include "SeTool.h"
#include "SeNetBase.h"
#include "SeNetStream.h"

#define SOCKET_STATUS_INIT 0
#define SOCKET_STATUS_ACCEPT 1
#define SOCKET_STATUS_CONNECTING 2
#define SOCKET_STATUS_CONNECTED 3
#define SOCKET_STATUS_DISCONNECT 4
#define SOCKET_STATUS_ACTIVECONNECT 5

struct SESOCKET
{
	HSOCKET					kHSocket;
	int						iStatus;
	int						iFlag;
	struct SENETSTREAM		kSendNetStream;
	struct SENETSTREAM		kRecvNetStream;
	struct SENODE			kMainNode;
	struct SENODE			kSendNode;
	struct SENODE			kRecvNode;
};

void SeNetSocketInit(struct SESOCKET *pkNetSocket);

void SeNetSocketReset(struct SESOCKET *pkNetSocket);

struct SESOCKET *SeNetSocketMainHeadPop(struct SELIST *pkSeList);

void SeNetSocketMainTailAdd(struct SELIST *pkSeList, struct SESOCKET *pkSeSocket);

struct SESOCKET *SeNetSocketSendHeadPop(struct SELIST *pkSeList);

void SeNetSocketSendTailAdd(struct SELIST *pkSeList, struct SESOCKET *pkSeSocket);

struct SESOCKET *SeNetSocketRecvHeadPop(struct SELIST *pkSeList);

void SeNetSocketRecvTailAdd(struct SELIST *pkSeList, struct SESOCKET *pkSeSocket);

#endif
