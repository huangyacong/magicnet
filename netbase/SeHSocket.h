#ifndef __SE_HSOCKET_H__
#define __SE_HSOCKET_H__

#include "SeTime.h"
#include "SeList.h"
#include "SeTool.h"
#include "SeHash.h"
#include "SeNetBase.h"

struct SEHSOCKETNODE
{
	struct SEHASHNODE		kHashNode;
	struct SENODE			kListNode;
	HSOCKET					kHSocket;
};

struct SEHSOCKET
{
	struct SEHASH			kMainList;
	struct SELIST			kIdleList;
	struct SEHSOCKETNODE	*pkHSocketNode;
	long long				llFlag;
};

void SeHSocketInit(struct SEHSOCKET *root, int max);

void SeHSocketFin(struct SEHSOCKET *root);


bool SeHSocketAdd(struct SEHSOCKET *root, HSOCKET kHSocket);

bool SeHSocketGet(struct SEHSOCKET *root, HSOCKET kHSocket);

bool SeHSocketGetHead(struct SEHSOCKET *root, HSOCKET *rkHSocket);

void SeHSocketRemove(struct SEHSOCKET *root, HSOCKET kHSocket);

bool SeHSocketPop(struct SEHSOCKET *root, HSOCKET *rkHSocket);

void SeHSocketMoveToEnd(struct SEHSOCKET *root, HSOCKET kHSocket);

#endif
