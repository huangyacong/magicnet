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

#define CLIENT_CONNECT 0
#define CLIENT_DISCONNECT 1
#define RECV_DATA_FROM_SVR 2
#define RECV_DATA_FROM_CLIENT 3

bool SeMagicNetSInit(struct SEMAGICNETS *pkMagicNetS, unsigned short usMax, unsigned short usOutPort, unsigned short usInPort);

void SeMagicNetSFin(struct SEMAGICNETS *pkMagicNetS);

void SeMagicNetSProcess(struct SEMAGICNETS *pkMagicNetS);

#endif
