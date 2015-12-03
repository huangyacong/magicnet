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
	pkMagicNetS->pcBuf = (char*)malloc(MAX_RECV_BUF_LEN);

	pkMagicNetS->kHScoketOut = SeNetCoreTCPListen(&pkMagicNetS->kNetCore, "0.0.0.0", usOutPort, 2, &SeGetHeader, &SeSetHeader);
	pkMagicNetS->kHScoketIn = SeNetCoreTCPListen(&pkMagicNetS->kNetCore, "127.0.0.1", usOutPort, 4, &SeGetHeader, &SeSetHeader);
	if(pkMagicNetS->kHScoketOut <= 0 || pkMagicNetS->kHScoketIn <= 0) { SeMagicNetSFin(pkMagicNetS); return false; }

	return true;
}

void SeMagicNetSFin(struct SEMAGICNETS *pkMagicNetS)
{
	free(pkMagicNetS->pcBuf);
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
	char			*pcBuf;
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
			&riEvent, &rkListenHSocket, &rkHSocket, pkMagicNetS->pcBuf, &riLen, &rSSize, &rRSize);
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

		if(riEvent == SENETCORE_EVENT_SOCKET_CONNECT)
		{
		}
		else if(riEvent == SENETCORE_EVENT_SOCKET_DISCONNECT)
		{
		}
		else if(riEvent == SENETCORE_EVENT_SOCKET_RECV_DATA)
		{
		}
	}
	
	//  内网
	if(rkListenHSocket == pkMagicNetS->kHScoketIn)
	{
		if(riLen < sizeof(struct SECOMMONDATA)) { assert(0 != 0); return; }
		pkComData = (struct SECOMMONDATA *)pkMagicNetS->pcBuf;

		if(pkComData->iProco == SVR_TO_MAGICNET_REG_SVR && riEvent == SENETCORE_EVENT_SOCKET_RECV_DATA)
		{
			pkSvr = SeGetRegSvrNodeBySocket(&pkMagicNetS->kRegSvrList, rkHSocket);
			if(pkSvr) { SeNetCoreDisconnect(&pkMagicNetS->kNetCore, rkHSocket); return; }

			if((sizeof(struct SECOMMONDATA) + pkComData->iBufLen) != riLen) { SeNetCoreDisconnect(&pkMagicNetS->kNetCore, rkHSocket); return; }
			if(pkComData->iBufLen >= MAX_SVR_NAME_LEN || pkComData->iBufLen <= 0) { SeNetCoreDisconnect(&pkMagicNetS->kNetCore, rkHSocket); return; }

			pkComData->pcBuf = (char*)pkComData + sizeof(struct SECOMMONDATA);
			memcpy(acName, pkComData->pcBuf, pkComData->iBufLen);
			acName[pkComData->iBufLen - 1] = '\0';

			pkSvr = SeGetRegSvrNodeBySvrName(&pkMagicNetS->kRegSvrList, acName);
			if(pkSvr) { SeNetCoreDisconnect(&pkMagicNetS->kNetCore, rkHSocket); return; }
			
			if(strcmp(acName, acWatchdogName) == 0)
			{
				pkSvrWatchdog = SeGetRegSvrNodeBySvrName(&pkMagicNetS->kRegSvrList, acWatchdogName);
				if(pkSvrWatchdog) { SeNetCoreDisconnect(&pkMagicNetS->kNetCore, rkHSocket); return; }
			}

			pkSvr = SeAddRegSvrNode(&pkMagicNetS->kRegSvrList, acName, rkHSocket);
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

			if((sizeof(struct SECOMMONDATA) + pkComData->iBufLen) != riLen) { return; }
			if(pkComData->iBufLen < 0 || pkComData->iBufLen > (0xFFFF*2)) { return; }
			pkComData->pcBuf = (char*)pkComData + sizeof(struct SECOMMONDATA);
			
			if(pkComData->iProco == SVR_TO_MAGICNET_SENDTO_SVR)
			{
				
			}
			else
			{
				
			}
		}
	}
}
