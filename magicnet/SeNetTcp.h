#ifndef __SE_NETTCP_H__
#define __SE_NETTCP_H__

#include "SeList.h"
#include "SeNetBase.h"
#include "SeNetStream.h"
#include "SeNetSocket.h"

#define MAX_SOCKET_LEN 65535

typedef void (*SENETTCPONCONNECT)(SOCKET /*֡����SOCKET*/, HSOCKET/*���ӹ�����HSOCKET*/);
typedef void (*SENETTCPDISCONNECT)(SOCKET /*֡����SOCKET*/, HSOCKET/*���ߵ�HSOCKET*/);
typedef void (*SENETTCPRECV)(SOCKET /*֡����SOCKET*/, HSOCKET/*�յ����ݵ�HSOCKET*/, const char*, int);

struct SENETTCP
{
	HANDLE					kHandle;
	struct SENETSTREAM		kMemCache;
	struct SENETSSOCKET		kSvrSocketList;

	struct SENETCSOCKET		kFreeCSocketList;
	struct SENETCSOCKET		kActiveCSocketList;
	struct SENETCSOCKET		kConnectCSocketList;
	struct SENETCSOCKET		kDisConnectCSocketList;
	struct SECSOCKETNODE	kClientSocket[MAX_SOCKET_LEN];

	SENETTCPONCONNECT		pkOnConnectFunc;
	SENETTCPDISCONNECT		pkOnDisconnectFunc;
	SENETTCPRECV			pkOnRecvDataFunc;
};

// don't use it,SeNetTcpInit will run it.
void SeNetTcpCreate(struct SENETTCP *pkNetTcp, SENETTCPONCONNECT pkOnConnectFunc, SENETTCPDISCONNECT pkOnDisconnectFunc, SENETTCPRECV pkOnRecvDataFunc);

void SeNetTcpInit(struct SENETTCP *pkNetTcp);

void SeNetTcpFin(struct SENETTCP *pkNetTcp);

SOCKET SeNetTcpCreateSvrSocket(struct SENETTCP *pkNetTcp, const char *pcIP, unsigned short usPort);

HSOCKET SeNetTcpCreateClientSocket(struct SENETTCP *pkNetTcp, const char *pcIP, unsigned short usPort);

bool SeNetTcpSend(struct SENETTCP *pkNetTcp, HSOCKET kHSocket, const char* pkData, int iSize);

void SeNetTcpDisconnect(struct SENETTCP *pkNetTcp, HSOCKET kHSocket);

#endif
