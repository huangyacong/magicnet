#include "SeMagicNet.h"
#include "SeTool.h"
#include "SeTime.h"

#define MAGICNET_TIME_OUT 1000*5 // sec
#define MAX_RECV_BUF_LEN 1024*1024*4
#define STAT_TIME 1000

#define SVR_TO_MAGICNET_REG_SVR 0
#define SVR_TO_MAGICNET_SENDTO_SVR 1
#define SVR_TO_MAGICNET_SENDTO_CLIENT 2
#define SVR_TO_MAGICNET_CLOSE_CLIENT 3
#define SVR_TO_MAGICNET_BIND_CLIENT 4
#define SVR_TO_MAGICNET_ACTIVE 5
#define SVR_TO_MAGICNET_GATE_STAT 6

#define MAGICNET_TO_SVR_CLIENT_CONNECT 0
#define MAGICNET_TO_SVR_CLIENT_DISCONNECT 1
#define MAGICNET_TO_SVR_RECV_DATA_FROM_SVR 2
#define MAGICNET_TO_SVR_RECV_DATA_FROM_CLIENT 3
#define MAGICNET_TO_SVR_ACTIVE 4
#define MAGICNET_TO_SVR_GATE_STAT 5

char acWatchdogName[] = "watchdog.";

union COMMDATA
{
	HSOCKET					kHSocket;
	char					acName[MAX_SVR_NAME_LEN];
};

struct SECOMMONDATA
{
	int						iProco;
	union COMMDATA			kData;
	int						iBufLen;
};

struct SEGATESTAT
{
	int iSend;
	int iRecv;
};

struct REGSVRNODE
{
	struct SENODE			kNode;
	unsigned long long		llActive;
	HSOCKET					kHSocket;
	char					acName[MAX_SVR_NAME_LEN];
};

// send
bool SeSetHeader(unsigned char* pcHeader, const int iheaderlen, const int ilen)
{
	// 大头
	/*
	if(iheaderlen == 2)
	{
		if(ilen < 0 || ilen > 0xFFFF) { return false; }
		pcHeader[0] = (ilen >> 8) & 0xff;
		pcHeader[1] = ilen & 0xff;
		return true;
	}
	*/

	// 小头
	if(iheaderlen == 2)
	{
		if(ilen < 0 || ilen > 0xFFFF) { return false; }
		pcHeader[0] = ilen & 0xFF;
		pcHeader[1] = (ilen >> 8) & 0xFF;
		return true;
	}

	// 小头
	if(iheaderlen == 4)
	{
		if(ilen < 0 || ilen > 1024*1024) { return false; }
		// 将int数值转换为占四个字节的byte数组，本方法适用于(低位在前，高位在后)的顺序。
		pcHeader[3] = ((ilen & 0xFF000000) >> 24);
		pcHeader[2] = ((ilen & 0x00FF0000) >> 16);
		pcHeader[1] = ((ilen & 0x0000FF00) >> 8);
		pcHeader[0] = ((ilen & 0x000000FF));
		return true;
	}

	return false;
}

// recv
bool SeGetHeader(const unsigned char* pcHeader, const int iheaderlen, int *ilen)
{
	// 大头
	/*
	if(iheaderlen == 2)
	{
		*ilen = (unsigned short)(pcHeader[0] << 8 | pcHeader[1]);
		if(*ilen < 0 || *ilen > 0xFFFF) { return false; }
		return true;
	}
	*/

	// 小头
	if(iheaderlen == 2)
	{
		*ilen = (unsigned short)(pcHeader[1] << 8 | pcHeader[0]);
		if(*ilen < 0 || *ilen > 0xFFFF) { return false; }
		return true;
	}

	// 小头
	if(iheaderlen == 4)
	{
		// byte数组中取int数值，本方法适用于(低位在前，高位在后)的顺序
		*ilen = (int)((pcHeader[0] & 0xFF) | ((pcHeader[1] << 8) & 0xFF00) | ((pcHeader[2] << 16) & 0xFF0000) | ((pcHeader[3] << 24) & 0xFF000000));
		if(*ilen < 0 || *ilen > 1024*1024) { return false; }
		return true;
	}

	return false;
}

struct REGSVRNODE *SeGetRegSvrNodeBySvrName(struct SELIST *pkRegSvrList, const char *pcName)
{
	struct SENODE *pkNode;
	struct REGSVRNODE *pkSvr;

	pkNode = pkRegSvrList->head;
	while(pkNode)
	{
		pkSvr = SE_CONTAINING_RECORD(pkNode, struct REGSVRNODE, kNode);
		if(strcmp(pcName, pkSvr->acName) == 0) { return pkSvr; }
		pkNode = pkNode->next;
	}

	return 0;
}

struct REGSVRNODE *SeGetRegSvrNodeBySocket(struct SELIST *pkRegSvrList, HSOCKET	kHSocket)
{
	struct SENODE *pkNode;
	struct REGSVRNODE *pkSvr;

	pkNode = pkRegSvrList->head;
	while(pkNode)
	{
		pkSvr = SE_CONTAINING_RECORD(pkNode, struct REGSVRNODE, kNode);
		if(kHSocket == pkSvr->kHSocket) { return pkSvr; }
		pkNode = pkNode->next;
	}

	return 0;
}

struct REGSVRNODE *SeAddRegSvrNode(struct SELIST *pkRegSvrList, const char *pcName, HSOCKET	kHSocket)
{
	struct REGSVRNODE *pkRegSvrNode;

