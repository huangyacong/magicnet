#include "SeList.h"
#include <stdio.h>
#include "SeHash.h"
#include <stdbool.h>

#define SE_CONTAINING_RECORD(ptr, type, member) \
		((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

struct SOCKETNODE
{
	char i;
	struct SEHASHNODE hashnode;
};

void read(struct SEHASH *pkTest)
{
	int i;
	struct SENODE *tmp;
	struct SEHASHNODE *hashnode;
	struct SELIST *pkMain;
	struct SOCKETNODE *pkNetStreamNode;

	printf("BEGIN ...................\n");
	for (i = 0; i < pkTest->max; i++)
	{
		pkMain = &pkTest->pkMain[i];

		tmp = pkMain->head;
		while (tmp)
		{
			hashnode = SE_CONTAINING_RECORD(tmp, struct SEHASHNODE, main);
			pkNetStreamNode = SE_CONTAINING_RECORD(hashnode, struct SOCKETNODE, hashnode);
			printf("%d list ->head %d->%c \n", i, hashnode->id, pkNetStreamNode->i);
			tmp = tmp->next;
		}
	}

	tmp = pkTest->list.head;
	while (tmp)
	{
		hashnode = SE_CONTAINING_RECORD(tmp, struct SEHASHNODE, list);
		pkNetStreamNode = SE_CONTAINING_RECORD(hashnode, struct SOCKETNODE, hashnode);
		printf("all ->head %d->%c \n", hashnode->id, pkNetStreamNode->i);
		tmp = tmp->next;
	}
	printf("END   ...................\n");
}

int main()
{
	struct SEHASH kTest;
	struct SOCKETNODE kSocketNodeA;
	struct SOCKETNODE kSocketNodeB;
	struct SOCKETNODE kSocketNodeC;
	struct SOCKETNODE kSocketNodeD;
	struct SOCKETNODE kSocketNodeE;
	struct SOCKETNODE kSocketNodeF;

	int i;
	struct SEHASHNODE *pkRead;
	struct SOCKETNODE *pkNetStreamNode;

	SeHashNodeInit(&kSocketNodeD.hashnode);
	SeHashNodeInit(&kSocketNodeA.hashnode);
	SeHashNodeInit(&kSocketNodeE.hashnode);
	SeHashNodeInit(&kSocketNodeC.hashnode);
	SeHashNodeInit(&kSocketNodeB.hashnode);
	SeHashNodeInit(&kSocketNodeF.hashnode);

	kSocketNodeA.i = 'A';
	kSocketNodeB.i = 'B';
	kSocketNodeC.i = 'C';
	kSocketNodeD.i = 'D';
	kSocketNodeE.i = 'E';
	kSocketNodeF.i = 'F';

	SeHashInit(&kTest, 3);

	read(&kTest);

	pkRead = SeHashPop(&kTest);
	if (pkRead)
	{
		pkNetStreamNode = SE_CONTAINING_RECORD(pkRead, struct SOCKETNODE, hashnode);
		printf("pop %d->%c \n", pkRead->id, pkNetStreamNode->i);
	}

	SeHashAdd(&kTest, 0, &kSocketNodeA.hashnode);
	SeHashAdd(&kTest, 2, &kSocketNodeC.hashnode);
	SeHashAdd(&kTest, 1, &kSocketNodeB.hashnode);

	pkRead = SeHashPop(&kTest);
	if (pkRead)
	{
		pkNetStreamNode = SE_CONTAINING_RECORD(pkRead, struct SOCKETNODE, hashnode);
		printf("pop %d->%c \n", pkRead->id, pkNetStreamNode->i);
	}

	printf("go-----------------------\n");
	for (i = 0; i < 6; i++)
	{
		pkRead = SeHashGet(&kTest, i);
		if (pkRead)
		{
			pkNetStreamNode = SE_CONTAINING_RECORD(pkRead, struct SOCKETNODE, hashnode);
			printf("%d->%c \n", i, pkNetStreamNode->i);
		}
	}
	printf("oo-----------------------\n");


	SeHashAdd(&kTest, 4, &kSocketNodeE.hashnode);
	SeHashAdd(&kTest, 3, &kSocketNodeD.hashnode);
	SeHashAdd(&kTest, 5, &kSocketNodeF.hashnode);

	printf("go-----------------------\n");
	for (i = 0; i < 6; i++)
	{
		pkRead = SeHashGet(&kTest, i);
		if (pkRead)
		{
			pkNetStreamNode = SE_CONTAINING_RECORD(pkRead, struct SOCKETNODE, hashnode);
			printf("%d->%c \n", i, pkNetStreamNode->i);
		}
	}
	printf("oo-----------------------\n");

	read(&kTest);

	pkRead = SeHashPop(&kTest);
	while (pkRead)
	{
		pkNetStreamNode = SE_CONTAINING_RECORD(pkRead, struct SOCKETNODE, hashnode);
		printf("pop %d->%c \n", pkRead->id, pkNetStreamNode->i);
		pkRead = SeHashPop(&kTest);
	}

	printf("go-----------------------\n");
	for (i = 0; i < 6; i++)
	{
		pkRead = SeHashGet(&kTest, i);
		if (pkRead)
		{
			pkNetStreamNode = SE_CONTAINING_RECORD(pkRead, struct SOCKETNODE, hashnode);
			printf("%d->%c \n", i, pkNetStreamNode->i);
		}
	}
	printf("oo-----------------------\n");

	read(&kTest);

	SeHashAdd(&kTest, 0, &kSocketNodeA.hashnode);
	SeHashAdd(&kTest, 2, &kSocketNodeC.hashnode);
	SeHashAdd(&kTest, 1, &kSocketNodeB.hashnode);
	SeHashAdd(&kTest, 4, &kSocketNodeE.hashnode);
	SeHashAdd(&kTest, 3, &kSocketNodeD.hashnode);
	SeHashAdd(&kTest, 5, &kSocketNodeF.hashnode);

	printf("go-----------------------\n");
	for (i = 0; i < 6; i++)
	{
		pkRead = SeHashGet(&kTest, i);
		if (pkRead)
		{
			pkNetStreamNode = SE_CONTAINING_RECORD(pkRead, struct SOCKETNODE, hashnode);
			printf("%d->%c \n", i, pkNetStreamNode->i);
		}
	}
	printf("oo-----------------------\n");

	read(&kTest);




	pkRead = SeHashGet(&kTest, 0);
	SeHashRemove(&kTest, pkRead);
	printf("remove \n");

	pkRead = SeHashGet(&kTest, 3);
	SeHashRemove(&kTest, pkRead);
	printf("remove \n");

	pkRead = SeHashGet(&kTest, 5);
	SeHashRemove(&kTest, pkRead);
	printf("remove \n");

	pkRead = SeHashGet(&kTest, 1);
	SeHashRemove(&kTest, pkRead);
	printf("remove \n");
	pkRead = SeHashGet(&kTest, 4);
	SeHashRemove(&kTest, pkRead);
	printf("remove \n");
	pkRead = SeHashGet(&kTest, 2);
	SeHashRemove(&kTest, pkRead);
	printf("remove \n");



	printf("go-----------------------\n");
	for (i = 0; i < 6; i++)
	{
		pkRead = SeHashGet(&kTest, i);
		if (pkRead)
		{
			pkNetStreamNode = SE_CONTAINING_RECORD(pkRead, struct SOCKETNODE, hashnode);
			printf("%d->%c \n", i, pkNetStreamNode->i);
		}
	}
	printf("oo-----------------------\n");

	read(&kTest);

	getchar();
	SeHashFin(&kTest);
	return 0;
}