#include "SeList.h"
#include <stdio.h>
#include "SeHSocket.h"
#include <stdbool.h>

int main()
{
	bool ret;
	HSOCKET rkHSocket;
	struct SEHSOCKET kTest;

	SeHSocketInit(&kTest, 3);

	ret = SeHSocketGetHead(&kTest, &rkHSocket);
	printf("ret=%d \n", ret);
	ret = SeHSocketPop(&kTest, &rkHSocket);
	printf("ret=%d \n", ret);
	SeHSocketMoveToEnd(&kTest, rkHSocket);




	rkHSocket = SeGetHSocket(12, 0, 12121);
	ret = SeHSocketAdd(&kTest, rkHSocket);
	printf("add ret=%d \n", ret);

	rkHSocket = SeGetHSocket(324, 0, 54645654);
	ret = SeHSocketAdd(&kTest, rkHSocket);
	printf("add ret=%d \n", ret);

	rkHSocket = SeGetHSocket(7876, 1, 4543534);
	ret = SeHSocketAdd(&kTest, rkHSocket);
	printf("add ret=%d \n", ret);

	rkHSocket = SeGetHSocket(5345, 2, 9879789);
	ret = SeHSocketAdd(&kTest, rkHSocket);
	printf("add ret=%d \n", ret);

	rkHSocket = SeGetHSocket(4345, 3, 987944789);
	ret = SeHSocketAdd(&kTest, rkHSocket);
	printf("add ret=%d \n", ret);


	rkHSocket = SeGetHSocket(12, 0, 12121);
	ret = SeHSocketGet(&kTest, rkHSocket);
	printf("get ret=%d %d \n", ret, rkHSocket == SeGetHSocket(12, 0, 12121));

	rkHSocket = SeGetHSocket(0, 0, 0);
	ret = SeHSocketGet(&kTest, rkHSocket);
	printf("get ret=%d \n", ret);

	ret = SeHSocketGetHead(&kTest, &rkHSocket);
	printf("get head ret=%d \n", ret);

	rkHSocket = SeGetHSocket(12, 0, 12121);
	SeHSocketMoveToEnd(&kTest, rkHSocket);

	rkHSocket = SeGetHSocket(324, 0, 54645654);
	SeHSocketMoveToEnd(&kTest, rkHSocket);

	rkHSocket = SeGetHSocket(5345, 2, 9879789);
	SeHSocketMoveToEnd(&kTest, rkHSocket);

	SeHSocketRemove(&kTest, 0);

	rkHSocket = SeGetHSocket(12, 4, 12121);
	ret = SeHSocketAdd(&kTest, rkHSocket);
	printf("add ret=%d \n", ret);

	rkHSocket = SeGetHSocket(12, 5, 12121);
	ret = SeHSocketAdd(&kTest, rkHSocket);
	printf("add ret=%d \n", ret);


	ret = SeHSocketPop(&kTest, &rkHSocket);
	printf("pop ret=%d \n", ret);

	ret = SeHSocketPop(&kTest, &rkHSocket);
	printf("pop ret=%d \n", ret);

	ret = SeHSocketPop(&kTest, &rkHSocket);
	printf("pop ret=%d \n", ret);

	ret = SeHSocketPop(&kTest, &rkHSocket);
	printf("pop ret=%d \n", ret);

	ret = SeHSocketGet(&kTest, rkHSocket);
	printf("get ret=%d \n", ret);

	SeHSocketFin(&kTest);
	return getchar();
}