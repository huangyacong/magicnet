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

	SeMagicNetCInit(&kTest, "game.log", 1000*30, 8887);
	SeMagicNetCReg(&kTest, "game");

	while (1)
	{
		//printf("aaaaaaaaaaa\n");
		pcBuf = 0;
		state = SeMagicNetCRead(&kTest, &rkRecvHSocket, &pcBuf, &riBufLen);
		if (state == MAGIC_SHUTDOWN_SVR) { printf("gate close\n"); break; }
		if (state == MAGIC_IDLE_SVR_DATA) { continue; }

		if (state == MAGIC_CLIENT_CONNECT)
		{
			printf("Client  connect! %llu\n", rkRecvHSocket);
		}

		if (state == MAGIC_CLIENT_DISCONNECT)
		{
			printf("Client  disconnect! %llu\n", rkRecvHSocket);
		}

		if (state == MAGIC_RECV_DATA_FROM_CLIENT)
		{
			printf("recv from client! %llu %d\n", rkRecvHSocket, riBufLen);
		}

		if (state == MAGIC_RECV_DATA_FROM_SVR)
		{
			assert(sizeof(acbug) > riBufLen);
			memset(acbug, 0, sizeof(acbug));
			memcpy(acbug, pcBuf, riBufLen);
			printf("recv from svr! %llu %s %d\n", rkRecvHSocket, acbug, riBufLen);
		}
	}

	SeMagicNetCFin(&kTest);
	return 0;
}