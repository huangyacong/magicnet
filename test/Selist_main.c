#include "SeList.h"
#include <stdio.h>
#include "SeList.h"
#include "SeBool.h"


struct SENETSTREAMNODE
{
	struct SENODE		kNode;
	char				*pkBuf;
	int					iMaxLen;
	int					iReadPos;
	int					iWritePos;
	int					iFlag;
};
struct test
{
	struct SELIST KLIST;
};

struct testnode
{
	struct SENODE node;
	int i;
};
#define SE_CONTAINING_RECORD(ptr, type, member) \
		((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

void test1()
{
	struct test kTest;
	SeListInit(&kTest.KLIST);

	struct testnode nodeA;
	SeListInitNode(&nodeA.node);
	nodeA.i = 1;
	SeListHeadAdd(&kTest.KLIST, &nodeA.node);

	struct testnode nodeB;
	SeListInitNode(&nodeB.node);
	nodeB.i = 2;
	SeListHeadAdd(&kTest.KLIST, &nodeB.node);

	struct testnode nodeC;
	SeListInitNode(&nodeC.node);
	nodeC.i = 3;
	SeListHeadAdd(&kTest.KLIST, &nodeC.node);

	struct SENODE *tmp = kTest.KLIST.head;
	while (tmp)
	{
		struct testnode *pkNetStreamNode;
		pkNetStreamNode = SE_CONTAINING_RECORD(tmp, struct testnode, node);
		printf("head %d \n", pkNetStreamNode->i);
		tmp = tmp->next;
	}

	tmp = kTest.KLIST.tail;
	while (tmp)
	{
		struct testnode *pkNetStreamNode;
		pkNetStreamNode = SE_CONTAINING_RECORD(tmp, struct testnode, node);
		printf("tail %d \n", pkNetStreamNode->i);
		tmp = tmp->prev;
	}
}

void test2()
{
	struct test kTest;
	SeListInit(&kTest.KLIST);

	struct testnode nodeA;
	SeListInitNode(&nodeA.node);
	nodeA.i = 1;
	SeListTailAdd(&kTest.KLIST, &nodeA.node);

	struct testnode nodeB;
	SeListInitNode(&nodeB.node);
	nodeB.i = 2;
	SeListTailAdd(&kTest.KLIST, &nodeB.node);

	struct testnode nodeC;
	SeListInitNode(&nodeC.node);
	nodeC.i = 3;
	SeListTailAdd(&kTest.KLIST, &nodeC.node);

	struct SENODE *tmp = kTest.KLIST.head;
	while (tmp)
	{
		struct testnode *pkNetStreamNode;
		pkNetStreamNode = SE_CONTAINING_RECORD(tmp, struct testnode, node);
		printf("head %d \n", pkNetStreamNode->i);
		tmp = tmp->next;
	}

	tmp = kTest.KLIST.tail;
	while (tmp)
	{
		struct testnode *pkNetStreamNode;
		pkNetStreamNode = SE_CONTAINING_RECORD(tmp, struct testnode, node);
		printf("tail %d \n", pkNetStreamNode->i);
		tmp = tmp->prev;
	}
}

void test3()
{
	struct test kTest;
	SeListInit(&kTest.KLIST);

	struct testnode nodeA;
	SeListInitNode(&nodeA.node);
	nodeA.i = 1;
	SeListHeadAdd(&kTest.KLIST, &nodeA.node);
	
	struct testnode nodeB;
	SeListInitNode(&nodeB.node);
	nodeB.i = 2;
	SeListTailAdd(&kTest.KLIST, &nodeB.node);

	struct testnode nodeC;
	SeListInitNode(&nodeC.node);
	nodeC.i = 3;
	SeListHeadAdd(&kTest.KLIST, &nodeC.node);
	
	struct SENODE *tmp = kTest.KLIST.head;
	while (tmp)
	{
		struct testnode *pkNetStreamNode;
		pkNetStreamNode = SE_CONTAINING_RECORD(tmp, struct testnode, node);
		printf("head %d \n", pkNetStreamNode->i);
		tmp = tmp->next;
	}

	tmp = kTest.KLIST.tail;
	while (tmp)
	{
		struct testnode *pkNetStreamNode;
		pkNetStreamNode = SE_CONTAINING_RECORD(tmp, struct testnode, node);
		printf("tail %d \n", pkNetStreamNode->i);
		tmp = tmp->prev;
	}
}

void test4()
{
	struct test kTest;
	SeListInit(&kTest.KLIST);
	
	struct testnode nodeA;
	SeListInitNode(&nodeA.node);
	nodeA.i = 1;
	SeListHeadAdd(&kTest.KLIST, &nodeA.node);

	struct testnode nodeB;
	SeListInitNode(&nodeB.node);
	nodeB.i = 2;
	SeListTailAdd(&kTest.KLIST, &nodeB.node);
	
	struct testnode nodeC;
	SeListInitNode(&nodeC.node);
	nodeC.i = 3;
	SeListHeadAdd(&kTest.KLIST, &nodeC.node);
	
	//SeListRemove(&kTest.KLIST, &nodeB.node);
	//SeListRemove(&kTest.KLIST, &nodeA.node);
	//SeListRemove(&kTest.KLIST, &nodeC.node);
	struct SENODE *tmpb = SeListRemoveEnd(&kTest.KLIST, &nodeC.node);
	struct SENODE *tmpa = tmpb;
	while (tmpa)
	{
		struct testnode *pkNetStreamNode;
		pkNetStreamNode = SE_CONTAINING_RECORD(tmpa, struct testnode, node);
		printf("head--- %d \n", pkNetStreamNode->i);
		tmpa = tmpa->next;
	}
	tmpa = tmpb;
	while (tmpa)
	{
		struct testnode *pkNetStreamNode;
		pkNetStreamNode = SE_CONTAINING_RECORD(tmpa, struct testnode, node);
		printf("tail----- %d \n", pkNetStreamNode->i);
		tmpa = tmpa->prev;
	}
	
	SeListHeadAddList(&kTest.KLIST, tmpb);
	//SeListHeadPop();
	/*
	while (tmpb)
	{
		tmpa = tmpb;
		tmpb = tmpb->next;
		SeListHeadAdd(&kTest.KLIST, tmpa);
		
	}
	*/


	struct SENODE *tmp = kTest.KLIST.head;
	while (tmp)
	{
		struct testnode *pkNetStreamNode;
		pkNetStreamNode = SE_CONTAINING_RECORD(tmp, struct testnode, node);
		printf("head %d \n", pkNetStreamNode->i);
		tmp = tmp->next;
	}

	tmp = kTest.KLIST.tail;
	while (tmp)
	{
		struct testnode *pkNetStreamNode;
		pkNetStreamNode = SE_CONTAINING_RECORD(tmp, struct testnode, node);
		printf("tail %d \n", pkNetStreamNode->i);
		tmp = tmp->prev;
	}
}

int main()
{
	test4();
	printf("end %d %d %d\n", sizeof(struct SENETSTREAMNODE), sizeof(struct SENODE), sizeof(char*));
	return getchar();
}