	if(SeGetRegSvrNodeBySvrName(pkRegSvrList, pcName)) { return 0; }
	if(SeGetRegSvrNodeBySocket(pkRegSvrList, kHSocket)) { return 0; }

	pkRegSvrNode = (struct REGSVRNODE *)SeMallocMem(sizeof(struct REGSVRNODE));
	if(!pkRegSvrNode) { return 0; }
	SeListInitNode(&pkRegSvrNode->kNode);
	pkRegSvrNode->llActive = SeTimeGetTickCount();
	pkRegSvrNode->kHSocket = kHSocket;
	memset(pkRegSvrNode->acName, 0, sizeof(pkRegSvrNode->acName));
	SeStrNcpy(pkRegSvrNode->acName, sizeof(pkRegSvrNode->acName), pcName);
	SeListHeadAdd(pkRegSvrList, &pkRegSvrNode->kNode);
	return pkRegSvrNode;
}

void SeDelRegSvrNode(struct SELIST *pkRegSvrList, HSOCKET kHSocket)
{
	struct SENODE *pkNode;
	struct REGSVRNODE *pkSvr;

	pkNode = pkRegSvrList->head;
	while(pkNode)
	{
		pkSvr = SE_CONTAINING_RECORD(pkNode, struct REGSVRNODE, kNode);
		if(kHSocket == pkSvr->kHSocket) { SeListRemove(pkRegSvrList, &pkSvr->kNode); SeFreeMem(pkSvr); break; }
		pkNode = pkNode->next;
	}
}

void SeFreeRegSvrNode(struct SELIST *pkRegSvrList)
{
	struct SENODE *pkNode;
	struct REGSVRNODE *pkRegSvrNode;

	pkNode = SeListHeadPop(pkRegSvrList);
	while(pkNode)
	{
		pkRegSvrNode = SE_CONTAINING_RECORD(pkNode, struct REGSVRNODE, kNode);
		SeFreeMem(pkRegSvrNode);
		pkNode = SeListHeadPop(pkRegSvrList);
	}
}

bool SeMagicNetSInit(struct SEMAGICNETS *pkMagicNetS, char *pcLogName, int iTimeOut, unsigned short usMax, unsigned short usOutPort, unsigned short usInPort, int iLogLV)
{
	SeNetCoreInit(&pkMagicNetS->kNetCore, pcLogName, iTimeOut, usMax, iLogLV);
	SeListInit(&pkMagicNetS->kRegSvrList);
	pkMagicNetS->pcRecvBuf = (char*)SeMallocMem(MAX_RECV_BUF_LEN);
	assert(pkMagicNetS->pcRecvBuf);
	pkMagicNetS->pcSendBuf = (char*)SeMallocMem(MAX_RECV_BUF_LEN);
	assert(pkMagicNetS->pcSendBuf);

	pkMagicNetS->ullTime = SeTimeGetTickCount();
	pkMagicNetS->iSendNum = 0;
	pkMagicNetS->iRecvNum = 0;

	pkMagicNetS->kHScoketOut = SeNetCoreTCPListen(&pkMagicNetS->kNetCore, "0.0.0.0", usOutPort, 2, &SeGetHeader, &SeSetHeader);
	if(pkMagicNetS->kHScoketOut <= 0) { return false; }
	pkMagicNetS->kHScoketIn = SeNetCoreTCPListen(&pkMagicNetS->kNetCore, "127.0.0.1", usInPort, 4, &SeGetHeader, &SeSetHeader);
	if(pkMagicNetS->kHScoketIn <= 0) { return false; }

	return true;
}

void SeMagicNetSFin(struct SEMAGICNETS *pkMagicNetS)
{
	SeFreeMem(pkMagicNetS->pcRecvBuf);
	SeFreeMem(pkMagicNetS->pcSendBuf);
	SeFreeRegSvrNode(&pkMagicNetS->kRegSvrList);
	SeNetCoreFin(&pkMagicNetS->kNetCore);
	pkMagicNetS->kHScoketOut = 0;
	pkMagicNetS->kHScoketIn = 0;
}

void SeMagicNetSSetWaitTime(struct SEMAGICNETS *pkMagicNetS, unsigned int uiWaitTime)
{
	SeNetCoreSetWaitTime(&pkMagicNetS->kNetCore, uiWaitTime);
}

void SeMagicNetSSendActive(struct SEMAGICNETS *pkMagicNetS)
{
	struct SENODE *pkNode;
	struct REGSVRNODE *pkSvr;
	struct SECOMMONDATA *pkComData;

	pkNode = pkMagicNetS->kRegSvrList.head;
	while(pkNode)
	{
		pkSvr = SE_CONTAINING_RECORD(pkNode, struct REGSVRNODE, kNode);
		if((pkSvr->llActive + MAGICNET_TIME_OUT) <= SeTimeGetTickCount())
		{
			pkSvr->llActive = SeTimeGetTickCount();
			pkComData = (struct SECOMMONDATA *)pkMagicNetS->pcRecvBuf;
			pkComData->iProco = MAGICNET_TO_SVR_ACTIVE;
			pkComData->iBufLen = 0;
			SeNetCoreSend(&pkMagicNetS->kNetCore, pkSvr->kHSocket, (char*)pkComData, (int)sizeof(struct SECOMMONDATA));
			pkMagicNetS->iSendNum++;
		}
		pkNode = pkNode->next;
	}
}

