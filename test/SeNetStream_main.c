#include "SeNetStream.h"
#include <stdio.h>
#include "SeTool.h"

#define datalen  6

bool kSetHeaderLenFun(unsigned char* pcHeader, const int iheaderlen, const int ilen)
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

bool kGetHeaderLenFun(const unsigned char* pcHeader, const int iheaderlen, int *ilen)
{
	if (iheaderlen == 2)
	{
		unsigned short tmp = 0;
		memcpy(&tmp, pcHeader, iheaderlen);
		*ilen = tmp;
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
	int i;
	struct SENETSTREAM kNetStreamIdle;
	struct SENETSTREAM kNetStream;

	SeNetSreamInit(&kNetStreamIdle);
	SeNetSreamInit(&kNetStream);

	for (i = 0; i < 1000; i++)
	{
		int ilen = sizeof(struct SENETSTREAMNODE) + 4;
		char *pcbuf = (char*)SeMallocMem(ilen);
		struct SENETSTREAMNODE *pkNetStreamNode = SeNetSreamNodeFormat(pcbuf, ilen);
		SeNetSreamNodeZero(pkNetStreamNode);
		SeNetSreamHeadAdd(&kNetStreamIdle, pkNetStreamNode);
	}

	
	

	int iheaderlen = 0;
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
	ret = SeNetSreamRead(&kNetStream, &kNetStreamIdle, &kGetHeaderLenFun, iheaderlen, recvbuf, &irecvlen);
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

		if (!SeNetSreamCanRead(&kNetStream, &kGetHeaderLenFun, iheaderlen))
		{
			break;
		}

		memset(recvbuf, 0, sizeof(recvbuf));
		irecvlen = sizeof(recvbuf) - 1;
		ret = SeNetSreamRead(&kNetStream, &kNetStreamIdle, &kGetHeaderLenFun, iheaderlen, recvbuf, &irecvlen);
		printf("SeNetSreamRead ret=%d use_size=%d use_count=%d --> idle_size=%d idle_count=%d \n", ret, kNetStream.iSize, kNetStream.iCount, kNetStreamIdle.iSize, kNetStreamIdle.iCount);
		printf("data=[%s] %d\n", recvbuf, ret ? irecvlen : 0);
		
	}

	ret = SeNetSreamWrite(&kNetStream, &kNetStreamIdle, &kSetHeaderLenFun, iheaderlen, data, datalen - 1);
	printf("SeNetSreamWrite ret=%d use_size=%d use_count=%d --> idle_size=%d idle_count=%d \n", ret, kNetStream.iSize, kNetStream.iCount, kNetStreamIdle.iSize, kNetStreamIdle.iCount);

	memset(recvbuf, 0, sizeof(recvbuf));
	irecvlen = sizeof(recvbuf) - 1;
	ret = SeNetSreamRead(&kNetStream, &kNetStreamIdle, &kGetHeaderLenFun, iheaderlen, recvbuf, &irecvlen);
	printf("SeNetSreamRead ret=%d use_size=%d use_count=%d --> idle_size=%d idle_count=%d \n", ret, kNetStream.iSize, kNetStream.iCount, kNetStreamIdle.iSize, kNetStreamIdle.iCount);
	printf("data=%s %d\n", recvbuf, ret ? irecvlen : 0);
	while (ret)
	{
		if (!SeNetSreamCanRead(&kNetStream, &kGetHeaderLenFun, iheaderlen))
		{
			break;
		}
		
		memset(recvbuf, 0, sizeof(recvbuf));
		irecvlen = sizeof(recvbuf) - 1;
		ret = SeNetSreamRead(&kNetStream, &kNetStreamIdle, &kGetHeaderLenFun, iheaderlen, recvbuf, &irecvlen);
		printf("SeNetSreamRead ret=%d use_size=%d use_count=%d --> idle_size=%d idle_count=%d \n", ret, kNetStream.iSize, kNetStream.iCount, kNetStreamIdle.iSize, kNetStreamIdle.iCount);
		printf("dataaaaaaaaaaa=[%s] %d\n", recvbuf, ret ? irecvlen : -1);
	}
}

