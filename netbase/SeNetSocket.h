#ifndef __SE_NETSOCKET_H__
#define __SE_NETSOCKET_H__

#include "SeList.h"
#include "SeTool.h"
#include "SeHash.h"
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
	unsigned short			usStatus;
	unsigned short			usIndex;
	int						iFlag;
	struct SENETSTREAM		kSendNetStream;
	struct SENETSTREAM		kRecvNetStream;
	struct SENODE			kMainNode;
	struct SEHASHNODE		kSendNode;
	struct SEHASHNODE		kRecvNode;
};

struct SESOCKETMGR
{
	int						iMax;
	struct SENETSTREAM		kNetStreamIdle;
	struct SESOCKET			*pkSeSocket;
	struct SELIST			kMainList;
	struct SEHASH			kSendList;
	struct SEHASH			kRecvList;
};

void SeNetSocketMgrInit(struct SESOCKETMGR *pkNetSocketMgr, unsigned short usIndex);

void SeNetSocketMgrFin(struct SESOCKETMGR *pkNetSocketMgr);


#endif