void SeMagicNetSSendStat(struct SEMAGICNETS *pkMagicNetS)
{
	unsigned long long ullTime;
	struct SEGATESTAT kGateStat;
	struct REGSVRNODE *pkSvrWatchdog;
	struct SECOMMONDATA *pkComDataGateStat;

	ullTime = SeTimeGetTickCount();

	if(pkMagicNetS->ullTime + STAT_TIME > ullTime)
	{
		return;
	}

	kGateStat.iSend = (pkMagicNetS->iSendNum*1000)/(int)(ullTime - pkMagicNetS->ullTime);
	kGateStat.iRecv = (pkMagicNetS->iRecvNum*1000)/(int)(ullTime - pkMagicNetS->ullTime);

	pkSvrWatchdog = SeGetRegSvrNodeBySvrName(&pkMagicNetS->kRegSvrList, acWatchdogName);
	if(pkSvrWatchdog)
	{
		pkComDataGateStat = (struct SECOMMONDATA *)pkMagicNetS->pcSendBuf;
		pkComDataGateStat->iProco = MAGICNET_TO_SVR_GATE_STAT;
		pkComDataGateStat->iBufLen = (int)sizeof(struct SEGATESTAT);
		SeStrNcpy(pkComDataGateStat->kData.acName, sizeof(pkComDataGateStat->kData.acName), "gate");
		memcpy(pkMagicNetS->pcSendBuf + (int)sizeof(struct SECOMMONDATA), &kGateStat, (int)sizeof(struct SEGATESTAT));
		SeNetCoreSend(&pkMagicNetS->kNetCore, pkSvrWatchdog->kHSocket, (char*)pkComDataGateStat, pkComDataGateStat->iBufLen + (int)sizeof(struct SECOMMONDATA));
	}

	pkMagicNetS->iRecvNum = 0;
	pkMagicNetS->iSendNum = 0;
	pkMagicNetS->ullTime = ullTime;
}