void test2()
{
	int i;
	struct SENETSTREAM kNetStreamIdle;
	SeNetSreamInit(&kNetStreamIdle);
	struct SENETSTREAM kNetStream;
	SeNetSreamInit(&kNetStream);

	for (i = 0; i < 1000; i++)
	{
		int ilen = sizeof(struct SENETSTREAMNODE) + 4;
		char *pcbuf = (char*)SeMallocMem(ilen);
		struct SENETSTREAMNODE *pkNetStreamNode = SeNetSreamNodeFormat(pcbuf, ilen);
		SeNetSreamNodeZero(pkNetStreamNode);
		SeNetSreamHeadAdd(&kNetStreamIdle, pkNetStreamNode);
	}

	int iheaderlen = 2;
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

	memset(recvbuf, 0, sizeof(recvbuf));
	irecvlen = sizeof(recvbuf) - 1;
	ret = SeNetSreamRead(&kNetStream, &kNetStreamIdle, &kGetHeaderLenFun, iheaderlen, recvbuf, &irecvlen);
	printf("SeNetSreamRead ret=%d use_size=%d use_count=%d --> idle_size=%d idle_count=%d \n", ret, kNetStream.iSize, kNetStream.iCount, kNetStreamIdle.iSize, kNetStreamIdle.iCount);
	printf("data=%s %d\n", recvbuf, ret ? irecvlen : 0);
	while (ret)
	{
		if (!SeNetSreamCanRead(&kNetStream, &kGetHeaderLenFun, iheaderlen))
		{
			break;
		}

		memset(recvbuf, 0, sizeof(recvbuf));
		irecvlen = sizeof(recvbuf) - 1;
		ret = SeNetSreamRead(&kNetStream, &kNetStreamIdle, &kGetHeaderLenFun, iheaderlen, recvbuf, &irecvlen);
		printf("SeNetSreamRead ret=%d use_size=%d use_count=%d --> idle_size=%d idle_count=%d \n", ret, kNetStream.iSize, kNetStream.iCount, kNetStreamIdle.iSize, kNetStreamIdle.iCount);
		printf("data=%s %d\n", recvbuf, ret ? irecvlen : 0);
	}


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
	ret = SeNetSreamRead(&kNetStream, &kNetStreamIdle, &kGetHeaderLenFun, iheaderlen, recvbuf, &irecvlen);
	printf("SeNetSreamRead ret=%d use_size=%d use_count=%d --> idle_size=%d idle_count=%d \n", ret, kNetStream.iSize, kNetStream.iCount, kNetStreamIdle.iSize, kNetStreamIdle.iCount);
	printf("data=%s %d\n", recvbuf, ret ? irecvlen : 0);
	while (ret)
	{
		if (!SeNetSreamCanRead(&kNetStream, &kGetHeaderLenFun, iheaderlen))
		{
			break;
		}

		memset(recvbuf, 0, sizeof(recvbuf));
		irecvlen = sizeof(recvbuf) - 1;
		ret = SeNetSreamRead(&kNetStream, &kNetStreamIdle, &kGetHeaderLenFun, iheaderlen, recvbuf, &irecvlen);
		printf("SeNetSreamRead ret=%d use_size=%d use_count=%d --> idle_size=%d idle_count=%d \n", ret, kNetStream.iSize, kNetStream.iCount, kNetStreamIdle.iSize, kNetStreamIdle.iCount);
		printf("data=%s %d\n", recvbuf, ret ? irecvlen : 0);
	}
}


struct SENETSTREAMNODE *NewStream()
{
	int MAX_BUF_LEN = 64;
	char *pcBuf = (char *)SeMallocMem(MAX_BUF_LEN);
	struct SENETSTREAMNODE *pkNetStreamNode = SeNetSreamNodeFormat(pcBuf, MAX_BUF_LEN);
	SeNetSreamNodeZero(pkNetStreamNode);
	//printf("---------------len=%d %d %d\n", pkNetStreamNode->usMaxLen, pkNetStreamNode->usReadPos, pkNetStreamNode->usWritePos);
	return pkNetStreamNode;
}

bool SetHeader(unsigned char* pcHeader, const int iheaderlen, const int ilen)
{
	// 小端
	/*if (iheaderlen == 2)
	{
	if(ilen < 0 || ilen > 0xFFFF) { return false; }
	pcHeader[0] = ilen & 0xFF;
	pcHeader[1] = (ilen >> 8) & 0xFF;
	return true;
	}
	*/

	// 大端
	if (iheaderlen == 2)
	{
		if (ilen < 0 || ilen > 0xFFFF) { return false; }
		pcHeader[0] = (ilen >> 8) & 0xff;
		pcHeader[1] = ilen & 0xff;
		return true;
	}

	// 小端
	if (iheaderlen == 4)
	{
		if (ilen < 0 || ilen > 1024 * 1024 * 3) { return false; }
		// 将int数值转换为占四个字节的byte数组，本方法适用于(低位在前，高位在后)的顺序。
		pcHeader[3] = ((ilen & 0xFF000000) >> 24);
		pcHeader[2] = ((ilen & 0x00FF0000) >> 16);
		pcHeader[1] = ((ilen & 0x0000FF00) >> 8);
		pcHeader[0] = ((ilen & 0x000000FF));
		return true;
	}

	return false;
}

