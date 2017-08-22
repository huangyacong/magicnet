#ifndef __SE_MAGICNET_H__
#define __SE_MAGICNET_H__

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "SeNetCore.h"
#include "SeTime.h"

#define MAX_SVR_NAME_LEN 128

typedef void (*MAGICNETENGINEGATESTAT)(void *pkContext, const char *pcSvrName, int, int);

struct SEMAGICNETS
{
	HSOCKET					kHScoketIn;
	HSOCKET					kHScoketOut;
	struct SELIST			kRegSvrList;
	char					*pcRecvBuf;
	char					*pcSendBuf;
	unsigned long long		ullTime;
	int						iSendNum;
	int						iRecvNum;
	struct SENETCORE		kNetCore;
};

bool SeMagicNetSInit(struct SEMAGICNETS *pkMagicNetS, const char *pcLogName, int iTimeOut, unsigned short usMax, bool bBigHeader, const char *pcOutIP, unsigned short usOutPort, unsigned short usInPort, int iLogLV);

void SeMagicNetSFin(struct SEMAGICNETS *pkMagicNetS);

void SeMagicNetSSetWaitTime(struct SEMAGICNETS *pkMagicNetS, unsigned int uiWaitTime);

void SeMagicNetSSetLogContextFunc(struct SEMAGICNETS *pkMagicNetS, SELOGCONTEXT pkLogContextFunc, void *pkLogContect);

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
	void					*pkContext;
	MAGICNETENGINEGATESTAT	pkGateStatFunc;
	unsigned long long		ullTime;
	char					*pcRecvBuf;
	char					*pcSendBuf;
	int						iInPort;
	int						iTimeOut;
	int						iSendNum;
	int						iRecvNum;
	char					acTag[56];
	char					acSvrName[MAX_SVR_NAME_LEN];
	struct SENETCORE		kNetCore;
};

bool SeMagicNetCInit(struct SEMAGICNETC *pkMagicNetC, const char *pcLogName, int iTimeOut, unsigned short usInPort, int iLogLV);

void SeMagicNetCFin(struct SEMAGICNETC *pkMagicNetC);

void SeMagicNetCSetLogContextFunc(struct SEMAGICNETC *pkMagicNetC, SELOGCONTEXT pkLogContextFunc, void *pkLogContect);

void SeMagicNetCSetGateStatFunc(struct SEMAGICNETC *pkMagicNetC, MAGICNETENGINEGATESTAT	pkGateStatFunc, void *pkContext);

void SeMagicNetCSetWaitTime(struct SEMAGICNETC *pkMagicNetC, unsigned int uiWaitTime);

bool SeMagicNetCReg(struct SEMAGICNETC *pkMagicNetC, const char *pcSvrName);

bool SeMagicNetCSendClient(struct SEMAGICNETC *pkMagicNetC, HSOCKET kHSocket, const char *pcBuf, int iLen);

bool SeMagicNetCSendClientExtend(struct SEMAGICNETC *pkMagicNetC, HSOCKET kHSocket, const struct SENETSTREAMBUF *pkBufList, int iNum);

void SeMagicNetCBindClientToSvr(struct SEMAGICNETC *pkMagicNetC, HSOCKET kHSocket, const char *pcSvrName);

void SeMagicNetCCloseClient(struct SEMAGICNETC *pkMagicNetC, HSOCKET kHSocket);

bool SeMagicNetCSendSvr(struct SEMAGICNETC *pkMagicNetC, const char *pcSvrName, const char *pcBuf, int iLen);

bool SeMagicNetCSendSvrExtend(struct SEMAGICNETC *pkMagicNetC, const char *pcSvrName, const struct SENETSTREAMBUF *pkBufList, int iNum);

enum MAGIC_STATE SeMagicNetCRead(struct SEMAGICNETC *pkMagicNetC, HSOCKET *rkRecvHSocket, char **pcBuf, int *riBufLen);

#ifdef	__cplusplus
}
#endif

#endif
