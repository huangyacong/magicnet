#ifndef __SE_NETCORE_H__
#define __SE_NETCORE_H__

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

struct SENETCORE
{
	HANDLE					kHandle;
	struct SELOG			kLog;
	struct SESOCKETMGR		kSocketMgr;
#if defined(__linux)
	struct epoll_event		akEvents[64];
	char					*pcBuf;
#elif (defined(_WIN32) || defined(WIN32))
	struct SELIST			kList;
	int						iListenNo;
#endif
};

void SeNetCoreInit(struct SENETCORE *pkNetCore, char *pcLogName, int iTimeOut, unsigned short usMax);

void SeNetCoreFin(struct SENETCORE *pkNetCore);

HSOCKET SeNetCoreTCPListen(struct SENETCORE *pkNetCore, const char *pcIP, unsigned short usPort,\
				int iHeaderLen, SEGETHEADERLENFUN pkGetHeaderLenFun, SESETHEADERLENFUN pkSetHeaderLenFun);

HSOCKET SeNetCoreTCPClient(struct SENETCORE *pkNetCore, const char *pcIP, unsigned short usPort,\
				int iHeaderLen, SEGETHEADERLENFUN pkGetHeaderLenFun, SESETHEADERLENFUN pkSetHeaderLenFun);

bool SeNetCoreSend(struct SENETCORE *pkNetCore, HSOCKET kHSocket, char* pcBuf, int iSize);

void SeNetCoreDisconnect(struct SENETCORE *pkNetCore, HSOCKET kHSocket);

bool SeNetCoreRead(struct SENETCORE *pkNetCore, int *riEvent, HSOCKET *rkListenHSocket, HSOCKET *rkHSocket, char *pcBuf, int *riLen, int *rSSize, int *rRSize);

struct SESOCKET *SeNetCoreGetSocket(struct SENETCORE *pkNetCore, HSOCKET kHSocket);

#endif
