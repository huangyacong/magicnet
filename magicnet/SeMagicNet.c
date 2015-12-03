#include "SeMagicNet.h"
#include "SeTool.h"
#include "SeTime.h"

#define MAX_SVR_NAME_LEN 128
#define MAX_RECV_BUF_LEN 1024*1024*4

bool SeSetHeader(char* pcHeader, const int iheaderlen, const int ilen)
{
	if(iheaderlen == 2)
	{
		if(ilen < 0 || ilen > 0xFFFF) { return false; }
		pcHeader[0] = (ilen >> 8) & 0xff;
		pcHeader[1] = ilen & 0xff;
		return true;
	}

	if(iheaderlen == 4)
	{
		if(ilen < 0 || ilen > (0xFFFF*2)) { return false; }
		// 将int数值转换为占四个字节的byte数组，本方法适用于(低位在前，高位在后)的顺序。
		pcHeader[3] = ((ilen & 0xFF000000) >> 24);
		pcHeader[2] = ((ilen & 0x00FF0000) >> 16);
		pcHeader[1] = ((ilen & 0x0000FF00) >> 8);
		pcHeader[0] = ((ilen & 0x000000FF));
		return true;
	}

	return false;
}

bool SeGetHeader(const char* pcHeader, const int iheaderlen, int *ilen)
{
	if(iheaderlen == 2)
	{
		*ilen = (unsigned short)(pcHeader[0] << 8 | pcHeader[1]);
		if(*ilen < 0 || *ilen > 0xFFFF) { return false; }
		return true;
	}

	if(iheaderlen == 4)
	{
		// byte数组中取int数值，本方法适用于(低位在前，高位在后)的顺序
		*ilen = (int)((pcHeader[0] & 0xFF) | ((pcHeader[1] << 8) & 0xFF00) | ((pcHeader[2] << 16) & 0xFF0000) | ((pcHeader[3] << 24) & 0xFF000000));
		if(*ilen < 0 || *ilen > (0xFFFF*2)) { return false; }
		return true;
	}

	return false;
}

struct REGSVRNODE
{
	struct SENODE			kNode;
	unsigned long long		llActive;
	HSOCKET					kHSocket;
	char					acName[MAX_SVR_NAME_LEN];
};

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

	pkRegSvrNode = (struct REGSVRNODE *)malloc(sizeof(struct REGSVRNODE));
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
		if(kHSocket == pkSvr->kHSocket) { SeListRemove(pkRegSvrList, &pkSvr->kNode); free(pkSvr); break; }
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
		free(pkRegSvrNode);
		pkNode = SeListHeadPop(pkRegSvrList);
	}
}

bool SeMagicNetSInit(struct SEMAGICNETS *pkMagicNetS, unsigned short usMax, unsigned short usOutPort, unsigned short usInPort)
{
	SeNetCoreInit(&pkMagicNetS->kNetCore, "magicnet.log", usMax);
	SeListInit(&pkMagicNetS->kRegSvrList);
	pkMagicNetS->pcRecvBuf = (char*)malloc(MAX_RECV_BUF_LEN);
	pkMagicNetS->pcSendBuf = (char*)malloc(MAX_RECV_BUF_LEN);

	pkMagicNetS->kHScoketOut = SeNetCoreTCPListen(&pkMagicNetS->kNetCore, "0.0.0.0", usOutPort, 2, &SeGetHeader, &SeSetHeader);
	pkMagicNetS->kHScoketIn = SeNetCoreTCPListen(&pkMagicNetS->kNetCore, "127.0.0.1", usOutPort, 4, &SeGetHeader, &SeSetHeader);
	if(pkMagicNetS->kHScoketOut <= 0 || pkMagicNetS->kHScoketIn <= 0) { SeMagicNetSFin(pkMagicNetS); return false; }

	return true;
}

void SeMagicNetSFin(struct SEMAGICNETS *pkMagicNetS)
{
	free(pkMagicNetS->pcRecvBuf);
	free(pkMagicNetS->pcSendBuf);
	SeFreeRegSvrNode(&pkMagicNetS->kRegSvrList);
	SeNetCoreFin(&pkMagicNetS->kNetCore);
}

#define SVR_TO_MAGICNET_REG_SVR 0
#define SVR_TO_MAGICNET_SENDTO_SVR 1
#define SVR_TO_MAGICNET_SENDTO_CLIENT 2

