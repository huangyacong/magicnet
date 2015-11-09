#include "SeList.h"
#include <stdio.h>
#include "SeHash.h"
#include <stdbool.h>
#include "SeNetSocket.h"

bool get(const char*a, const int b, int* c)
{
	return false;
}

bool set(char *a, const int b, const int c)
{
	return false;
}

int main()
{
	HSOCKET kHSocket;
	struct SESOCKET *pkNetSocketa, *pkNetSocketb, *pkNetSocketc, *pkNetSocketd;
	struct SESOCKETMGR kTest;

	SeNetSocketMgrInit(&kTest, 3);

	pkNetSocketa = SeNetSocketMgrAdd(&kTest, 0, 1, 0, get, set);
	pkNetSocketb = SeNetSocketMgrAdd(&kTest, 0, 1, 0, get, set);
	pkNetSocketc = SeNetSocketMgrAdd(&kTest, 0, 1, 0, get, set);
	pkNetSocketd = SeNetSocketMgrAdd(&kTest, 0, 1, 0, get, set);
	assert(pkNetSocketa && pkNetSocketb && pkNetSocketc);
	if (!pkNetSocketd) printf("SeNetSocketMgrAdd failed pkNetSocketd\n");

	kHSocket = pkNetSocketa->kHSocket;
	pkNetSocketd = SeNetSocketMgrGet(&kTest, kHSocket);
	assert(pkNetSocketd);

	kHSocket = pkNetSocketb->kHSocket;
	pkNetSocketd = SeNetSocketMgrGet(&kTest, kHSocket);
	assert(pkNetSocketd);

	kHSocket = pkNetSocketc->kHSocket;
	pkNetSocketd = SeNetSocketMgrGet(&kTest, kHSocket);
	assert(pkNetSocketd);

	SeNetSocketMgrAddSendOrRecvInList(&kTest, pkNetSocketa, true);
	SeNetSocketMgrAddSendOrRecvInList(&kTest, pkNetSocketa, true);
	SeNetSocketMgrAddSendOrRecvInList(&kTest, pkNetSocketa, false);
	SeNetSocketMgrAddSendOrRecvInList(&kTest, pkNetSocketa, false);

	pkNetSocketd = SeNetSocketMgrPopSendOrRecvOutList(&kTest, true);
	if (!pkNetSocketd) printf("SeNetSocketMgrPopSendOrRecvOutList true failed \n");
	pkNetSocketd = SeNetSocketMgrPopSendOrRecvOutList(&kTest, true);
	if (!pkNetSocketd) printf("SeNetSocketMgrPopSendOrRecvOutList true failed \n");

	pkNetSocketd = SeNetSocketMgrPopSendOrRecvOutList(&kTest, false);
	if (!pkNetSocketd) printf("SeNetSocketMgrPopSendOrRecvOutList false failed \n");
	pkNetSocketd = SeNetSocketMgrPopSendOrRecvOutList(&kTest, false);
	if (!pkNetSocketd) printf("SeNetSocketMgrPopSendOrRecvOutList false failed \n");

	SeNetSocketMgrUpdateNetStreamIdle(&kTest, 1024);
	SeNetSocketMgrUpdateNetStreamIdle(&kTest, 1024);
	SeNetSocketMgrUpdateNetStreamIdle(&kTest, 1024);
	SeNetSocketMgrUpdateNetStreamIdle(&kTest, 1024);

	SeNetSocketMgrDel(&kTest, pkNetSocketa);
	SeNetSocketMgrDel(&kTest, pkNetSocketb);
	SeNetSocketMgrDel(&kTest, pkNetSocketc);
	//SeNetSocketMgrDel(&kTest, pkNetSocketa);

	pkNetSocketd = SeNetSocketMgrPopSendOrRecvOutList(&kTest, true);
	if (!pkNetSocketd) printf("SeNetSocketMgrPopSendOrRecvOutList true failed \n");
	pkNetSocketd = SeNetSocketMgrPopSendOrRecvOutList(&kTest, true);
	if (!pkNetSocketd) printf("SeNetSocketMgrPopSendOrRecvOutList true failed \n");

	pkNetSocketd = SeNetSocketMgrPopSendOrRecvOutList(&kTest, false);
	if (!pkNetSocketd) printf("SeNetSocketMgrPopSendOrRecvOutList false failed \n");
	pkNetSocketd = SeNetSocketMgrPopSendOrRecvOutList(&kTest, false);
	if (!pkNetSocketd) printf("SeNetSocketMgrPopSendOrRecvOutList false failed \n");

	pkNetSocketd = SeNetSocketMgrGet(&kTest, pkNetSocketb->kHSocket);
	if (!pkNetSocketd) printf("SeNetSocketMgrGet failed %lld \n", pkNetSocketb->kHSocket);
	else printf("SeNetSocketMgrGet ok %lld  %lld \n", pkNetSocketd->kHSocket, pkNetSocketb->kHSocket);

	getchar();
	SeNetSocketMgrFin(&kTest);
	return 0;
}