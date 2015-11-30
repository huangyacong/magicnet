#include "SeMagicNet.h"

bool SeSetHeader(char* pcHeader, const int iheaderlen, const int ilen)
{
	if(iheaderlen == 2)
	{
		pcHeader[0] = (ilen >> 8) & 0xff;
		pcHeader[1] = ilen & 0xff;
		return true;
	}

	if(iheaderlen == 4)
	{
		// ��int��ֵת��Ϊռ�ĸ��ֽڵ�byte���飬������������(��λ��ǰ����λ�ں�)��˳��
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
		*ilen = pcHeader[0] << 8 | pcHeader[1];
		return true;
	}

	if(iheaderlen == 4)
	{
		// byte������ȡint��ֵ��������������(��λ��ǰ����λ�ں�)��˳��
		*ilen = (int)((pcHeader[0] & 0xFF) | ((pcHeader[1] << 8) & 0xFF00) | ((pcHeader[2] << 16) & 0xFF0000) | ((pcHeader[3] << 24) & 0xFF000000));
		return true;
	}

	return false;
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
	SeNetCoreFin(&pkMagicNetS->kNetCore);
	SeHashFin(&pkMagicNetS->kRegSvrList);
}
