#ifndef __SE_NETTCP_H__
#define __SE_NETTCP_H__

#include "SeList.h"
#include "SeNetBase.h"
#include "SeNetStream.h"

unsigned int usMaxSocketLen = 65535;

struct SECSOCKET
{
	HSOCKET				kHSocket;
	SOCKET				kOwnerListenSocket;
	unsigned short		usIndex;
	struct SENODE		kNode;
	struct SENETSTREAM	kSendNetStream;
	struct SENETSTREAM	kRecvNetStream;
};

struct SESVRSOCKETNODE
{
	SOCKET				kListenSocket;
	struct SENODE		kNode;
};

struct SENETTCP
{
	HANDLE				kHandle;
	struct SENETSTREAM	kMemCache;
	struct SELIST		kSvrSocketList;
	struct SECSOCKET	kClientSocket[usMaxSocketLen + 1];
};

typedef void (*SENETTCPONCONNECT)(SOCKET /*֡����SOCKET*/, HSOCKET/*���ӹ�����HSOCKET*/);
typedef void (*SENETTCPDISCONNECT)(SOCKET /*֡����SOCKET*/, HSOCKET/*���ߵ�HSOCKET*/);
typedef void (*SENETTCPRECV)(SOCKET /*֡����SOCKET*/, HSOCKET/*�յ����ݵ�HSOCKET*/, const char*, int);

void SeNetTcpInit(struct SENETTCP *pkNetTcp);

void SeNetTcpFin(struct SENETTCP *pkNetTcp);

SOCKET SeNetTcpCreateSvrSocket(struct SENETTCP *pkNetTcp, const char *pcIP, unsigned short usPort);

HSOCKET SeNetTcpCreateClientSocket(struct SENETTCP *pkNetTcp, const char *pcIP, unsigned short usPort);

bool SeNetTcpSend(struct SENETTCP *pkNetTcp, HSOCKET kHSocket, const char* pkData, int iSize);

void SeNetTcpDisconnect(struct SENETTCP *pkNetTcp, HSOCKET kHSocket);

void SeNetTcpProcess(struct SENETTCP *pkNetTcp, \
	SENETTCPONCONNECT pkOnConnectFunc, SENETTCPDISCONNECT pkOnDisconnectFunc, SENETTCPRECV pkOnRecvDataFunc);

#endif
