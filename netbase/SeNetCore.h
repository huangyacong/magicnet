#ifndef __SE_NETCORE_H__
#define __SE_NETCORE_H__

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "SeLog.h"
#include "SeNetBase.h"
#include "SeNetSocket.h"
#include "SeTool.h"

enum SENETCORE_EVENT_SOCKET
{
	SENETCORE_EVENT_SOCKET_IDLE = 0,
	SENETCORE_EVENT_SOCKET_CONNECT = 1,
	SENETCORE_EVENT_SOCKET_CONNECT_FAILED = 2,
	SENETCORE_EVENT_SOCKET_DISCONNECT = 3,
	SENETCORE_EVENT_SOCKET_RECV_DATA = 4,
	SENETCORE_EVENT_SOCKET_TIMER = 5,
};

#define NET_CORE_WAIT_TIME -1
#define SENETCORE_TIMER_FIRST 32 
#define SENETCORE_SOCKET_BACKLOG 1024
#define SENETCORE_SOCKET_RECV_BUF_LEN 1024

struct SENETCORE
{
#if defined(__linux)
	struct epoll_event		akEvents[SENETCORE_SOCKET_BACKLOG];
	char					acFlag[48];
	int						iTag;
#elif (defined(_WIN32) || defined(WIN32))
	struct SELIST			kList;
	struct SELIST			kListenList;
#endif
	int						iWaitTime;
	int						iFlag;
	HANDLE					kHandle;
	HANDLE					kTimerHandle;
	struct SESOCKETMGR		kSocketMgr;
	struct SELOG			kLog;
};

void SeNetCoreInit(struct SENETCORE *pkNetCore, const char *pcLogName, unsigned short usMax, int iTimerCnt, int iLogLV);

void SeNetCoreFin(struct SENETCORE *pkNetCore);

void SeNetCoreSetLogContextFunc(struct SENETCORE *pkNetCore, SELOGCONTEXT pkLogContextFunc, void *pkLogContect);

void SeNetCoreSetWaitTime(struct SENETCORE *pkNetCore, int iWaitTime);

HSOCKET SeNetCoreTCPListen(struct SENETCORE *pkNetCore, int iDomain, bool bReusePort, const char *pcIP, unsigned short usPort, int iHeaderLen, bool bNoDelay, int iTimeOut, SEGETHEADERLENFUN pkGetHeaderLenFun, SESETHEADERLENFUN pkSetHeaderLenFun);

HSOCKET SeNetCoreTCPClient(struct SENETCORE *pkNetCore, int iDomain, const char *pcIP, unsigned short usPort, int iHeaderLen, bool bNoDelay, int iTimeOut, int iConnectTimeOut, SEGETHEADERLENFUN pkGetHeaderLenFun, SESETHEADERLENFUN pkSetHeaderLenFun);

bool SeNetCoreSend(struct SENETCORE *pkNetCore, HSOCKET kHSocket, const char* pcBuf, int iSize);

bool SeNetCoreSendExtend(struct SENETCORE *pkNetCore, HSOCKET kHSocket, const struct SENETSTREAMBUF *pkBufList, int iNum);

void SeNetCoreDisconnect(struct SENETCORE *pkNetCore, HSOCKET kHSocket);

bool SeNetCoreRead(struct SENETCORE *pkNetCore, int *riEvent, HSOCKET *rkListenHSocket, HSOCKET *rkHSocket, char *pcBuf, int *riLen, int *rSSize, int *rRSize);

struct SESOCKET *SeNetCoreGetSocket(struct SENETCORE *pkNetCore, HSOCKET kHSocket);

#ifdef	__cplusplus
}
#endif

#endif
