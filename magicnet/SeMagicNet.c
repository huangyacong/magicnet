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
	struct SEHASHNODE		kHashNode;
	unsigned int			uiSvrNo;
	unsigned long long		llActive;
	HSOCKET					kHSocket;
	char					acName[MAX_SVR_NAME_LEN];
};

struct REGSVRNODE *SeGetRegSvrNode(struct SEHASH *pkRegSvrList, int id)
{
	struct SEHASHNODE *pkHashNode;
	
	pkHashNode = SeHashGet(pkRegSvrList, id);
	if(!pkHashNode) { return 0; }
	return SE_CONTAINING_RECORD(pkHashNode, struct REGSVRNODE, kHashNode);
}

struct REGSVRNODE *SeGetRegSvrNodeBySocket(struct SEHASH *pkRegSvrList, HSOCKET	kHSocket)
{
	struct SENODE *pkNode;
	struct REGSVRNODE *pkSvr;
	struct SEHASHNODE *pkHashNode;

	pkNode = pkRegSvrList->list.head;
	while(pkNode)
	{
		pkHashNode = SE_CONTAINING_RECORD(pkNode, struct SEHASHNODE, list);
		pkSvr = SE_CONTAINING_RECORD(pkHashNode, struct REGSVRNODE, kHashNode);
		if(pkSvr->kHSocket == kHSocket) { return pkSvr; }
		pkNode = pkNode->next;
	}

	return 0;
}

struct REGSVRNODE *SeAddRegSvrNode(struct SEHASH *pkRegSvrList, int id)
{
	struct REGSVRNODE *pkRegSvrNode;

	if(SeHashGet(pkRegSvrList, id)) { return 0; }
	pkRegSvrNode = (struct REGSVRNODE *)malloc(sizeof(struct REGSVRNODE));
	SeHashNodeInit(&pkRegSvrNode->kHashNode);
	SeHashAdd(pkRegSvrList, id, &pkRegSvrNode->kHashNode);
	return pkRegSvrNode;
}

void SeDelRegSvrNode(struct SEHASH *pkRegSvrList, int id)
{
	struct SEHASHNODE *pkHashNode;
	struct REGSVRNODE *pkRegSvrNode;

	pkHashNode = SeHashGet(pkRegSvrList, id);
	if (!pkHashNode) { return; }
	pkRegSvrNode = SE_CONTAINING_RECORD(pkHashNode, struct REGSVRNODE, kHashNode);
	SeHashRemove(pkRegSvrList, &pkRegSvrNode->kHashNode);
	free(pkRegSvrNode);
}

void SeFreeRegSvrNode(struct SEHASH *pkRegSvrList)
{
	struct SEHASHNODE *pkHashNode;
	struct REGSVRNODE *pkRegSvrNode;

	pkHashNode = SeHashPop(pkRegSvrList);
	while(pkHashNode)
	{
		pkRegSvrNode = SE_CONTAINING_RECORD(pkHashNode, struct REGSVRNODE, kHashNode);
		free(pkRegSvrNode);
		pkHashNode = SeHashPop(pkRegSvrList);
	}
}

bool SeMagicNetSInit(struct SEMAGICNETS *pkMagicNetS, unsigned short usMax, unsigned short usOutPort, unsigned short usInPort)
{
	SeNetCoreInit(&pkMagicNetS->kNetCore, "magicnet.log", usMax);
	SeHashInit(&pkMagicNetS->kRegSvrList, 1000);
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
	SeHashFin(&pkMagicNetS->kRegSvrList);
}

char acWatchdogName[] = "watchdog.";
#define UIWATCHDOGSVRNO SeStr2Hash(acWatchdogName, (int)strlen(acWatchdogName))

#define SVR_TO_MAGICNET_REG_SVR 0
#define SVR_TO_MAGICNET_SENDTO_SVR 1
#define SVR_TO_MAGICNET_SENDTO_CLIENT 2

#define MAGICNET_TO_SVR_CLIENT_CONNECT 0
#define MAGICNET_TO_SVR_CLIENT_DISCONNECT 1
#define MAGICNET_TO_SVR_RECV_DATA_FROM_SVR 2
#define MAGICNET_TO_SVR_RECV_DATA_FROM_CLIENT 3

struct SECOMMONDATA
{
	int		iProco;
	int		iDstSvrNo;
	int		iFlag;
	int		iBufLen;
	HSOCKET	kHSocket;
	char	*pcBuf;
};

