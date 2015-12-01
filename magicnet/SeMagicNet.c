#include "SeMagicNet.h"
#include "SeTool.h"
#include "SeTime.h"

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

struct REGSVRNODE *SeGetRegSvrNode(struct SEHASH *pkRegSvrList, int id)
{
	struct SEHASHNODE *pkHashNode;
	
	pkHashNode = SeHashGet(pkRegSvrList, id);
	if(!pkHashNode) { return 0; }
	return SE_CONTAINING_RECORD(pkHashNode, struct SEHASHNODE, kHashNode);
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

void SeFreeRegSvrNode(struct SEHASH *pkRegSvrList)
{
	struct SEHASHNODE *pkHashNode;
	struct REGSVRNODE *pkRegSvrNode;

	pkHashNode = SeHashPop(pkRegSvrList);
	while(pkHashNode)
	{
		pkRegSvrNode = SE_CONTAINING_RECORD(pkHashNode, struct SEHASHNODE, kHashNode);
		free(pkRegSvrNode);
		pkHashNode = SeHashPop(pkRegSvrList);
	}
}

bool SeMagicNetSInit(struct SEMAGICNETS *pkMagicNetS, unsigned short usMax, unsigned short usOutPort, unsigned short usInPort)
{
	SeNetCoreInit(&pkMagicNetS->kNetCore, "magicnet.log", usMax);
	SeHashInit(&pkMagicNetS->kRegSvrList, 1000);

	pkMagicNetS->kHScoketOut = SeNetCoreTCPListen(&pkMagicNetS->kNetCore, "0.0.0.0", usOutPort, 2, &SeGetHeader, &SeSetHeader);
	pkMagicNetS->kHScoketIn = SeNetCoreTCPListen(&pkMagicNetS->kNetCore, "127.0.0.1", usOutPort, 4, &SeGetHeader, &SeSetHeader);
	if(pkMagicNetS->kHScoketOut <= 0 || pkMagicNetS->kHScoketIn <= 0) { SeMagicNetSFin(&pkMagicNetS->kNetCore); return false; }

	return true;
}

void SeMagicNetSFin(struct SEMAGICNETS *pkMagicNetS)
{
	SeFreeRegSvrNode(&pkMagicNetS->kRegSvrList);
	SeNetCoreFin(&pkMagicNetS->kNetCore);
	SeHashFin(&pkMagicNetS->kRegSvrList);
}

void SeMagicNetSProcess(struct SEMAGICNETS *pkMagicNetS)
{
}