bool GetHeader(const unsigned char* pcHeader, const int iheaderlen, int *ilen)
{
	// 小端
	/*if (iheaderlen == 2)
	{
	*ilen = (unsigned short)(pcHeader[1] << 8 | pcHeader[0]);
	if(*ilen < 0 || *ilen > 0xFFFF) { return false; }
	return true;
	}
	*/

	// 大端
	if (iheaderlen == 2)
	{
		*ilen = (unsigned short)(pcHeader[0] << 8 | pcHeader[1]);
		if (*ilen < 0 || *ilen > 0xFFFF) { return false; }
		return true;
	}

	// 小端
	if (iheaderlen == 4)
	{
		// byte数组中取int数值，本方法适用于(低位在前，高位在后)的顺序
		*ilen = (int)((pcHeader[0] & 0xFF) | ((pcHeader[1] << 8) & 0xFF00) | ((pcHeader[2] << 16) & 0xFF0000) | ((pcHeader[3] << 24) & 0xFF000000));
		if (*ilen < 0 || *ilen > 1024 * 1024 * 3) { return false; }
		return true;
	}

	return false;
}


void test3()
{
	static char acbuf[65535];
	struct SENETSTREAMNODE *pkNetStreamNode = 0;

	struct SENETSTREAM	kSendNetStream;
	SeNetSreamInit(&kSendNetStream);

	struct SENETSTREAM kNetStreamIdle;
	SeNetSreamInit(&kNetStreamIdle);
	for (int i = 0; i < 1000; i++)
	{
		pkNetStreamNode = NewStream();
		SeNetSreamHeadAdd(&kNetStreamIdle, pkNetStreamNode);
	}

	printf("--------- %d %d \n", kNetStreamIdle.iCount, kNetStreamIdle.iSize);

	while (SeNetSreamCanRead(&kSendNetStream, GetHeader, 2))
	{
		int iBufLen = 65535;
		bool ret = SeNetSreamRead(&kSendNetStream, &kNetStreamIdle, GetHeader, 2, acbuf, &iBufLen);
		printf("test recv %s %d\n", ret == true ? "true" : "false", iBufLen);
	}

	printf("TEST11111111 %d %d \n\n", kNetStreamIdle.iCount, kNetStreamIdle.iSize);

	SeNetSreamWrite(&kSendNetStream, &kNetStreamIdle, SetHeader, 2, acbuf, 29);
	printf("----------------------\n");
	SeNetSreamWrite(&kSendNetStream, &kNetStreamIdle, SetHeader, 2, acbuf, 31);
	printf("goAAAA %d %d \n", kSendNetStream.iCount, kSendNetStream.iSize);

	while (SeNetSreamCanRead(&kSendNetStream, GetHeader, 2))
	{
		int iBufLen = 65535;
		bool ret = SeNetSreamRead(&kSendNetStream, &kNetStreamIdle, GetHeader, 2, acbuf, &iBufLen);
		printf("recv %s %d\n", ret == true ? "true" : "false", iBufLen);
	}

	printf("EndAAAA %d %d \n\n", kNetStreamIdle.iCount, kNetStreamIdle.iSize);



	SeNetSreamWrite(&kSendNetStream, &kNetStreamIdle, SetHeader, 2, acbuf, 28);
	printf("----------------------\n");
	SeNetSreamWrite(&kSendNetStream, &kNetStreamIdle, SetHeader, 2, acbuf, 32);
	printf("goBBBB %d %d \n", kSendNetStream.iCount, kSendNetStream.iSize);

	while (SeNetSreamCanRead(&kSendNetStream, GetHeader, 2))
	{
		int iBufLen = 65535;
		bool ret = SeNetSreamRead(&kSendNetStream, &kNetStreamIdle, GetHeader, 2, acbuf, &iBufLen);
		printf("recv %s %d\n", ret == true ? "true" : "false", iBufLen);
	}

	printf("EndBBBBB %d %d \n\n", kNetStreamIdle.iCount, kNetStreamIdle.iSize);




	SeNetSreamWrite(&kSendNetStream, &kNetStreamIdle, SetHeader, 2, acbuf, 30);
	printf("----------------------\n");
	SeNetSreamWrite(&kSendNetStream, &kNetStreamIdle, SetHeader, 2, acbuf, 30);
	printf("goCCCCC %d %d \n", kSendNetStream.iCount, kSendNetStream.iSize);

	while (SeNetSreamCanRead(&kSendNetStream, GetHeader, 2))
	{
		int iBufLen = 65535;
		bool ret = SeNetSreamRead(&kSendNetStream, &kNetStreamIdle, GetHeader, 2, acbuf, &iBufLen);
		printf("recv %s %d\n", ret == true ? "true" : "false", iBufLen);
	}

	printf("EndCCC %d %d \n\n", kNetStreamIdle.iCount, kNetStreamIdle.iSize);



	SeNetSreamWrite(&kSendNetStream, &kNetStreamIdle, SetHeader, 2, acbuf, 14);
	printf("----------------------\n");
	SeNetSreamWrite(&kSendNetStream, &kNetStreamIdle, SetHeader, 2, acbuf, 13);
	printf("----------------------\n");
	SeNetSreamWrite(&kSendNetStream, &kNetStreamIdle, SetHeader, 2, acbuf, 31);
	printf("goDDDD %d %d \n", kSendNetStream.iCount, kSendNetStream.iSize);

	while (SeNetSreamCanRead(&kSendNetStream, GetHeader, 2))
	{
		int iBufLen = 65535;
		bool ret = SeNetSreamRead(&kSendNetStream, &kNetStreamIdle, GetHeader, 2, acbuf, &iBufLen);
		printf("recv %s %d\n", ret == true ? "true" : "false", iBufLen);
	}

	printf("EndDDDD %d %d \n\n", kNetStreamIdle.iCount, kNetStreamIdle.iSize);












	SeNetSreamWrite(&kSendNetStream, &kNetStreamIdle, SetHeader, 2, acbuf, 14);
	printf("----------------------\n");
	SeNetSreamWrite(&kSendNetStream, &kNetStreamIdle, SetHeader, 2, acbuf, 13);
	printf("----------------------\n");
	SeNetSreamWrite(&kSendNetStream, &kNetStreamIdle, SetHeader, 2, acbuf, 64 * 2 + 1);
	printf("goEEEEEE %d %d \n", kSendNetStream.iCount, kSendNetStream.iSize);

	while (SeNetSreamCanRead(&kSendNetStream, GetHeader, 2))
	{
		int iBufLen = 65535;
		bool ret = SeNetSreamRead(&kSendNetStream, &kNetStreamIdle, GetHeader, 2, acbuf, &iBufLen);
		printf("recv %s %d\n", ret == true ? "true" : "false", iBufLen);
	}

	printf("EndEEEEE %d %d \n\n", kNetStreamIdle.iCount, kNetStreamIdle.iSize);





	SeNetSreamWrite(&kSendNetStream, &kNetStreamIdle, SetHeader, 2, acbuf, 14);
	printf("goFFFFFF %d %d \n", kSendNetStream.iCount, kSendNetStream.iSize);

	while (SeNetSreamCanRead(&kSendNetStream, GetHeader, 2))
	{
		int iBufLen = 65535;
		bool ret = SeNetSreamRead(&kSendNetStream, &kNetStreamIdle, GetHeader, 2, acbuf, &iBufLen);
		printf("recv %s %d\n", ret == true ? "true" : "false", iBufLen);
	}

	printf("EndFFFFF %d %d \n\n", kNetStreamIdle.iCount, kNetStreamIdle.iSize);



	SeNetSreamWrite(&kSendNetStream, &kNetStreamIdle, SetHeader, 2, acbuf, 28);
	printf("----------------------\n");
	SeNetSreamWrite(&kSendNetStream, &kNetStreamIdle, SetHeader, 2, acbuf, 14);
	printf("goGGGGG %d %d \n", kSendNetStream.iCount, kSendNetStream.iSize);

	while (SeNetSreamCanRead(&kSendNetStream, GetHeader, 2))
	{
		int iBufLen = 65535;
		bool ret = SeNetSreamRead(&kSendNetStream, &kNetStreamIdle, GetHeader, 2, acbuf, &iBufLen);
		printf("recv %s %d\n", ret == true ? "true" : "false", iBufLen);
	}

	printf("EndGGGG %d %d \n\n", kNetStreamIdle.iCount, kNetStreamIdle.iSize);
}

int main()
{
	test1();
	printf("----------------------\n");
	test2();
	printf("----------------------\n");
	test3();
	printf("end\n");
	return getchar();
}