void SeMagicNetSWork(struct SEMAGICNETS *pkMagicNetS)
{
	int riLen;
	int rSSize;
	int rRSize;
	int riEvent;
	bool result;
	HSOCKET rkHSocket;
	HSOCKET rkListenHSocket;
	struct REGSVRNODE *pkSvr;
	struct REGSVRNODE *pkSvrMe;
	struct SESOCKET *pkSeSocket;
	char acName[MAX_SVR_NAME_LEN];
	struct SECOMMONDATA *pkComData;
	struct REGSVRNODE *pkSvrWatchdog;

	SeMagicNetSSendActive(pkMagicNetS);
	SeMagicNetSSendStat(pkMagicNetS);
	
	riLen = MAX_RECV_BUF_LEN - (int)sizeof(struct SECOMMONDATA);
	result = SeNetCoreRead(&pkMagicNetS->kNetCore, 
		&riEvent, &rkListenHSocket, &rkHSocket, pkMagicNetS->pcRecvBuf + (int)sizeof(struct SECOMMONDATA), &riLen, &rSSize, &rRSize);
	if(!result) { return; }
	if(riEvent == SENETCORE_EVENT_SOCKET_IDLE) { SeTimeSleep(1); return; }

	pkMagicNetS->iRecvNum++;
	
	// 外网 
	if(rkListenHSocket == pkMagicNetS->kHScoketOut)
	{	
		// watchdog is working? 
		pkSvrWatchdog = SeGetRegSvrNodeBySvrName(&pkMagicNetS->kRegSvrList, acWatchdogName);
		if(!pkSvrWatchdog) { SeNetCoreDisconnect(&pkMagicNetS->kNetCore, rkHSocket); return; }

		if(riEvent == SENETCORE_EVENT_SOCKET_CONNECT || riEvent == SENETCORE_EVENT_SOCKET_DISCONNECT)
		{
			pkSeSocket = SeNetCoreGetSocket(&pkMagicNetS->kNetCore, rkHSocket);
			assert(pkSeSocket);

			pkComData = (struct SECOMMONDATA *)pkMagicNetS->pcRecvBuf;
			pkComData->iProco = riEvent == SENETCORE_EVENT_SOCKET_CONNECT ? MAGICNET_TO_SVR_CLIENT_CONNECT : MAGICNET_TO_SVR_CLIENT_DISCONNECT;
			pkComData->kData.kHSocket = rkHSocket;
			pkComData->iBufLen = riEvent == SENETCORE_EVENT_SOCKET_CONNECT ? riLen : 0;
			SeNetCoreSend(&pkMagicNetS->kNetCore, pkSvrWatchdog->kHSocket, (char*)pkComData, pkComData->iBufLen + (int)sizeof(struct SECOMMONDATA));

			if(riEvent == SENETCORE_EVENT_SOCKET_CONNECT)
			{
				memset(pkSeSocket->acBindSvrName, 0, sizeof(pkSeSocket->acBindSvrName));
			}

			if(riEvent == SENETCORE_EVENT_SOCKET_DISCONNECT)
			{
				pkSvr = SeGetRegSvrNodeBySvrName(&pkMagicNetS->kRegSvrList, pkSeSocket->acBindSvrName);
				if(pkSvr) { SeNetCoreSend(&pkMagicNetS->kNetCore, pkSvr->kHSocket, (char*)pkComData, pkComData->iBufLen + (int)sizeof(struct SECOMMONDATA)); }
				memset(pkSeSocket->acBindSvrName, 0, sizeof(pkSeSocket->acBindSvrName));
			}
			pkMagicNetS->iSendNum++;
		}

		if(riEvent == SENETCORE_EVENT_SOCKET_RECV_DATA)
		{
			pkSeSocket = SeNetCoreGetSocket(&pkMagicNetS->kNetCore, rkHSocket);
			assert(pkSeSocket);

			pkComData = (struct SECOMMONDATA *)pkMagicNetS->pcRecvBuf;
			pkComData->iProco = MAGICNET_TO_SVR_RECV_DATA_FROM_CLIENT;
			pkComData->kData.kHSocket = rkHSocket;
			pkComData->iBufLen = riLen;

			pkSvr = SeGetRegSvrNodeBySvrName(&pkMagicNetS->kRegSvrList, pkSeSocket->acBindSvrName);
			if(pkSvr) { SeNetCoreSend(&pkMagicNetS->kNetCore, pkSvr->kHSocket, (char*)pkComData, pkComData->iBufLen + (int)sizeof(struct SECOMMONDATA)); }
			else { SeNetCoreSend(&pkMagicNetS->kNetCore, pkSvrWatchdog->kHSocket, (char*)pkComData, pkComData->iBufLen + (int)sizeof(struct SECOMMONDATA)); }
			pkMagicNetS->iSendNum++;
		}
	}
	
	//  内网
	if(rkListenHSocket == pkMagicNetS->kHScoketIn)
	{
		if(riEvent == SENETCORE_EVENT_SOCKET_CONNECT || riEvent == SENETCORE_EVENT_SOCKET_CONNECT_FAILED) { return; }

		if(riLen < (int)sizeof(struct SECOMMONDATA)) { assert(0 != 0); return; }
		pkComData = (struct SECOMMONDATA *)(pkMagicNetS->pcRecvBuf + (int)sizeof(struct SECOMMONDATA));

		if(riEvent == SENETCORE_EVENT_SOCKET_DISCONNECT) { SeDelRegSvrNode(&pkMagicNetS->kRegSvrList, rkHSocket); }

		if(pkComData->iProco == SVR_TO_MAGICNET_ACTIVE) { return; }

		if(pkComData->iProco == SVR_TO_MAGICNET_BIND_CLIENT && pkComData->iBufLen > 0 && pkComData->iBufLen < sizeof(acName))
		{
			if(((int)sizeof(struct SECOMMONDATA) + pkComData->iBufLen) != riLen) { return; }

			memset(acName, 0, sizeof(acName));
			memcpy(acName, (char*)pkComData + (int)sizeof(struct SECOMMONDATA), pkComData->iBufLen);
			acName[sizeof(acName) - 1] = '\0';
			if(strcmp(acWatchdogName, acName) == 0) { return; }
			pkSeSocket = SeNetCoreGetSocket(&pkMagicNetS->kNetCore, pkComData->kData.kHSocket);
			if(!pkSeSocket) { return; }
			if(pkSeSocket->usStatus != SOCKET_STATUS_ACTIVECONNECT) { return; }
			if(pkSeSocket->kBelongListenHSocket != pkMagicNetS->kHScoketOut) { return; }
			if(SeGetRegSvrNodeBySocket(&pkMagicNetS->kRegSvrList, pkComData->kData.kHSocket)) { return; }
			pkSvr = SeGetRegSvrNodeBySvrName(&pkMagicNetS->kRegSvrList, acName);
			if(!pkSvr) { return; }
			if((int)strlen(pkSeSocket->acBindSvrName) != 0) { return; }
			if(SeGetRegSvrNodeBySvrName(&pkMagicNetS->kRegSvrList, pkSeSocket->acBindSvrName)) { return; }
			SeStrNcpy(pkSeSocket->acBindSvrName, sizeof(pkSeSocket->acBindSvrName), pkSvr->acName);

			pkComData = (struct SECOMMONDATA *)pkMagicNetS->pcRecvBuf;
			pkComData->iProco = MAGICNET_TO_SVR_CLIENT_CONNECT;
			pkComData->kData.kHSocket = pkSeSocket->kHSocket;
			pkComData->iBufLen = 0;
			SeNetCoreSend(&pkMagicNetS->kNetCore, pkSvr->kHSocket, (char*)pkComData, pkComData->iBufLen + (int)sizeof(struct SECOMMONDATA));
			pkMagicNetS->iSendNum++;
			return;
		}

		if(pkComData->iProco == SVR_TO_MAGICNET_CLOSE_CLIENT)
		{
			pkSvr = SeGetRegSvrNodeBySocket(&pkMagicNetS->kRegSvrList, pkComData->kData.kHSocket);
			if(pkSvr) { return; }
			SeNetCoreDisconnect(&pkMagicNetS->kNetCore, pkComData->kData.kHSocket);
		}

		if(pkComData->iProco == SVR_TO_MAGICNET_REG_SVR && riEvent == SENETCORE_EVENT_SOCKET_RECV_DATA)
		{
			if(((int)sizeof(struct SECOMMONDATA) + pkComData->iBufLen) != riLen) { SeNetCoreDisconnect(&pkMagicNetS->kNetCore, rkHSocket); return; }
			pkComData->kData.acName[sizeof(pkComData->kData.acName) - 1] = '\0';

			pkSvr = SeGetRegSvrNodeBySocket(&pkMagicNetS->kRegSvrList, rkHSocket);
			if(pkSvr) { SeNetCoreDisconnect(&pkMagicNetS->kNetCore, rkHSocket); return; }
			pkSvr = SeGetRegSvrNodeBySvrName(&pkMagicNetS->kRegSvrList, pkComData->kData.acName);
			if(pkSvr) { SeNetCoreDisconnect(&pkMagicNetS->kNetCore, rkHSocket); return; }

			pkSvr = SeAddRegSvrNode(&pkMagicNetS->kRegSvrList, pkComData->kData.acName, rkHSocket);
			if(!pkSvr) { SeNetCoreDisconnect(&pkMagicNetS->kNetCore, rkHSocket); return; }
		}

		if(pkComData->iProco == SVR_TO_MAGICNET_SENDTO_CLIENT && riEvent == SENETCORE_EVENT_SOCKET_RECV_DATA)
		{
			pkSvrWatchdog = SeGetRegSvrNodeBySvrName(&pkMagicNetS->kRegSvrList, acWatchdogName);
			if(!pkSvrWatchdog) { SeNetCoreDisconnect(&pkMagicNetS->kNetCore, rkHSocket); return; }

			pkSvr = SeGetRegSvrNodeBySocket(&pkMagicNetS->kRegSvrList, rkHSocket);
			if(!pkSvr) { SeNetCoreDisconnect(&pkMagicNetS->kNetCore, rkHSocket); return; }

			if(((int)sizeof(struct SECOMMONDATA) + pkComData->iBufLen) != riLen) { return; }
			if(pkComData->iBufLen < 0) { return; }

			pkSvr = SeGetRegSvrNodeBySocket(&pkMagicNetS->kRegSvrList, pkComData->kData.kHSocket);
			if(pkSvr) { SeNetCoreDisconnect(&pkMagicNetS->kNetCore, rkHSocket); return; }// not send to client,is send to svr
			
			SeNetCoreSend(&pkMagicNetS->kNetCore, pkComData->kData.kHSocket, (char*)pkComData + (int)sizeof(struct SECOMMONDATA), pkComData->iBufLen);
			pkMagicNetS->iSendNum++;
		}

		if(pkComData->iProco == SVR_TO_MAGICNET_SENDTO_SVR && riEvent == SENETCORE_EVENT_SOCKET_RECV_DATA)
		{
			pkSvrWatchdog = SeGetRegSvrNodeBySvrName(&pkMagicNetS->kRegSvrList, acWatchdogName);
			if(!pkSvrWatchdog) { SeNetCoreDisconnect(&pkMagicNetS->kNetCore, rkHSocket); return; }

			pkSvrMe = SeGetRegSvrNodeBySocket(&pkMagicNetS->kRegSvrList, rkHSocket);
			if(!pkSvrMe) { SeNetCoreDisconnect(&pkMagicNetS->kNetCore, rkHSocket); return; }

			if(((int)sizeof(struct SECOMMONDATA) + pkComData->iBufLen) != riLen) { return; }
			if(pkComData->iBufLen < 0) { return; }
			
			memset(acName, 0, sizeof(acName));
			pkComData->kData.acName[sizeof(pkComData->kData.acName) - 1] = '\0';
			SeStrNcpy(acName, sizeof(acName), pkComData->kData.acName);
			pkComData->kData.kHSocket = pkSvrMe->kHSocket;
			pkSvr = SeGetRegSvrNodeBySvrName(&pkMagicNetS->kRegSvrList, acName);
			if (!pkSvr) { return; }
			if(pkSvrMe->kHSocket == pkSvr->kHSocket) { return; } //me send to me?

			pkComData->iProco = MAGICNET_TO_SVR_RECV_DATA_FROM_SVR;
			SeNetCoreSend(&pkMagicNetS->kNetCore, pkSvr->kHSocket, (char*)pkComData, pkComData->iBufLen + (int)sizeof(struct SECOMMONDATA));
			pkMagicNetS->iSendNum++;
		}

		if(pkComData->iProco == SVR_TO_MAGICNET_GATE_STAT && riEvent == SENETCORE_EVENT_SOCKET_RECV_DATA)
		{
			pkSvrWatchdog = SeGetRegSvrNodeBySvrName(&pkMagicNetS->kRegSvrList, acWatchdogName);
			if(!pkSvrWatchdog) { SeNetCoreDisconnect(&pkMagicNetS->kNetCore, rkHSocket); return; }

			pkSvrMe = SeGetRegSvrNodeBySocket(&pkMagicNetS->kRegSvrList, rkHSocket);
			if(!pkSvrMe) { SeNetCoreDisconnect(&pkMagicNetS->kNetCore, rkHSocket); return; }

			if(((int)sizeof(struct SECOMMONDATA) + pkComData->iBufLen) != riLen) { return; }
			if(pkComData->iBufLen < 0) { return; }
			if(pkSvrMe->kHSocket == pkSvrWatchdog->kHSocket) { return; } //me send to me?

			pkComData->iProco = MAGICNET_TO_SVR_GATE_STAT;
			SeNetCoreSend(&pkMagicNetS->kNetCore, pkSvrWatchdog->kHSocket, (char*)pkComData, pkComData->iBufLen + (int)sizeof(struct SECOMMONDATA));
			pkMagicNetS->iSendNum++;
		}
	}
}

