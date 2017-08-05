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
	SENETCORE_EVENT_SOCKET_RECV_DATA = 4
};

#define SENETCORE_SOCKET_BACKLOG 256
#define SENETCORE_SOCKET_RECV_BUF_LEN 1024
#define SENETCORE_SOCKET_RS_BUF_LEN 1024*1024*1024

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
	struct SESOCKETMGR		kSocketMgr;
	struct SELOG			kLog;
};

#define NET_CORE_WAIT_TIME 0

void SeNetCoreInit(struct SENETCORE *pkNetCore, char *pcLogName, unsigned short usMax, int iLogLV);

void SeNetCoreFin(struct SENETCORE *pkNetCore);

void SeNetCoreSetLogContextFunc(struct SENETCORE *pkNetCore, SELOGCONTEXT pkLogContextFunc, void *pkLogContect);

void SeNetCoreSetWaitTime(struct SENETCORE *pkNetCore, unsigned int uiWaitTime);

HSOCKET SeNetCoreTCPListen(struct SENETCORE *pkNetCore, const char *pcIP, unsigned short usPort, int iHeaderLen, int iTimeOut, SEGETHEADERLENFUN pkGetHeaderLenFun, SESETHEADERLENFUN pkSetHeaderLenFun);

HSOCKET SeNetCoreTCPClient(struct SENETCORE *pkNetCore, const char *pcIP, unsigned short usPort, int iHeaderLen, int iTimeOut, int iConnectTimeOut, SEGETHEADERLENFUN pkGetHeaderLenFun, SESETHEADERLENFUN pkSetHeaderLenFun);

bool SeNetCoreSend(struct SENETCORE *pkNetCore, HSOCKET kHSocket, const char* pcBuf, int iSize);

bool SeNetCoreSendExtend(struct SENETCORE *pkNetCore, HSOCKET kHSocket, const struct SENETSTREAMBUF *pkBufList, int iNum);

void SeNetCoreDisconnect(struct SENETCORE *pkNetCore, HSOCKET kHSocket);

bool SeNetCoreRead(struct SENETCORE *pkNetCore, int *riEvent, HSOCKET *rkListenHSocket, HSOCKET *rkHSocket, char *pcBuf, int *riLen, int *rSSize, int *rRSize);

struct SESOCKET *SeNetCoreGetSocket(struct SENETCORE *pkNetCore, HSOCKET kHSocket);

#ifdef	__cplusplus
}
#endif

#endif
