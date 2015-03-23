#ifndef __SE_NETSOCKET_H__
#define __SE_NETSOCKET_H__

#include "SeList.h"
#include "SeNetBase.h"
#include "SeNetStream.h"

#define CSOCKET_STATUS_INIT 0
#define CSOCKET_STATUS_CONNECT 1
#define CSOCKET_STATUS_DISCONNECT 2
#define CSOCKET_STATUS_ACTIVECONNECT 3

struct SESSOCKETNODE
{
	SOCKET				kListenSocket;
	int					iProtoFormat;
	long long			llMemSize;
	struct SENETSTREAM	kMemSCache;
	struct SENODE		kNode;
};

struct SECSOCKETNODE
{
	HSOCKET					kHSocket;
	struct SESSOCKETNODE*	pkBelongToSvr;
	int						iStatus;
	int						iEvent;
	int						iProtoFormat;
	int						iFlag;
	struct SENETSTREAM		kSendNetStream;
	struct SENETSTREAM		kRecvNetStream;
	struct SENODE			kNode;
};


//--------------------------------------------------------------------------------------------------
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


//--------------------------------------------------------------------------------------------------
struct SENETSSOCKET
{
	long long			llListCount;
	struct SELIST		kList;
};

void SeNetSSocketNodeInit(struct SESSOCKETNODE *pkNetSSocketNode);

void SeNetSSocketNodeFin(struct SESSOCKETNODE *pkNetSSocketNode, struct SENETSTREAM	*pkMemCache);

void SeNetSSocketInit(struct SENETSSOCKET *pkNetSSocket);

void SeNetSSocketAdd(struct SENETSSOCKET *pkNetSSocket, struct SESSOCKETNODE *pkNetSSocketNode);

struct SESSOCKETNODE *SeNetSSocketPop(struct SENETSSOCKET *pkNetSSocket);

#endif
