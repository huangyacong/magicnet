#include "SeNetStream.h"
#include <stdio.h>

bool kSetHeaderLenFun(char* pcHeader, const int iheaderlen, const int ilen)
{
	if (iheaderlen == 2)
	{
		unsigned short tmp = (unsigned short)ilen;
		memcpy(pcHeader, &tmp, iheaderlen);
		printf("%d %d \n", pcHeader[0], pcHeader[1]);
		return true;
	}
	if (iheaderlen == 0)
	{
		return true;
	}
	return false;
}

bool kGetHeaderLenFun(const char* pcHeader, const int iheaderlen, int &ilen)
{
	if (iheaderlen == 2)
	{
		unsigned short tmp = 0;
		memcpy(&tmp, pcHeader, iheaderlen);
		ilen = tmp;
		return true;
	}
	if (iheaderlen == 0)
	{
		ilen = 0;
		return true;
	}
	return false;
}

void test1()
{
	struct SENETSTREAM kNetStreamIdle;
	SeNetSreamInit(&kNetStreamIdle);

	for (int i = 0; i < 1000; i++)
	{
		int ilen = sizeof(struct SENETSTREAMNODE) + 4;
		char *pcbuf = (char*)malloc(ilen);
		struct SENETSTREAMNODE *pkNetStreamNode = SeNetSreamNodeFormat(pcbuf, ilen);
		SeNetSreamNodeZero(pkNetStreamNode);
		SeNetSreamHeadAdd(&kNetStreamIdle, pkNetStreamNode);
	}

	struct SENETSTREAM kNetStream;
	SeNetSreamInit(&kNetStream);

	int iheaderlen = 0;
	const int datalen = 6;
	char data[datalen] = { 0 };
	data[0] = 'a';
	data[1] = 'b';
	data[2] = 'c';
	data[3] = 'd';
	data[4] = 'e';
	printf("%s\n", data);
	
	char recvbuf[200] = {0};
	int irecvlen = sizeof(recvbuf) - 1;

	printf("INIT..............  use_size=%d use_count=%d --> idle_size=%d idle_count=%d \n",  kNetStream.iSize, kNetStream.iCount, kNetStreamIdle.iSize, kNetStreamIdle.iCount);

	bool ret = SeNetSreamWrite(&kNetStream, &kNetStreamIdle, &kSetHeaderLenFun, iheaderlen, data, datalen - 1);
	printf("SeNetSreamWrite ret=%d use_size=%d use_count=%d --> idle_size=%d idle_count=%d \n", ret, kNetStream.iSize, kNetStream.iCount, kNetStreamIdle.iSize, kNetStreamIdle.iCount);

	ret = SeNetSreamWrite(&kNetStream, &kNetStreamIdle, &kSetHeaderLenFun, iheaderlen, data, datalen - 1);
	printf("SeNetSreamWrite ret=%d use_size=%d use_count=%d --> idle_size=%d idle_count=%d \n", ret, kNetStream.iSize, kNetStream.iCount, kNetStreamIdle.iSize, kNetStreamIdle.iCount);

	
	int test = 0;
	memset(recvbuf, 0, sizeof(recvbuf));
	irecvlen = sizeof(recvbuf) - 1;
	ret = SeNetSreamRead(&kNetStream, &kNetStreamIdle, &kGetHeaderLenFun, iheaderlen, recvbuf, irecvlen);
	printf("SeNetSreamRead ret=%d use_size=%d use_count=%d --> idle_size=%d idle_count=%d \n", ret, kNetStream.iSize, kNetStream.iCount, kNetStreamIdle.iSize, kNetStreamIdle.iCount);
	printf("data=%s %d\n", recvbuf, ret ? irecvlen:0);
	while (ret)
	{
		if (test == 0)
		{
			test++;
			SeNetSreamWrite(&kNetStream, &kNetStreamIdle, &kSetHeaderLenFun, iheaderlen, data, datalen - 2);
			printf("SeNetSreamWrite ret=%d use_size=%d use_count=%d --> idle_size=%d idle_count=%d \n", ret, kNetStream.iSize, kNetStream.iCount, kNetStreamIdle.iSize, kNetStreamIdle.iCount);
		}

		memset(recvbuf, 0, sizeof(recvbuf));
		irecvlen = sizeof(recvbuf) - 1;
		ret = SeNetSreamRead(&kNetStream, &kNetStreamIdle, &kGetHeaderLenFun, iheaderlen, recvbuf, irecvlen);
		printf("SeNetSreamRead ret=%d use_size=%d use_count=%d --> idle_size=%d idle_count=%d \n", ret, kNetStream.iSize, kNetStream.iCount, kNetStreamIdle.iSize, kNetStreamIdle.iCount);
		printf("data=%s %d\n", recvbuf, ret ? irecvlen : 0);
		
	}

	ret = SeNetSreamWrite(&kNetStream, &kNetStreamIdle, &kSetHeaderLenFun, iheaderlen, data, datalen - 1);
	printf("SeNetSreamWrite ret=%d use_size=%d use_count=%d --> idle_size=%d idle_count=%d \n", ret, kNetStream.iSize, kNetStream.iCount, kNetStreamIdle.iSize, kNetStreamIdle.iCount);

	memset(recvbuf, 0, sizeof(recvbuf));
	irecvlen = sizeof(recvbuf) - 1;
	ret = SeNetSreamRead(&kNetStream, &kNetStreamIdle, &kGetHeaderLenFun, iheaderlen, recvbuf, irecvlen);
	printf("SeNetSreamRead ret=%d use_size=%d use_count=%d --> idle_size=%d idle_count=%d \n", ret, kNetStream.iSize, kNetStream.iCount, kNetStreamIdle.iSize, kNetStreamIdle.iCount);
	printf("data=%s %d\n", recvbuf, ret ? irecvlen : 0);
	while (ret)
	{
		
		memset(recvbuf, 0, sizeof(recvbuf));
		irecvlen = sizeof(recvbuf) - 1;
		ret = SeNetSreamRead(&kNetStream, &kNetStreamIdle, &kGetHeaderLenFun, iheaderlen, recvbuf, irecvlen);
		printf("SeNetSreamRead ret=%d use_size=%d use_count=%d --> idle_size=%d idle_count=%d \n", ret, kNetStream.iSize, kNetStream.iCount, kNetStreamIdle.iSize, kNetStreamIdle.iCount);
		printf("data=%s %d\n", recvbuf, ret ? irecvlen : 0);
	}
}