#define MAGICNET_TO_SVR_CLIENT_CONNECT 0
#define MAGICNET_TO_SVR_CLIENT_DISCONNECT 1
#define MAGICNET_TO_SVR_RECV_DATA_FROM_SVR 2
#define MAGICNET_TO_SVR_RECV_DATA_FROM_CLIENT 3

union COMMDATA
{
	HSOCKET			kHSocket;
	char			acName[MAX_SVR_NAME_LEN];
};

struct SECOMMONDATA
{
	int				iProco;
	union COMMDATA	kData;
	int				iBufLen;
};

char acWatchdogName[] = "watchdog.";

void SeMagicNetSProcess(struct SEMAGICNETS *pkMagicNetS)
{
	int riLen;
	int rSSize;
	int rRSize;
	int riEvent;
	bool result;
	HSOCKET rkHSocket;
	HSOCKET rkListenHSocket;
	struct REGSVRNODE *pkSvr;
	char acName[MAX_SVR_NAME_LEN];
	struct SECOMMONDATA *pkComData;
	struct REGSVRNODE *pkSvrWatchdog;
	
	riLen = MAX_RECV_BUF_LEN;
	result = SeNetCoreRead(&pkMagicNetS->kNetCore, 
		&riEvent, &rkListenHSocket, &rkHSocket, pkMagicNetS->pcRecvBuf, &riLen, &rSSize, &rRSize);
	if(!result) { return; }
	if(riEvent == SENETCORE_EVENT_SOCKET_IDLE) { SeTimeSleep(1); return; }
	
	// 外网 
	if(rkListenHSocket == pkMagicNetS->kHScoketOut)
	{	
		// watchdog is working? 
		pkSvrWatchdog = SeGetRegSvrNodeBySvrName(&pkMagicNetS->kRegSvrList, acWatchdogName);
		if(!pkSvrWatchdog)
		{
			SeNetCoreDisconnect(&pkMagicNetS->kNetCore, rkHSocket);
			return;
		}

		if(riEvent == SENETCORE_EVENT_SOCKET_CONNECT || riEvent == SENETCORE_EVENT_SOCKET_DISCONNECT)
		{
			pkComData = (struct SECOMMONDATA *)pkMagicNetS->pcSendBuf;
			pkComData->iProco = riEvent == SENETCORE_EVENT_SOCKET_CONNECT ? MAGICNET_TO_SVR_CLIENT_CONNECT : MAGICNET_TO_SVR_CLIENT_DISCONNECT;
			pkComData->kData.kHSocket = rkHSocket;
			pkComData->iBufLen = 0;
			SeNetCoreSend(&pkMagicNetS->kNetCore, pkSvrWatchdog->kHSocket, (char*)pkComData, pkComData->iBufLen + (int)sizeof(struct SECOMMONDATA));
		}

		if(riEvent == SENETCORE_EVENT_SOCKET_RECV_DATA)
		{
			pkComData = (struct SECOMMONDATA *)pkMagicNetS->pcSendBuf;
			pkComData->iProco = riEvent == MAGICNET_TO_SVR_RECV_DATA_FROM_CLIENT;
			pkComData->kData.kHSocket = rkHSocket;
			pkComData->iBufLen = riLen;
			memcpy((char*)pkComData + (int)sizeof(struct SECOMMONDATA), pkMagicNetS->pcRecvBuf, pkComData->iBufLen);
			SeNetCoreSend(&pkMagicNetS->kNetCore, pkSvrWatchdog->kHSocket, (char*)pkComData, pkComData->iBufLen + (int)sizeof(struct SECOMMONDATA));
		}
	}
	
	//  内网
	if(rkListenHSocket == pkMagicNetS->kHScoketIn)
	{
		if(riLen < (int)sizeof(struct SECOMMONDATA)) { assert(0 != 0); return; }
		pkComData = (struct SECOMMONDATA *)pkMagicNetS->pcRecvBuf;

		if(pkComData->iProco == SVR_TO_MAGICNET_REG_SVR && riEvent == SENETCORE_EVENT_SOCKET_RECV_DATA)
		{
			if(((int)sizeof(struct SECOMMONDATA) + pkComData->iBufLen) != riLen) { SeNetCoreDisconnect(&pkMagicNetS->kNetCore, rkHSocket); return; }
			pkComData->kData.acName[MAX_SVR_NAME_LEN - 1] = '\0';

			pkSvr = SeGetRegSvrNodeBySocket(&pkMagicNetS->kRegSvrList, rkHSocket);
			if (pkSvr) { SeNetCoreDisconnect(&pkMagicNetS->kNetCore, rkHSocket); return; }
			pkSvr = SeGetRegSvrNodeBySvrName(&pkMagicNetS->kRegSvrList, pkComData->kData.acName);
			if(pkSvr) { SeNetCoreDisconnect(&pkMagicNetS->kNetCore, rkHSocket); return; }

			pkSvr = SeAddRegSvrNode(&pkMagicNetS->kRegSvrList, pkComData->kData.acName, rkHSocket);
			if(!pkSvr) { SeNetCoreDisconnect(&pkMagicNetS->kNetCore, rkHSocket); return; }
		}

		if(riEvent == SENETCORE_EVENT_SOCKET_DISCONNECT)
		{
			SeDelRegSvrNode(&pkMagicNetS->kRegSvrList, rkHSocket);
		}

		if((pkComData->iProco == SVR_TO_MAGICNET_SENDTO_SVR || pkComData->iProco == SVR_TO_MAGICNET_SENDTO_CLIENT) 
				&& riEvent == SENETCORE_EVENT_SOCKET_RECV_DATA)
		{
			pkSvrWatchdog = SeGetRegSvrNodeBySvrName(&pkMagicNetS->kRegSvrList, acWatchdogName);
			if(!pkSvrWatchdog) { SeNetCoreDisconnect(&pkMagicNetS->kNetCore, rkHSocket); return; }

			pkSvr = SeGetRegSvrNodeBySocket(&pkMagicNetS->kRegSvrList, rkHSocket);
			if(!pkSvr) { SeNetCoreDisconnect(&pkMagicNetS->kNetCore, rkHSocket); return; }

			if(((int)sizeof(struct SECOMMONDATA) + pkComData->iBufLen) != riLen) { return; }
			if(pkComData->iBufLen < 0 || pkComData->iBufLen > (0xFFFF*2)) { return; }
			
			if(pkComData->iProco == SVR_TO_MAGICNET_SENDTO_SVR)
			{
				memset(acName, 0, sizeof(acName));
				pkComData->kData.acName[MAX_SVR_NAME_LEN - 1] = '\0';
				SeStrNcpy(acName, sizeof(acName), pkComData->kData.acName);
				pkComData->kData.kHSocket = pkSvr->kHSocket;
				pkSvr = SeGetRegSvrNodeBySvrName(&pkMagicNetS->kRegSvrList, acName);
				if (!pkSvr) { return; }

				pkComData->iProco = MAGICNET_TO_SVR_RECV_DATA_FROM_SVR;
				SeNetCoreSend(&pkMagicNetS->kNetCore, pkSvr->kHSocket, (char*)pkComData, riLen);
			}
			else
			{
				SeNetCoreSend(&pkMagicNetS->kNetCore, pkComData->kData.kHSocket, (char*)pkComData + (int)sizeof(struct SECOMMONDATA), pkComData->iBufLen);
			}
		}
	}
}

bool SeMagicNetCInit(struct SEMAGICNETC *pkMagicNetC, unsigned short usInPort)
{
	SeNetCoreInit(&pkMagicNetC->kNetCore, "magicnetsvr.log", 1);
	pkMagicNetC->pcRecvBuf = (char*)malloc(MAX_RECV_BUF_LEN);
	pkMagicNetC->pcSendBuf = (char*)malloc(MAX_RECV_BUF_LEN);
	pkMagicNetC->llActive = SeTimeGetTickCount();

	pkMagicNetC->kHScoket = SeNetCoreTCPClient(&pkMagicNetC->kNetCore, "127.0.0.1", usInPort, 4, &SeGetHeader, &SeSetHeader);
	if(pkMagicNetC->kHScoket <= 0) { SeMagicNetCFin(pkMagicNetC); return false; }

	return true;
}

void SeMagicNetCFin(struct SEMAGICNETC *pkMagicNetC)
{
	free(pkMagicNetC->pcRecvBuf);
	free(pkMagicNetC->pcSendBuf);
	SeNetCoreFin(&pkMagicNetC->kNetCore);
	pkMagicNetC->kHScoket = 0;
}

