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
	char					*pcSendBuf;
};

bool SeMagicNetSInit(struct SEMAGICNETS *pkMagicNetS, char *pcLogName, int iTimeOut, unsigned short usMax, unsigned short usOutPort, unsigned short usInPort);

void SeMagicNetSFin(struct SEMAGICNETS *pkMagicNetS);

void SeMagicNetSSetWaitTime(struct SEMAGICNETS *pkMagicNetS, unsigned int uiWaitTime);

void SeMagicNetSProcess(struct SEMAGICNETS *pkMagicNetS);

enum MAGIC_STATE
{
	MAGIC_SHUTDOWN_SVR = -1,
	MAGIC_IDLE_SVR_DATA,
	MAGIC_CLIENT_CONNECT,
	MAGIC_CLIENT_DISCONNECT,
	MAGIC_RECV_DATA_FROM_SVR,
	MAGIC_RECV_DATA_FROM_CLIENT,
};

struct SEMAGICNETC
{
	HSOCKET					kHScoket;
	unsigned long long		llActive;
	struct SENETCORE		kNetCore;
	unsigned short			usInPort;
	char					*pcRecvBuf;
	char					*pcSendBuf;
};

bool SeMagicNetCInit(struct SEMAGICNETC *pkMagicNetC, char *pcLogName, int iTimeOut, unsigned short usInPort);

void SeMagicNetCFin(struct SEMAGICNETC *pkMagicNetC);

void SeMagicNetCSetWaitTime(struct SEMAGICNETC *pkMagicNetC, unsigned int uiWaitTime);

bool SeMagicNetCReg(struct SEMAGICNETC *pkMagicNetC, const char *pcSvrName);

bool SeMagicNetCSendClient(struct SEMAGICNETC *pkMagicNetC, HSOCKET kHSocket, const char *pcBuf, int iLen);

void SeMagicNetCBindClientToSvr(struct SEMAGICNETC *pkMagicNetC, HSOCKET kHSocket, const char *pcSvrName);

void SeMagicNetCCloseClient(struct SEMAGICNETC *pkMagicNetC, HSOCKET kHSocket);

bool SeMagicNetCSendSvr(struct SEMAGICNETC *pkMagicNetC, const char *pcSvrName, const char *pcBuf, int iLen);

enum MAGIC_STATE SeMagicNetCRead(struct SEMAGICNETC *pkMagicNetC, HSOCKET *rkRecvHSocket, char **pcBuf, int *riBufLen);

#endif
