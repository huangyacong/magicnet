#ifndef __SE_NETTCP_H__
#define __SE_NETTCP_H__

#include "SeLog.h"
#include "SeTool.h"
#include "SeList.h"
#include "SeNetBase.h"
#include "SeNetStream.h"
#include "SeNetSocket.h"

#define C_LEN 1024*4
#define MAX_SOCKET_LEN 65535

typedef void (*SENETTCPONCONNECT)(SOCKET /*֡����SOCKET*/, HSOCKET/*���ӹ�����HSOCKET*/);
typedef void (*SENETTCPDISCONNECT)(SOCKET /*֡����SOCKET*/, HSOCKET/*���ߵ�HSOCKET*/);
typedef void (*SENETTCPRECV)(SOCKET /*֡����SOCKET*/, HSOCKET/*�յ����ݵ�HSOCKET*/, const char*, int);

struct SENETTCP
{
	HANDLE					kHandle;
	HANDLE					kListenHandle;
	struct SELOG			kLog;
	struct SENETSTREAM		kMemCCache;
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

void SeNetTcpCreate(struct SENETTCP *pkNetTcp, char *pcLogName);

void SeNetTcpFree(struct SENETTCP *pkNetTcp);

SOCKET SeNetTcpAddSvr(struct SENETTCP *pkNetTcp, const char *pcIP, unsigned short usPort, int iMemSize, int iProtoFormat);


// ����ĺ�����user�ӿ�
void SeNetTcpInit(struct SENETTCP *pkNetTcp, char *pcLogName, SENETTCPONCONNECT pkOnConnectFunc, SENETTCPDISCONNECT pkOnDisconnectFunc, SENETTCPRECV pkOnRecvDataFunc);

void SeNetTcpFin(struct SENETTCP *pkNetTcp);

SOCKET SeNetTcpCreateSvrSocket(struct SENETTCP *pkNetTcp, const char *pcIP, unsigned short usPort, int iMemSize, int iProtoFormat);

HSOCKET SeNetTcpCreateClientSocket(struct SENETTCP *pkNetTcp, const char *pcIP, unsigned short usPort, int iProtoFormat);

bool SeNetTcpSend(struct SENETTCP *pkNetTcp, HSOCKET kHSocket, const char* pkData, int iSize);

void SeNetTcpDisconnect(struct SENETTCP *pkNetTcp, HSOCKET kHSocket);

#endif
