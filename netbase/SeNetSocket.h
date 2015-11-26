#ifndef __SE_NETSOCKET_H__
#define __SE_NETSOCKET_H__

#include "SeTime.h"
#include "SeList.h"
#include "SeTool.h"
#include "SeHash.h"
#include "SeNetBase.h"
#include "SeNetStream.h"

#define SOCKET_STATUS_INIT 0
#define SOCKET_STATUS_CONNECTING 1
#define SOCKET_STATUS_CONNECTED 2
#define SOCKET_STATUS_CONNECTED_FAILED 3
#define SOCKET_STATUS_DISCONNECT 4
#define SOCKET_STATUS_ACTIVECONNECT 5

#define LISTEN_TCP_TYPE_SOCKET 1
#define CLIENT_TCP_TYPE_SOCKET 2
#define ACCEPT_TCP_TYPE_SOCKET 3

#define READ_EVENT_SOCKET (1<<1)
#define WRITE_EVENT_SOCKET (1<<2)

struct SESOCKET
{
	HSOCKET					kHSocket;
	HSOCKET					kBelongListenHSocket;
	unsigned short			usStatus;
	unsigned short			usIndex;
	int						iHeaderLen;
	int						iTypeSocket;
	int						iEventSocket;
	unsigned long long		ullActive;
	struct SENETSTREAM		kSendNetStream;
	struct SENETSTREAM		kRecvNetStream;
	struct SEHASHNODE		kMainNode;
	struct SEHASHNODE		kSendNode;
	struct SEHASHNODE		kRecvNode;
	SEGETHEADERLENFUN		pkGetHeaderLenFun;
	SESETHEADERLENFUN		pkSetHeaderLenFun;
};

struct SESOCKETMGR
{
	int						iMax;
	int						iCounter;
	struct SENETSTREAM		kNetStreamIdle;
	struct SESOCKET			*pkSeSocket;
	struct SEHASH			kMainList;
	struct SEHASH			kActiveList;
	struct SEHASH			kSendList;
	struct SEHASH			kRecvList;
};

void SeNetSocketMgrInit(struct SESOCKETMGR *pkNetSocketMgr, unsigned short usMax);

void SeNetSocketMgrFin(struct SESOCKETMGR *pkNetSocketMgr);

HSOCKET SeNetSocketMgrAdd(struct SESOCKETMGR *pkNetSocketMgr, SOCKET socket, int iTypeSocket, int iHeaderLen, \
			SEGETHEADERLENFUN pkGetHeaderLenFun, SESETHEADERLENFUN pkSetHeaderLenFun);

void SeNetSocketMgrDel(struct SESOCKETMGR *pkNetSocketMgr, HSOCKET kHSocket);

struct SESOCKET *SeNetSocketMgrGet(struct SESOCKETMGR *pkNetSocketMgr, HSOCKET kHSocket);

void SeNetSocketMgrAddSendOrRecvInList(struct SESOCKETMGR *pkNetSocketMgr, struct SESOCKET *pkNetSocket, bool bSendOrRecv);

struct SESOCKET *SeNetSocketMgrPopSendOrRecvOutList(struct SESOCKETMGR *pkNetSocketMgr, bool bSendOrRecv);

void SeNetSocketMgrUpdateNetStreamIdle(struct SESOCKETMGR *pkNetSocketMgr, int iSize, int iBufLen);

bool SeNetSocketMgrHasEvent(struct SESOCKET *pkNetSocket, int iEventSocket);

void SeNetSocketMgrAddEvent(struct SESOCKET *pkNetSocket, int iEventSocket);

void SeNetSocketMgrClearEvent(struct SESOCKET *pkNetSocket, int iEventSocket);

#endif
