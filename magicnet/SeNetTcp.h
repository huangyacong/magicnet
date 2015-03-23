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
	struct SELOG			kLog;
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

void SeNetTcpCreate(struct SENETTCP *pkNetTcp, char *pcLogName);

void SeNetTcpFree(struct SENETTCP *pkNetTcp);


// 下面的函数是user接口
void SeNetTcpInit(struct SENETTCP *pkNetTcp, char *pcLogName, SENETTCPONCONNECT pkOnConnectFunc, SENETTCPDISCONNECT pkOnDisconnectFunc, SENETTCPRECV pkOnRecvDataFunc);

void SeNetTcpFin(struct SENETTCP *pkNetTcp);

SOCKET SeNetTcpCreateSvrSocket(struct SENETTCP *pkNetTcp, const char *pcIP, unsigned short usPort);

HSOCKET SeNetTcpCreateClientSocket(struct SENETTCP *pkNetTcp, const char *pcIP, unsigned short usPort);

bool SeNetTcpSend(struct SENETTCP *pkNetTcp, HSOCKET kHSocket, const char* pkData, int iSize);

void SeNetTcpDisconnect(struct SENETTCP *pkNetTcp, HSOCKET kHSocket);

#endif
