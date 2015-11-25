#include "SeList.h"
#include <stdio.h>
#include "SeHash.h"
#include "SeTime.h"
#include <stdbool.h>
#include "SeNetCore.h"

bool kSetHeaderLenFun(char* pcHeader, const int iheaderlen, const int ilen)
{
	char head[2];

	if (iheaderlen == 2)
	{
		head[0] = (ilen >> 8) & 0xff;
		head[1] = ilen & 0xff;
		memcpy(pcHeader, &head, iheaderlen);
		return true;
	}

	if (iheaderlen == 0)
	{
		return true;
	}

	return false;
}

bool kGetHeaderLenFun(const char* pcHeader, const int iheaderlen, int *ilen)
{
	if (iheaderlen == 2)
	{
		*ilen = pcHeader[0] << 8 | pcHeader[1];
		return true;
	}

	if (iheaderlen == 0)
	{
		ilen = 0;
		return true;
	}

	return false;
}

int main()
{
	bool ret;
	int iLen;
	int riEvent;
	HSOCKET khsocket;
	HSOCKET rkHSocket;
	char *buf = (char*)malloc(1024*1024*4);
	HSOCKET rkListenHSocket;
	struct SENETCORE kNet;

	SeNetCoreInit(&kNet, "out.txt", 1000);
	khsocket = SeNetCoreTCPListen(&kNet, "0.0.0.0", 8888, 2, kGetHeaderLenFun, kSetHeaderLenFun);
	
	while(khsocket != 0)
	{
		iLen = 1024*1024*4;
		ret = SeNetCoreRead(&kNet, &riEvent, &rkListenHSocket, &rkHSocket, buf, &iLen);
		if(!ret) { continue; }

		if(riEvent == SENETCORE_EVENT_SOCKET_IDLE)
		{
			SeTimeSleep(1);
			continue;
		}

		if(riEvent == SENETCORE_EVENT_SOCKET_CONNECT)
		{
			printf("connect! %lld\n", khsocket);
		}

		if(riEvent == SENETCORE_EVENT_SOCKET_DISCONNECT)
		{
			printf("disconnect! %lld\n", khsocket);
		}

		if(riEvent == SENETCORE_EVENT_SOCKET_RECV_DATA)
		{
			printf("%s\n", buf);
			SeNetCoreSend(&kNet, rkHSocket, buf, iLen);
		}
		printf(".......\n");
	}
	printf("end\n");
	getchar();
	free(buf);
	SeNetCoreFin(&kNet);
	return 0;
}