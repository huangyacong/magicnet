#ifndef __SE_NETCORE_H__
#define __SE_NETCORE_H__

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "SeLog.h"
#include "SeNetBase.h"
#include "SeNetSocket.h"

#define SENETCORE_MAX_SOCKET_BUF_LEN 1024*1024*4

#define SENETCORE_EVENT_SOCKET_CONNECT 1
#define SENETCORE_EVENT_SOCKET_CONNECT_FAILED 2
#define SENETCORE_EVENT_SOCKET_DISCONNECT 3
#define SENETCORE_EVENT_SOCKET_RECV_DATA 4

struct SENETCORE
{
	HANDLE					kHandle;
	struct SELOG			kLog;
	struct SESOCKETMGR		kSocketMgr;
#if defined(__linux)
	struct epoll_event		akEvents[64];
#endif
	char					acBuf[SENETCORE_MAX_SOCKET_BUF_LEN];
};

void SeNetCoreInit(struct SENETCORE *pkNetCore, char *pcLogName, unsigned short usMax);

void SeNetCoreFin(struct SENETCORE *pkNetCore);

HSOCKET SeNetCoreTCPListen(struct SENETCORE *pkNetCore, const char *pcIP, unsigned short usPort,\
				int iHeaderLen, SEGETHEADERLENFUN pkGetHeaderLenFun, SESETHEADERLENFUN pkSetHeaderLenFun);

HSOCKET SeNetCoreTCPClient(struct SENETCORE *pkNetCore, const char *pcIP, unsigned short usPort,\
				int iHeaderLen, SEGETHEADERLENFUN pkGetHeaderLenFun, SESETHEADERLENFUN pkSetHeaderLenFun);

bool SeNetCoreSend(struct SENETCORE *pkNetCore, HSOCKET kHSocket, char* pcBuf, int iSize);

void SeNetCoreDisconnect(struct SENETCORE *pkNetCore, HSOCKET kHSocket);

bool SeNetCoreRead(struct SENETCORE *pkNetCore, int *riEvent, HSOCKET *rkListenHSocket, HSOCKET *rkHSocket, char *pcBuf, int *riLen);

#endif
