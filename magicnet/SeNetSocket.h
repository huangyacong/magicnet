#ifndef __SE_NETSOCKET_H__
#define __SE_NETSOCKET_H__

#include "SeList.h"
#include "SeNetBase.h"
#include "SeNetStream.h"

#define CSOCKET_STATUS_INIT 0
#define CSOCKET_STATUS_ACCEPT 1
#define CSOCKET_STATUS_CONNECTING 2
#define CSOCKET_STATUS_CONNECTED 3
#define CSOCKET_STATUS_DISCONNECT 4
#define CSOCKET_STATUS_ACTIVECONNECT 5

union SOCKET_EVENT
{
	int						iEvent;
	void					*ptr;
};

struct SECSOCKETNODE
{
	HSOCKET					kHSocket;
	SOCKET					kSvrSocket;
	int						iStatus;
	int						iProtoFormat;
	int						iFlag;
	long long				llMemSize;
	union SOCKET_EVENT		ukEvent;
	struct SENETSTREAM		kSendNetStream;
	struct SENETSTREAM		kRecvNetStream;
	struct SENODE			kNode;
};

struct SENETCSOCKET
{
	long long			llListCount;
	struct SELIST		kList;
};

void SeNetCSocketNodeInit(struct SECSOCKETNODE *pkNetCSocketNode);

void SeNetCSocketNodeFin(struct SECSOCKETNODE *pkNetCSocketNode, struct SENETSTREAM	*pkMemCache);

void SeNetCSocketInit(struct SENETCSOCKET *pkNetCSocket);

struct SECSOCKETNODE *SeNetCSocketPop(struct SENETCSOCKET *pkNetCSocket);

void SeNetCSocketHeadAdd(struct SENETCSOCKET *pkNetCSocket, struct SECSOCKETNODE *pkNetCSocketNode);

void SeNetCSocketTailAdd(struct SENETCSOCKET *pkNetCSocket, struct SECSOCKETNODE *pkNetCSocketNode);

struct SECSOCKETNODE *SeNetCSocketRemove(struct SENETCSOCKET *pkNetCSocket, struct SECSOCKETNODE *pkNetCSocketNode);

#endif