void SeMagicNetSProcess(struct SEMAGICNETS *pkMagicNetS)
{
	int riLen;
	int rSSize;
	int rRSize;
	int riEvent;
	bool result;
	int	iDstSvrNo;
	char acName[256];
	HSOCKET rkHSocket;
	HSOCKET rkListenHSocket;
	struct REGSVRNODE *pkSvr;
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
		pkSvrWatchdog = SeGetRegSvrNode(&pkMagicNetS->kRegSvrList, UIWATCHDOGSVRNO);
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
			assert((sizeof(struct SECOMMONDATA) + pkComData->iBufLen) == riLen);
			assert(sizeof(acName) > pkComData->iBufLen);

			pkSvr = SeGetRegSvrNodeBySocket(&pkMagicNetS->kRegSvrList, rkHSocket);
			if(pkSvr) { SeNetCoreDisconnect(&pkMagicNetS->kNetCore, rkHSocket); return; }

			if((sizeof(struct SECOMMONDATA) + pkComData->iBufLen) != riLen) { SeNetCoreDisconnect(&pkMagicNetS->kNetCore, rkHSocket); return; }
			if(pkComData->iBufLen >= MAX_SVR_NAME_LEN || pkComData->iBufLen <= 0) { SeNetCoreDisconnect(&pkMagicNetS->kNetCore, rkHSocket); return; }

			pkComData->pcBuf = (char*)pkComData + sizeof(struct SECOMMONDATA);
			memcpy(acName, pkComData->pcBuf, pkComData->iBufLen);
			acName[pkComData->iBufLen - 1] = '\0';
			iDstSvrNo = SeStr2Hash(acName, (int)strlen(acName));
			
			if(strcmp(acName, acWatchdogName) == 0)
			{
				assert(iDstSvrNo == UIWATCHDOGSVRNO);
				pkSvrWatchdog = SeGetRegSvrNode(&pkMagicNetS->kRegSvrList, UIWATCHDOGSVRNO);
				if(pkSvrWatchdog) { SeNetCoreDisconnect(&pkMagicNetS->kNetCore, rkHSocket); return; }

				pkSvrWatchdog = SeAddRegSvrNode(&pkMagicNetS->kRegSvrList, UIWATCHDOGSVRNO);
				if(!pkSvrWatchdog) { SeNetCoreDisconnect(&pkMagicNetS->kNetCore, rkHSocket); return; }
				memset(pkSvrWatchdog->acName, 0, sizeof(pkSvrWatchdog->acName));

				pkSvrWatchdog->uiSvrNo = UIWATCHDOGSVRNO;
				pkSvrWatchdog->llActive = SeTimeGetTickCount();
				pkSvrWatchdog->kHSocket = rkHSocket;
				SeStrNcpy(pkSvrWatchdog->acName, sizeof(pkSvrWatchdog->acName), acName);
				assert(iDstSvrNo == SeStr2Hash(pkSvrWatchdog->acName, (int)strlen(pkSvrWatchdog->acName)));
				return;
			}
			else
			{
				pkSvr = SeAddRegSvrNode(&pkMagicNetS->kRegSvrList, iDstSvrNo);
				if(!pkSvr) { SeNetCoreDisconnect(&pkMagicNetS->kNetCore, rkHSocket); return; }
				memset(pkSvr->acName, 0, sizeof(pkSvr->acName));

				pkSvr->uiSvrNo = UIWATCHDOGSVRNO;
				pkSvr->llActive = SeTimeGetTickCount();
				pkSvr->kHSocket = rkHSocket;
				SeStrNcpy(pkSvr->acName, sizeof(pkSvr->acName), acName);
				assert(iDstSvrNo == SeStr2Hash(pkSvr->acName, (int)strlen(pkSvr->acName)));
				return;
			}
		}

		if(riEvent == SENETCORE_EVENT_SOCKET_DISCONNECT)
		{
			pkSvr = SeGetRegSvrNodeBySocket(&pkMagicNetS->kRegSvrList, rkHSocket);
			if(pkSvr) { SeHashRemove(&pkMagicNetS->kRegSvrList, &pkSvr->kHashNode); }
		}

		if((pkComData->iProco == SVR_TO_MAGICNET_SENDTO_SVR || pkComData->iProco == SVR_TO_MAGICNET_SENDTO_CLIENT) 
				&& riEvent == SENETCORE_EVENT_SOCKET_RECV_DATA)
		{
			pkSvrWatchdog = SeGetRegSvrNode(&pkMagicNetS->kRegSvrList, UIWATCHDOGSVRNO);
			if(!pkSvrWatchdog) { SeNetCoreDisconnect(&pkMagicNetS->kNetCore, rkHSocket); return; }

			pkSvr = SeGetRegSvrNodeBySocket(&pkMagicNetS->kRegSvrList, rkHSocket);
			if(!pkSvr) { SeNetCoreDisconnect(&pkMagicNetS->kNetCore, rkHSocket); return; }

			assert((sizeof(struct SECOMMONDATA) + pkComData->iBufLen) == riLen);
			if((sizeof(struct SECOMMONDATA) + pkComData->iBufLen) != riLen) { return; }
			if(pkComData->iBufLen < 0 || pkComData->iBufLen > (0xFFFF*2)) { return; }
			pkComData->pcBuf = (char*)pkComData + sizeof(struct SECOMMONDATA);
			
			if(pkComData->iProco == SVR_TO_MAGICNET_SENDTO_SVR)
			{
				pkSvr = SeGetRegSvrNode(&pkMagicNetS->kRegSvrList, pkComData->iDstSvrNo);
				if(pkSvr) { SeNetCoreSend(&pkMagicNetS->kNetCore, pkSvr->kHSocket, pkComData->pcBuf, pkComData->iBufLen); }
			}
			else
			{
				SeNetCoreSend(&pkMagicNetS->kNetCore, pkComData->kHSocket, pkComData->pcBuf, pkComData->iBufLen);
			}
		}
	}
}