void SeMagicNetSProcess(struct SEMAGICNETS *pkMagicNetS)
{
	SeMagicNetSWork(pkMagicNetS);
}

bool SeMagicNetCInit(struct SEMAGICNETC *pkMagicNetC, char *pcLogName, int iTimeOut, unsigned short usInPort, int iLogLV)
{
	SeNetCoreInit(&pkMagicNetC->kNetCore, pcLogName, iTimeOut, 10000, iLogLV);
	pkMagicNetC->pcRecvBuf = (char*)SeMallocMem(MAX_RECV_BUF_LEN);
	pkMagicNetC->pcSendBuf = (char*)SeMallocMem(MAX_RECV_BUF_LEN);
	pkMagicNetC->llActive = SeTimeGetTickCount();
	pkMagicNetC->usInPort = usInPort;
	pkMagicNetC->kHScoket = 0;
	pkMagicNetC->pkGateStatFunc = 0;
	pkMagicNetC->ullTime = SeTimeGetTickCount();
	pkMagicNetC->iSendNum = 0;
	pkMagicNetC->iRecvNum = 0;
	memset(pkMagicNetC->acSvrName, 0, sizeof(pkMagicNetC->acSvrName));

	return true;
}

void SeMagicNetCFin(struct SEMAGICNETC *pkMagicNetC)
{
	SeFreeMem(pkMagicNetC->pcRecvBuf);
	SeFreeMem(pkMagicNetC->pcSendBuf);
	SeNetCoreFin(&pkMagicNetC->kNetCore);
	pkMagicNetC->kHScoket = 0;
}

