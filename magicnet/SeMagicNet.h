#ifndef __SE_MAGICNET_H__
#define __SE_MAGICNET_H__

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "SeNetCore.h"
#include "SeTime.h"

struct REGSVRNODE
{
	int						iSvrNo;
	unsigned long long		llActive;
	HSOCKET					kHSocket;
	struct SEHASHNODE		kHashNode;
};

struct SEMAGICNETS
{
	HSOCKET					kHScoketIn;
	HSOCKET					kHScoketOut;
	struct SENETCORE		kNetCore;
	struct SEHASH			kRegSvrList;
	char					*pcBuf;
};

#define SHUTDOWN_SVR -1
#define IDLE_SVR_DATA 0
#define CLIENT_CONNECT 1
#define CLIENT_DISCONNECT 2
#define RECV_DATA_FROM_SVR 3
#define RECV_DATA_FROM_CLIENT 4

bool SeMagicNetSInit(struct SEMAGICNETS *pkMagicNetS, unsigned short usMax, unsigned short usOutPort, unsigned short usInPort);

void SeMagicNetSFin(struct SEMAGICNETS *pkMagicNetS);

void SeMagicNetSProcess(struct SEMAGICNETS *pkMagicNetS);

#endif