void test2()
{
	struct SENETSTREAM kNetStreamIdle;
	SeNetSreamInit(&kNetStreamIdle);
	struct SENETSTREAM kNetStream;
	SeNetSreamInit(&kNetStream);

	for (int i = 0; i < 1000; i++)
	{
		int ilen = sizeof(struct SENETSTREAMNODE) + 4;
		char *pcbuf = (char*)malloc(ilen);
		struct SENETSTREAMNODE *pkNetStreamNode = SeNetSreamNodeFormat(pcbuf, ilen);
		SeNetSreamNodeZero(pkNetStreamNode);
		SeNetSreamHeadAdd(&kNetStreamIdle, pkNetStreamNode);
	}

	int iheaderlen = 2;
	const int datalen = 6;
	char data[datalen] = { 0 };
	data[0] = 'a';
	data[1] = 'b';
	data[2] = 'c';
	data[3] = 'd';
	data[4] = 'e';
	printf("%s\n", data);

	char recvbuf[200] = { 0 };
	int irecvlen = sizeof(recvbuf) - 1;

	printf("INIT..............  use_size=%d use_count=%d --> idle_size=%d idle_count=%d \n", kNetStream.iSize, kNetStream.iCount, kNetStreamIdle.iSize, kNetStreamIdle.iCount);

	bool ret = SeNetSreamWrite(&kNetStream, &kNetStreamIdle, &kSetHeaderLenFun, iheaderlen, data, 0);
	printf("SeNetSreamWrite ret=%d use_size=%d use_count=%d --> idle_size=%d idle_count=%d \n", ret, kNetStream.iSize, kNetStream.iCount, kNetStreamIdle.iSize, kNetStreamIdle.iCount);

	ret = SeNetSreamWrite(&kNetStream, &kNetStreamIdle, &kSetHeaderLenFun, iheaderlen, data, 1);
	printf("SeNetSreamWrite ret=%d use_size=%d use_count=%d --> idle_size=%d idle_count=%d \n", ret, kNetStream.iSize, kNetStream.iCount, kNetStreamIdle.iSize, kNetStreamIdle.iCount);
	
	ret = SeNetSreamWrite(&kNetStream, &kNetStreamIdle, &kSetHeaderLenFun, iheaderlen, data, 6);
	printf("SeNetSreamWrite ret=%d use_size=%d use_count=%d --> idle_size=%d idle_count=%d \n", ret, kNetStream.iSize, kNetStream.iCount, kNetStreamIdle.iSize, kNetStreamIdle.iCount);
	
	ret = SeNetSreamWrite(&kNetStream, &kNetStreamIdle, &kSetHeaderLenFun, iheaderlen, data, 3);
	printf("SeNetSreamWrite ret=%d use_size=%d use_count=%d --> idle_size=%d idle_count=%d \n", ret, kNetStream.iSize, kNetStream.iCount, kNetStreamIdle.iSize, kNetStreamIdle.iCount);

	ret = SeNetSreamWrite(&kNetStream, &kNetStreamIdle, &kSetHeaderLenFun, iheaderlen, data, 0);
	printf("SeNetSreamWrite ret=%d use_size=%d use_count=%d --> idle_size=%d idle_count=%d \n", ret, kNetStream.iSize, kNetStream.iCount, kNetStreamIdle.iSize, kNetStreamIdle.iCount);

	ret = SeNetSreamWrite(&kNetStream, &kNetStreamIdle, &kSetHeaderLenFun, iheaderlen, data, 1);
	printf("SeNetSreamWrite ret=%d use_size=%d use_count=%d --> idle_size=%d idle_count=%d \n", ret, kNetStream.iSize, kNetStream.iCount, kNetStreamIdle.iSize, kNetStreamIdle.iCount);
	
	char a[2] = { 0 };
	a[0] = 0;
	a[1] = 0;

	ret = SeNetSreamWrite(&kNetStream, &kNetStreamIdle, &kSetHeaderLenFun, 0, a, 2);
	printf("SeNetSreamWrite ret=%d use_size=%d use_count=%d --> idle_size=%d idle_count=%d \n", ret, kNetStream.iSize, kNetStream.iCount, kNetStreamIdle.iSize, kNetStreamIdle.iCount);
	
	a[0] = 0;
	ret = SeNetSreamWrite(&kNetStream, &kNetStreamIdle, &kSetHeaderLenFun, 0, a, 1);
	printf("SeNetSreamWrite ret=%d use_size=%d use_count=%d --> idle_size=%d idle_count=%d \n", ret, kNetStream.iSize, kNetStream.iCount, kNetStreamIdle.iSize, kNetStreamIdle.iCount);
	
	a[0] = 0;
	ret = SeNetSreamWrite(&kNetStream, &kNetStreamIdle, &kSetHeaderLenFun, 0, a, 1);
	printf("SeNetSreamWrite ret=%d use_size=%d use_count=%d --> idle_size=%d idle_count=%d \n", ret, kNetStream.iSize, kNetStream.iCount, kNetStreamIdle.iSize, kNetStreamIdle.iCount);

	a[0] = 1;
	ret = SeNetSreamWrite(&kNetStream, &kNetStreamIdle, &kSetHeaderLenFun, 0, a, 1);
	printf("SeNetSreamWrite ret=%d use_size=%d use_count=%d --> idle_size=%d idle_count=%d \n", ret, kNetStream.iSize, kNetStream.iCount, kNetStreamIdle.iSize, kNetStreamIdle.iCount);

	a[0] = 0;
	ret = SeNetSreamWrite(&kNetStream, &kNetStreamIdle, &kSetHeaderLenFun, 0, a, 1);
	printf("SeNetSreamWrite ret=%d use_size=%d use_count=%d --> idle_size=%d idle_count=%d \n", ret, kNetStream.iSize, kNetStream.iCount, kNetStreamIdle.iSize, kNetStreamIdle.iCount);

	a[0] = 'k';
	ret = SeNetSreamWrite(&kNetStream, &kNetStreamIdle, &kSetHeaderLenFun, 0, a, 1);
	printf("SeNetSreamWrite ret=%d use_size=%d use_count=%d --> idle_size=%d idle_count=%d \n", ret, kNetStream.iSize, kNetStream.iCount, kNetStreamIdle.iSize, kNetStreamIdle.iCount);
	

	ret = SeNetSreamWrite(&kNetStream, &kNetStreamIdle, &kSetHeaderLenFun, iheaderlen, data, 1);
	printf("SeNetSreamWrite ret=%d use_size=%d use_count=%d --> idle_size=%d idle_count=%d \n", ret, kNetStream.iSize, kNetStream.iCount, kNetStreamIdle.iSize, kNetStreamIdle.iCount);

	memset(recvbuf, 0, sizeof(recvbuf));
	irecvlen = sizeof(recvbuf) - 1;
	ret = SeNetSreamRead(&kNetStream, &kNetStreamIdle, &kGetHeaderLenFun, iheaderlen, recvbuf, irecvlen);
	printf("SeNetSreamRead ret=%d use_size=%d use_count=%d --> idle_size=%d idle_count=%d \n", ret, kNetStream.iSize, kNetStream.iCount, kNetStreamIdle.iSize, kNetStreamIdle.iCount);
	printf("data=%s %d\n", recvbuf, ret ? irecvlen : 0);
	while (ret)
	{

		memset(recvbuf, 0, sizeof(recvbuf));
		irecvlen = sizeof(recvbuf) - 1;
		ret = SeNetSreamRead(&kNetStream, &kNetStreamIdle, &kGetHeaderLenFun, iheaderlen, recvbuf, irecvlen);
		printf("SeNetSreamRead ret=%d use_size=%d use_count=%d --> idle_size=%d idle_count=%d \n", ret, kNetStream.iSize, kNetStream.iCount, kNetStreamIdle.iSize, kNetStreamIdle.iCount);
		printf("data=%s %d\n", recvbuf, ret ? irecvlen : 0);
	}
}

int main()
{
	test2();
	return getchar();
}
