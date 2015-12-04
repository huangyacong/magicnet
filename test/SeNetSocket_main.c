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
	HSOCKET kHScoket;
	struct SESOCKET *pkSocket;
	HSOCKET kHSocketA, kHSocketB, kHSocketC;
	struct SESOCKETMGR kTest;

	SeNetSocketMgrInit(&kTest, 3, 3);

	
	kHSocketA = SeNetSocketMgrAdd(&kTest, 1, 1, 0, get, set);
	kHSocketB = SeNetSocketMgrAdd(&kTest, 2, 1, 0, get, set);
	kHSocketC = SeNetSocketMgrAdd(&kTest, 3, 1, 0, get, set);
	assert(kHSocketA > 0 && kHSocketB > 0 && kHSocketC > 0);

	kHScoket = SeNetSocketMgrAdd(&kTest, 4, 1, 0, get, set);
	if (kHScoket <= 0) printf("SeNetSocketMgrAdd failed 4\n");
	
	pkSocket = SeNetSocketMgrGet(&kTest, kHSocketA);
	if (!pkSocket) printf("SeNetSocketMgrGet failed kHSocketA \n");
	pkSocket = SeNetSocketMgrGet(&kTest, kHSocketB);
	if (!pkSocket) printf("SeNetSocketMgrGet failed kHSocketB \n");
	pkSocket = SeNetSocketMgrGet(&kTest, kHSocketC);
	if (!pkSocket) printf("SeNetSocketMgrGet failed kHSocketC \n");
	pkSocket = SeNetSocketMgrGet(&kTest, kHScoket);
	if (!pkSocket) printf("SeNetSocketMgrGet failed kHScoket \n");

	SeNetSocketMgrDel(&kTest, kHSocketA);
	SeNetSocketMgrDel(&kTest, kHSocketB);
	SeNetSocketMgrDel(&kTest, kHSocketC);
	SeNetSocketMgrDel(&kTest, kHScoket);

	pkSocket = SeNetSocketMgrGet(&kTest, kHSocketA);
	if (!pkSocket) printf("SeNetSocketMgrGet failed kHSocketA \n");
	pkSocket = SeNetSocketMgrGet(&kTest, kHSocketB);
	if (!pkSocket) printf("SeNetSocketMgrGet failed kHSocketB \n");
	pkSocket = SeNetSocketMgrGet(&kTest, kHSocketC);
	if (!pkSocket) printf("SeNetSocketMgrGet failed kHSocketC \n");
	pkSocket = SeNetSocketMgrGet(&kTest, kHScoket);
	if (!pkSocket) printf("SeNetSocketMgrGet failed kHScoket \n");

	kHSocketA = SeNetSocketMgrAdd(&kTest, 1, 1, 0, get, set);
	kHSocketB = SeNetSocketMgrAdd(&kTest, 2, 1, 0, get, set);
	kHSocketC = SeNetSocketMgrAdd(&kTest, 3, 1, 0, get, set);
	assert(kHSocketA > 0 && kHSocketB > 0 && kHSocketC > 0);

	pkSocket = SeNetSocketMgrPopSendOrRecvOutList(&kTest, true);
	while (pkSocket)
	{
		printf("SeNetSocketMgrPopSendOrRecvOutList true ok\n");
		pkSocket = SeNetSocketMgrPopSendOrRecvOutList(&kTest, true);
	}

	pkSocket = SeNetSocketMgrGet(&kTest, kHSocketA);
	SeNetSocketMgrAddSendOrRecvInList(&kTest, pkSocket, true);
	SeNetSocketMgrAddSendOrRecvInList(&kTest, pkSocket, true);
	SeNetSocketMgrAddSendOrRecvInList(&kTest, pkSocket, false);
	SeNetSocketMgrAddSendOrRecvInList(&kTest, pkSocket, false);

	pkSocket = SeNetSocketMgrPopSendOrRecvOutList(&kTest, true);
	while (pkSocket)
	{
		printf("SeNetSocketMgrPopSendOrRecvOutList true ok\n");
		pkSocket = SeNetSocketMgrPopSendOrRecvOutList(&kTest, true);
	}

	pkSocket = SeNetSocketMgrPopSendOrRecvOutList(&kTest, false);
	while (pkSocket)
	{
		printf("SeNetSocketMgrPopSendOrRecvOutList false ok\n");
		pkSocket = SeNetSocketMgrPopSendOrRecvOutList(&kTest, false);
	}

	pkSocket = SeNetSocketMgrGet(&kTest, kHSocketB);
	SeNetSocketMgrAddSendOrRecvInList(&kTest, pkSocket, false);

	pkSocket = SeNetSocketMgrPopSendOrRecvOutList(&kTest, true);
	while (pkSocket)
	{
		printf("SeNetSocketMgrPopSendOrRecvOutList true ok\n");
		pkSocket = SeNetSocketMgrPopSendOrRecvOutList(&kTest, true);
	}

	pkSocket = SeNetSocketMgrGet(&kTest, kHSocketA);
	if (!pkSocket) printf("SeNetSocketMgrGet failed kHSocketA \n");
	pkSocket = SeNetSocketMgrGet(&kTest, kHSocketB);
	if (!pkSocket) printf("SeNetSocketMgrGet failed kHSocketB \n");
	pkSocket = SeNetSocketMgrGet(&kTest, kHSocketC);
	if (!pkSocket) printf("SeNetSocketMgrGet failed kHSocketC \n");
	pkSocket = SeNetSocketMgrGet(&kTest, kHScoket);
	if (!pkSocket) printf("SeNetSocketMgrGet failed kHScoket \n");

	//pkSocket = SeNetSocketMgrGet(&kTest, kHScoket);
	//SeNetSocketMgrAddSendOrRecvInList(&kTest, pkSocket, true);

	SeNetSocketMgrUpdateNetStreamIdle(&kTest, 1024, 1024 * 1024 * 4);
	
	getchar();
	SeNetSocketMgrFin(&kTest);
	return 0;
}