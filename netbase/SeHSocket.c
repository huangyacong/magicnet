#include "SeHSocket.h"

void SeHSocketInit(struct SEHSOCKET *root, int max)
{
	int i;

	SeHashInit(&root->kMainList, max);
	SeListInit(&root->kIdleList);
	root->pkHSocketNode = (struct SEHSOCKETNODE*)SeMallocMem(sizeof(struct SEHSOCKETNODE)*max);
	for(i = 0; i < max; i++)
	{
		SeHashNodeInit(&(root->pkHSocketNode[i].kHashNode));
		SeListInitNode(&(root->pkHSocketNode[i].kListNode));
		root->pkHSocketNode[i].kHSocket = 0;
		SeListHeadAdd(&root->kIdleList, &(root->pkHSocketNode[i].kListNode));
	}
}

void SeHSocketFin(struct SEHSOCKET *root)
{
	SeHashFin(&root->kMainList);
	SeFreeMem(root->pkHSocketNode);
}

bool SeHSocketAdd(struct SEHSOCKET *root, HSOCKET kHSocket)
{
	struct SENODE *pkNode;
	unsigned short usIndex;
	struct SEHSOCKETNODE *pkHSocketNode;

	usIndex = SeGetIndexByHScoket(kHSocket);
	if(SeHashGet(&root->kMainList, usIndex)) return false;
	pkNode = SeListTailPop(&root->kIdleList);
	if(!pkNode) return false;
	pkHSocketNode = SE_CONTAINING_RECORD(pkNode, struct SEHSOCKETNODE, kListNode);
	pkHSocketNode->kHSocket = kHSocket;
	SeHashAdd(&root->kMainList, usIndex, &pkHSocketNode->kHashNode);
	return true;
}

bool SeHSocketGet(struct SEHSOCKET *root, HSOCKET kHSocket)
{
	unsigned short usIndex;
	struct SEHASHNODE *pkHashNode;
	struct SEHSOCKETNODE *pkHSocketNode;

	usIndex = SeGetIndexByHScoket(kHSocket);
	pkHashNode = SeHashGet(&root->kMainList, usIndex);
	if(!pkHashNode) return false;
	pkHSocketNode = SE_CONTAINING_RECORD(pkHashNode, struct SEHSOCKETNODE, kHashNode);
	if(pkHSocketNode->kHSocket != kHSocket) return false;
	return true;
}

bool SeHSocketGetHead(struct SEHSOCKET *root, HSOCKET *rkHSocket)
{
	struct SEHASHNODE *pkHashNode;
	struct SEHSOCKETNODE *pkHSocketNode;

	pkHashNode = SeHashGetHead(&root->kMainList);
	if(!pkHashNode) return false;
	pkHSocketNode = SE_CONTAINING_RECORD(pkHashNode, struct SEHSOCKETNODE, kHashNode);
	*rkHSocket = pkHSocketNode->kHSocket;
	return true;
}

void SeHSocketRemove(struct SEHSOCKET *root, HSOCKET kHSocket)
{
	unsigned short usIndex;
	struct SEHASHNODE *pkHashNode;
	struct SEHSOCKETNODE *pkHSocketNode;

	usIndex = SeGetIndexByHScoket(kHSocket);
	pkHashNode = SeHashGet(&root->kMainList, usIndex);
	if(!pkHashNode) return;
	SeHashRemove(&root->kMainList, pkHashNode);
	pkHSocketNode = SE_CONTAINING_RECORD(pkHashNode, struct SEHSOCKETNODE, kHashNode);
	SeListHeadAdd(&root->kIdleList, &pkHSocketNode->kListNode);
}

bool SeHSocketPop(struct SEHSOCKET *root, HSOCKET *rkHSocket)
{
	struct SEHASHNODE *pkHashNode;
	struct SEHSOCKETNODE *pkHSocketNode;

	pkHashNode = SeHashPop(&root->kMainList);
	if(!pkHashNode) return false;
	pkHSocketNode = SE_CONTAINING_RECORD(pkHashNode, struct SEHSOCKETNODE, kHashNode);
	SeListHeadAdd(&root->kIdleList, &pkHSocketNode->kListNode);
	*rkHSocket = pkHSocketNode->kHSocket;
	return true;
}

void SeHSocketMoveToEnd(struct SEHSOCKET *root, HSOCKET kHSocket)
{
	unsigned short usIndex;
	struct SEHASHNODE *pkHashNode;
	struct SEHSOCKETNODE *pkHSocketNode;

	usIndex = SeGetIndexByHScoket(kHSocket);
	pkHashNode = SeHashGet(&root->kMainList, usIndex);
	if(!pkHashNode) return;
	pkHSocketNode = SE_CONTAINING_RECORD(pkHashNode, struct SEHSOCKETNODE, kHashNode);
	if(pkHSocketNode->kHSocket != kHSocket) return;
	SeHashMoveToEnd(&root->kMainList, pkHashNode);
}
