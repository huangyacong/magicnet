#ifndef __SE_MAGICNET_H__
#define __SE_MAGICNET_H__

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "SeNetCore.h"

struct REGSVRNODE
{
	int						iSvrNo;
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

struct SEMAGICNETC
{
	int						iSvrNo;
	struct SENETCORE		kNetCore;
};

bool SeMagicNetSInit(struct SEMAGICNETS *pkMagicNetS, unsigned short usMax, unsigned short usOutPort, unsigned short usInPort);

void SeMagicNetSFin(struct SEMAGICNETS *pkMagicNetS);

void SeMagicNetSProcess(struct SEMAGICNETS *pkMagicNetS);

#endif