void SeMagicNetCSetGateStatFunc(struct SEMAGICNETC *pkMagicNetC, MAGICNETENGINEGATESTAT	pkGateStatFunc)
{
	pkMagicNetC->pkGateStatFunc = pkGateStatFunc;
}

void SeMagicNetCSetWaitTime(struct SEMAGICNETC *pkMagicNetC, unsigned int uiWaitTime)
{
	SeNetCoreSetWaitTime(&pkMagicNetC->kNetCore, uiWaitTime);
}

bool SeMagicNetCReg(struct SEMAGICNETC *pkMagicNetC, const char *pcSvrName)
{
	int riLen;
	int rSSize;
	int rRSize;
	int riEvent;
	bool result;
	HSOCKET rkHSocket;
	HSOCKET rkListenHSocket;
	struct SECOMMONDATA *pkComData;

	assert((int)strlen(pcSvrName) > 0);
	assert(MAX_SVR_NAME_LEN > (int)strlen(pcSvrName));

	if(pkMagicNetC->kHScoket > 0) { return true; }
	pkMagicNetC->llActive = SeTimeGetTickCount();
	pkMagicNetC->kHScoket = SeNetCoreTCPClient(&pkMagicNetC->kNetCore, "127.0.0.1", pkMagicNetC->usInPort, 4, &SeGetHeader, &SeSetHeader);
	if(pkMagicNetC->kHScoket <= 0) { return false; }

	while(true)
	{
		riLen = MAX_RECV_BUF_LEN;
		result = SeNetCoreRead(&pkMagicNetC->kNetCore,
			&riEvent, &rkListenHSocket, &rkHSocket, pkMagicNetC->pcRecvBuf, &riLen, &rSSize, &rRSize);
		if(!result) { continue; }
		if(riEvent == SENETCORE_EVENT_SOCKET_IDLE) { continue; }
		assert(rkHSocket == pkMagicNetC->kHScoket);

		if(riEvent == SENETCORE_EVENT_SOCKET_CONNECT)
		{
			pkComData = (struct SECOMMONDATA *)pkMagicNetC->pcRecvBuf;
			pkComData->iProco = SVR_TO_MAGICNET_REG_SVR;
			memset(pkComData->kData.acName, 0, (int)sizeof(pkComData->kData.acName));
			SeStrNcpy(pkComData->kData.acName, (int)sizeof(pkComData->kData.acName), pcSvrName);
			SeStrNcpy(pkMagicNetC->acSvrName, sizeof(pkMagicNetC->acSvrName), pcSvrName);
			pkComData->iBufLen = 0;
			if(!SeNetCoreSend(&pkMagicNetC->kNetCore, pkMagicNetC->kHScoket, (char*)pkComData, (int)sizeof(struct SECOMMONDATA))) { return false; }
			return true;
		}

		if(riEvent == SENETCORE_EVENT_SOCKET_CONNECT_FAILED || riEvent == SENETCORE_EVENT_SOCKET_DISCONNECT) { pkMagicNetC->kHScoket = 0; return false; }
	}
	return false;
}

bool SeMagicNetCSendClient(struct SEMAGICNETC *pkMagicNetC, HSOCKET kHSocket, const char *pcBuf, int iLen)
{
	struct SECOMMONDATA *pkComData;

	pkComData = (struct SECOMMONDATA *)pkMagicNetC->pcSendBuf;
	pkComData->iProco = SVR_TO_MAGICNET_SENDTO_CLIENT;
	pkComData->kData.kHSocket = kHSocket;
	pkComData->iBufLen = iLen;
	memcpy(pkMagicNetC->pcSendBuf + (int)sizeof(struct SECOMMONDATA), pcBuf, pkComData->iBufLen);
	pkMagicNetC->iSendNum++;
	return SeNetCoreSend(&pkMagicNetC->kNetCore, pkMagicNetC->kHScoket, (char*)pkComData, pkComData->iBufLen + (int)sizeof(struct SECOMMONDATA));
}

