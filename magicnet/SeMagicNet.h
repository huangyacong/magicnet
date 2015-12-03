#ifndef __SE_MAGICNET_H__
#define __SE_MAGICNET_H__

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "SeNetCore.h"
#include "SeTime.h"

struct SEMAGICNETS
{
	HSOCKET					kHScoketIn;
	HSOCKET					kHScoketOut;
	struct SENETCORE		kNetCore;
	struct SELIST			kRegSvrList;
	char					*pcRecvBuf;
};

bool SeMagicNetSInit(struct SEMAGICNETS *pkMagicNetS, unsigned short usMax, unsigned short usOutPort, unsigned short usInPort);

void SeMagicNetSFin(struct SEMAGICNETS *pkMagicNetS);

void SeMagicNetSProcess(struct SEMAGICNETS *pkMagicNetS);

#define SHUTDOWN_SVR -1
#define IDLE_SVR_DATA 0
#define CLIENT_CONNECT 1
#define CLIENT_DISCONNECT 2
#define RECV_DATA_FROM_SVR 3
#define RECV_DATA_FROM_CLIENT 4

char acWatchdogName[] = "watchdog.";

struct SEMAGICNETC
{
	HSOCKET					kHScoket;
	unsigned long long		llActive;
	struct SENETCORE		kNetCore;
	char					*pcRecvBuf;
};

bool SeMagicNetCInit(struct SEMAGICNETC *pkMagicNetC, unsigned short usInPort);

void SeMagicNetCFin(struct SEMAGICNETC *pkMagicNetC);

bool SeMagicNetCReg(struct SEMAGICNETC *pkMagicNetC, const char *pcSvrName);

bool SeMagicNetCSendClient(struct SEMAGICNETC *pkMagicNetC, HSOCKET kHSocket, const char *pcBuf, int iLen);

bool SeMagicNetCSendSvr(struct SEMAGICNETC *pkMagicNetC, const char *pcSvrName, const char *pcBuf, int iLen);

#endif
