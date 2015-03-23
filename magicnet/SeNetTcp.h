#ifndef __SE_NETTCP_H__
#define __SE_NETTCP_H__

#include "SeLog.h"
#include "SeTool.h"
#include "SeList.h"
#include "SeNetBase.h"
#include "SeNetStream.h"
#include "SeNetSocket.h"

#define MAX_SOCKET_LEN 65535

typedef void (*SENETTCPONCONNECT)(SOCKET /*帧听着SOCKET*/, HSOCKET/*连接过来的HSOCKET*/);
typedef void (*SENETTCPDISCONNECT)(SOCKET /*帧听着SOCKET*/, HSOCKET/*断线的HSOCKET*/);
typedef void (*SENETTCPRECV)(SOCKET /*帧听着SOCKET*/, HSOCKET/*收到数据的HSOCKET*/, const char*, int);

struct SENETTCP
{
	HANDLE					kHandle;
	unsigned short			usCounter;

	struct SELOG			kLog;
	struct SENETCSOCKET		kFreeCSocketList;
	struct SENETCSOCKET		kAcceptCSocketList;
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

struct SECSOCKETNODE* SeNetTcpAddSvr(struct SENETTCP *pkNetTcp, const char *pcIP, unsigned short usPort, int iMemSize, int iProtoFormat);


// 下面的函数是user接口
void SeNetTcpInit(struct SENETTCP *pkNetTcp, char *pcLogName, SENETTCPONCONNECT pkOnConnectFunc, SENETTCPDISCONNECT pkOnDisconnectFunc, SENETTCPRECV pkOnRecvDataFunc);

void SeNetTcpFin(struct SENETTCP *pkNetTcp);

SOCKET SeNetTcpCreateSvrSocket(struct SENETTCP *pkNetTcp, const char *pcIP, unsigned short usPort, int iMemSize, int iProtoFormat);

HSOCKET SeNetTcpCreateClientSocket(struct SENETTCP *pkNetTcp, const char *pcIP, unsigned short usPort, int iProtoFormat);

bool SeNetTcpSend(struct SENETTCP *pkNetTcp, HSOCKET kHSocket, const char* pkData, int iSize);

void SeNetTcpDisconnect(struct SENETTCP *pkNetTcp, HSOCKET kHSocket);

void SeNetTcpProcess(struct SENETTCP *pkNetTcp);

#endif