void SeMagicNetCBindClientToSvr(struct SEMAGICNETC *pkMagicNetC, HSOCKET kHSocket, const char *pcSvrName)
{
	struct SECOMMONDATA *pkComData;

	pkComData = (struct SECOMMONDATA *)pkMagicNetC->pcSendBuf;
	pkComData->iProco = SVR_TO_MAGICNET_BIND_CLIENT;
	pkComData->kData.kHSocket = kHSocket;
	pkComData->iBufLen = (int)strlen(pcSvrName);
	memcpy(pkMagicNetC->pcSendBuf + (int)sizeof(struct SECOMMONDATA), pcSvrName, (int)strlen(pcSvrName));
	SeNetCoreSend(&pkMagicNetC->kNetCore, pkMagicNetC->kHScoket, (char*)pkComData, pkComData->iBufLen + (int)sizeof(struct SECOMMONDATA));
}

char *SePacketData(struct SEMAGICNETC *pkMagicNetC, bool bToClient, const char *pcSvrName, HSOCKET kHSocket, int iLen, int *riBegin)
{
	struct SECOMMONDATA *pkComData;

	if(iLen + (int)sizeof(struct SECOMMONDATA) >= MAX_RECV_BUF_LEN)
	{
		*riBegin = 0;
		return 0;
	}

	pkComData = (struct SECOMMONDATA *)pkMagicNetC->pcSendBuf;
	pkComData->iProco = bToClient ? SVR_TO_MAGICNET_SENDTO_CLIENT : SVR_TO_MAGICNET_SENDTO_SVR;

	if(bToClient)
	{
		pkComData->kData.kHSocket = kHSocket;
	}
	else
	{
		SeStrNcpy(pkComData->kData.acName, (int)sizeof(pkComData->kData.acName), pcSvrName);
	}

	pkComData->iBufLen = iLen;

	*riBegin = (int)sizeof(struct SECOMMONDATA);
	return pkMagicNetC->pcSendBuf;
}

bool SeMagicNetCSendPacket(struct SEMAGICNETC *pkMagicNetC, const char *pcBuf, int iLen)
{
	pkMagicNetC->iSendNum++;
	return SeNetCoreSend(&pkMagicNetC->kNetCore, pkMagicNetC->kHScoket, (char*)pcBuf, iLen);
}

void SeMagicNetCCloseClient(struct SEMAGICNETC *pkMagicNetC, HSOCKET kHSocket)
{
	struct SECOMMONDATA *pkComData;

	pkComData = (struct SECOMMONDATA *)pkMagicNetC->pcSendBuf;
	pkComData->iProco = SVR_TO_MAGICNET_CLOSE_CLIENT;
	pkComData->kData.kHSocket = kHSocket;
	pkComData->iBufLen = 0;
	SeNetCoreSend(&pkMagicNetC->kNetCore, pkMagicNetC->kHScoket, (char*)pkComData, (int)sizeof(struct SECOMMONDATA));
}

bool SeMagicNetCSendSvr(struct SEMAGICNETC *pkMagicNetC, const char *pcSvrName, const char *pcBuf, int iLen)
{
	struct SECOMMONDATA *pkComData;

	pkComData = (struct SECOMMONDATA *)pkMagicNetC->pcSendBuf;
	pkComData->iProco = SVR_TO_MAGICNET_SENDTO_SVR;
	memset(pkComData->kData.acName, 0, (int)sizeof(pkComData->kData.acName));
	SeStrNcpy(pkComData->kData.acName, (int)sizeof(pkComData->kData.acName), pcSvrName);
	pkComData->iBufLen = iLen;
	memcpy(pkMagicNetC->pcSendBuf + (int)sizeof(struct SECOMMONDATA), pcBuf, pkComData->iBufLen);
	pkMagicNetC->iSendNum++;
	return SeNetCoreSend(&pkMagicNetC->kNetCore, pkMagicNetC->kHScoket, (char*)pkComData, pkComData->iBufLen + (int)sizeof(struct SECOMMONDATA));
}

void SeMagicNetCSendStat(struct SEMAGICNETC *pkMagicNetC)
{
	unsigned long long ullTime;
	struct SEGATESTAT kGateStat;
	struct SECOMMONDATA *pkComDataGateStat;

	if(strlen(pkMagicNetC->acSvrName) <= 0)
	{
		return;
	}

	ullTime = SeTimeGetTickCount();

	if(pkMagicNetC->ullTime + STAT_TIME > ullTime)
	{
		return;
	}

	kGateStat.iSend = (pkMagicNetC->iSendNum*1000)/(int)(ullTime - pkMagicNetC->ullTime);
	kGateStat.iRecv = (pkMagicNetC->iRecvNum*1000)/(int)(ullTime - pkMagicNetC->ullTime);

	if(strcmp(pkMagicNetC->acSvrName, acWatchdogName) == 0)
	{
		if(pkMagicNetC->pkGateStatFunc)
		{
			pkMagicNetC->pkGateStatFunc(pkMagicNetC->acSvrName, kGateStat.iSend, kGateStat.iRecv);
		}
	}
	else
	{
		pkComDataGateStat = (struct SECOMMONDATA *)pkMagicNetC->pcSendBuf;
		pkComDataGateStat->iProco = SVR_TO_MAGICNET_GATE_STAT;
		pkComDataGateStat->iBufLen = (int)sizeof(struct SEGATESTAT);
		SeStrNcpy(pkComDataGateStat->kData.acName, sizeof(pkComDataGateStat->kData.acName), pkMagicNetC->acSvrName);
		memcpy(pkMagicNetC->pcSendBuf + (int)sizeof(struct SECOMMONDATA), &kGateStat, (int)sizeof(struct SEGATESTAT));
		SeMagicNetCSendPacket(pkMagicNetC, (char*)pkComDataGateStat, pkComDataGateStat->iBufLen + (int)sizeof(struct SECOMMONDATA));
	}

	pkMagicNetC->iRecvNum = 0;
	pkMagicNetC->iSendNum = 0;
	pkMagicNetC->ullTime = ullTime;
}

