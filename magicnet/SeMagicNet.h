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

typedef void (*MAGICNETENGINEGATESTAT)(void *pkContext, char *pcSvrName, int, int);

struct SEMAGICNETS
{
	HSOCKET					kHScoketIn;
	HSOCKET					kHScoketOut;
	struct SENETCORE		kNetCore;
	struct SELIST			kRegSvrList;
	char					*pcRecvBuf;
	char					*pcSendBuf;
	unsigned long long		ullTime;
	int						iSendNum;
	int						iRecvNum;
};

bool SeMagicNetSInit(struct SEMAGICNETS *pkMagicNetS, char *pcLogName, int iTimeOut, unsigned short usMax, unsigned short usOutPort, unsigned short usInPort, int iLogLV);

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
	void					*pkContext;
	MAGICNETENGINEGATESTAT	pkGateStatFunc;
	unsigned long long		ullTime;
	int						iSendNum;
	int						iRecvNum;
	char					acSvrName[MAX_SVR_NAME_LEN];
};

bool SeMagicNetCInit(struct SEMAGICNETC *pkMagicNetC, char *pcLogName, int iTimeOut, unsigned short usInPort, int iLogLV);

void SeMagicNetCFin(struct SEMAGICNETC *pkMagicNetC);

void SeMagicNetCSetGateStatFunc(struct SEMAGICNETC *pkMagicNetC, MAGICNETENGINEGATESTAT	pkGateStatFunc, void *pkContext);

void SeMagicNetCSetWaitTime(struct SEMAGICNETC *pkMagicNetC, unsigned int uiWaitTime);

bool SeMagicNetCReg(struct SEMAGICNETC *pkMagicNetC, const char *pcSvrName);

bool SeMagicNetCSendClient(struct SEMAGICNETC *pkMagicNetC, HSOCKET kHSocket, const char *pcBuf, int iLen);

void SeMagicNetCBindClientToSvr(struct SEMAGICNETC *pkMagicNetC, HSOCKET kHSocket, const char *pcSvrName);

void SeMagicNetCCloseClient(struct SEMAGICNETC *pkMagicNetC, HSOCKET kHSocket);

bool SeMagicNetCSendSvr(struct SEMAGICNETC *pkMagicNetC, const char *pcSvrName, const char *pcBuf, int iLen);

char *SePacketData(struct SEMAGICNETC *pkMagicNetC, bool bToClient, const char *pcSvrName, HSOCKET kHSocket, int iLen, int *riBegin);

bool SeMagicNetCSendPacket(struct SEMAGICNETC *pkMagicNetC, const char *pcBuf, int iLen);

enum MAGIC_STATE SeMagicNetCRead(struct SEMAGICNETC *pkMagicNetC, HSOCKET *rkRecvHSocket, char **pcBuf, int *riBufLen);

#ifdef	__cplusplus
}
#endif

#endif
