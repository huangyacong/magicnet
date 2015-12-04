#include "SeList.h"
#include <stdio.h>
#include "SeHash.h"
#include "SeTime.h"
#include <stdbool.h>
#include "SeMagicNet.h"


int main()
{
	char acbug[1024];
	char *pcBuf;
	int riBufLen;
	enum MAGIC_STATE state;
	HSOCKET rkRecvHSocket;
	struct SEMAGICNETC kTest;

	SeMagicNetCInit(&kTest, "watchdog.log", 1000 * 30, 8887);
	SeMagicNetCReg(&kTest, "watchdog.");

	while (1)
	{
		pcBuf = 0;
		state = SeMagicNetCRead(&kTest, &rkRecvHSocket, &pcBuf, &riBufLen);
		if (state == MAGIC_SHUTDOWN_SVR) { printf("gate close\n"); break; }
		if (state == MAGIC_IDLE_SVR_DATA) { continue; }

		if (state == MAGIC_CLIENT_CONNECT)
		{
			memset(acbug, 0, sizeof(acbug));
			memcpy(acbug, pcBuf, riBufLen);
			printf("Client  connect! %llu %s %d\n", rkRecvHSocket, acbug, riBufLen);
			//SeMagicNetCCloseClient(&kTest, rkRecvHSocket);
		}

		if (state == MAGIC_CLIENT_DISCONNECT)
		{
			printf("Client  disconnect! %llu %d\n", rkRecvHSocket, riBufLen);
		}

		if (state == MAGIC_RECV_DATA_FROM_CLIENT)
		{
			memset(acbug, 0, sizeof(acbug));
			memcpy(acbug, pcBuf, riBufLen);
			printf("recv from client! %llu %s\n", rkRecvHSocket, acbug);
			SeMagicNetCSendSvr(&kTest, "game", acbug, riBufLen);
			SeMagicNetCSendClient(&kTest, rkRecvHSocket, acbug, riBufLen);
		}

		if (state == MAGIC_RECV_DATA_FROM_SVR)
		{
			memset(acbug, 0, sizeof(acbug));
			memcpy(acbug, pcBuf, riBufLen);
			printf("recv from svr! %llu %s %d\n", rkRecvHSocket, acbug, riBufLen);
			
		}
	}

	SeMagicNetCFin(&kTest);
	return 0;
}