enum MAGIC_STATE SeMagicNetCRead(struct SEMAGICNETC *pkMagicNetC, HSOCKET *rkRecvHSocket, char **pcBuf, int *riBufLen)
{
	int riLen;
	int rSSize;
	int rRSize;
	int riEvent;
	bool result;
	HSOCKET rkHSocket;
	HSOCKET rkListenHSocket;
	struct SEGATESTAT *pkGateStat;
	struct SECOMMONDATA *pkComData;

	*pcBuf = pkMagicNetC->pcRecvBuf;
	*riBufLen = 0;

	if(pkMagicNetC->kHScoket <= 0) { SeTimeSleep(1); return MAGIC_SHUTDOWN_SVR; }

	if((pkMagicNetC->llActive + MAGICNET_TIME_OUT) <= SeTimeGetTickCount())
	{
		pkMagicNetC->llActive = SeTimeGetTickCount();
		pkComData = (struct SECOMMONDATA *)pkMagicNetC->pcRecvBuf;
		pkComData->iProco = SVR_TO_MAGICNET_ACTIVE;
		pkComData->iBufLen = 0;
		SeNetCoreSend(&pkMagicNetC->kNetCore, pkMagicNetC->kHScoket, (char*)pkComData, (int)sizeof(struct SECOMMONDATA));
	}

	SeMagicNetCSendStat(pkMagicNetC);

	riLen = MAX_RECV_BUF_LEN;
	result = SeNetCoreRead(&pkMagicNetC->kNetCore,
		&riEvent, &rkListenHSocket, &rkHSocket, pkMagicNetC->pcRecvBuf, &riLen, &rSSize, &rRSize);
	if(!result) { return MAGIC_IDLE_SVR_DATA; }
	if(riEvent == SENETCORE_EVENT_SOCKET_IDLE) { SeTimeSleep(1); return MAGIC_IDLE_SVR_DATA; }
	assert(rkHSocket == pkMagicNetC->kHScoket);
	if(riEvent == SENETCORE_EVENT_SOCKET_CONNECT) { assert(0 != 0); SeTimeSleep(1); return MAGIC_IDLE_SVR_DATA; }// no call here
	if(riEvent == SENETCORE_EVENT_SOCKET_CONNECT_FAILED) { assert(0 != 0); SeTimeSleep(1); return MAGIC_IDLE_SVR_DATA; }// no call here
	if(riEvent == SENETCORE_EVENT_SOCKET_DISCONNECT) { pkMagicNetC->kHScoket = 0; SeTimeSleep(1);return MAGIC_SHUTDOWN_SVR; }

	if(riEvent != SENETCORE_EVENT_SOCKET_RECV_DATA) { assert(0 != 0); SeTimeSleep(1); return MAGIC_IDLE_SVR_DATA; }// no call here
	assert(riLen >= (int)sizeof(struct SECOMMONDATA));
	pkComData = (struct SECOMMONDATA *)pkMagicNetC->pcRecvBuf;
	if(pkComData->iBufLen + (int)sizeof(struct SECOMMONDATA) != riLen) { return MAGIC_IDLE_SVR_DATA; }

	pkMagicNetC->iRecvNum++;

	if(pkComData->iProco == MAGICNET_TO_SVR_ACTIVE) { return MAGIC_IDLE_SVR_DATA; }

	if(pkComData->iProco == MAGICNET_TO_SVR_GATE_STAT)
	{
		if(!pkMagicNetC->pkGateStatFunc)
		{
			return MAGIC_IDLE_SVR_DATA;
		}
		pkGateStat = (struct SEGATESTAT *)((char*)pkComData + (int)sizeof(struct SECOMMONDATA));
		pkMagicNetC->pkGateStatFunc(pkComData->kData.acName, pkGateStat->iSend, pkGateStat->iRecv);
		return MAGIC_IDLE_SVR_DATA;
	}

	if(pkComData->iProco == MAGICNET_TO_SVR_CLIENT_CONNECT || pkComData->iProco == MAGICNET_TO_SVR_CLIENT_DISCONNECT)
	{
		*rkRecvHSocket = pkComData->kData.kHSocket;
		*pcBuf = (char*)pkComData + (int)sizeof(struct SECOMMONDATA);
		*riBufLen = pkComData->iBufLen;
		return pkComData->iProco == MAGICNET_TO_SVR_CLIENT_CONNECT ? MAGIC_CLIENT_CONNECT : MAGIC_CLIENT_DISCONNECT;
	}

	if(pkComData->iProco == MAGICNET_TO_SVR_RECV_DATA_FROM_SVR || pkComData->iProco == MAGICNET_TO_SVR_RECV_DATA_FROM_CLIENT)
	{
		*rkRecvHSocket = pkComData->kData.kHSocket;
		*pcBuf = (char*)pkComData + (int)sizeof(struct SECOMMONDATA);
		*riBufLen = pkComData->iBufLen;
		return pkComData->iProco == MAGICNET_TO_SVR_RECV_DATA_FROM_SVR ? MAGIC_RECV_DATA_FROM_SVR : MAGIC_RECV_DATA_FROM_CLIENT;
	}
	
	// no call here
	assert(0 != 0);
	SeTimeSleep(1);
	return MAGIC_IDLE_SVR_DATA;
}
