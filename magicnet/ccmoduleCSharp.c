#include "ccmoduleCSharp.h"

static struct SEMAGICNETS kMagicNetGate;
static struct SEMAGICNETC kMagicNetSvr;

bool GateInit(char *pcLogName, int iTimeOut, unsigned short usMax, unsigned short usOutPort, unsigned short usInPort, int iLogLV)
{
	return SeMagicNetSInit(&kMagicNetGate, pcLogName, iTimeOut, usMax, false, "0.0.0.0", usOutPort, usInPort, iLogLV);
}

void GateFin()
{
	SeMagicNetSFin(&kMagicNetGate);
}

void GateProcess()
{
	SeMagicNetSProcess(&kMagicNetGate);
}

bool SvrInit(char *pcLogName, int iTimeOut, unsigned short usInPort, int iLogLV)
{
	return SeMagicNetCInit(&kMagicNetSvr, pcLogName, iTimeOut, usInPort, iLogLV);
}

void SvrFin()
{
	SeMagicNetCFin(&kMagicNetSvr);
}

bool RegSvr(char *pcSvrName)
{
	return SeMagicNetCReg(&kMagicNetSvr, (const char *)pcSvrName);
}

bool SvrSendClient(HSOCKET kHSocket, char *pcBuf, int iLen)
{
	return SeMagicNetCSendClient(&kMagicNetSvr, kHSocket, (const char *)pcBuf, iLen);
}

void SvrBindClient(HSOCKET kHSocket, char *pcSvrName)
{
	SeMagicNetCBindClientToSvr(&kMagicNetSvr, kHSocket, (const char *)pcSvrName);
}

void SvrCloseClient(HSOCKET kHSocket)
{
	SeMagicNetCCloseClient(&kMagicNetSvr, kHSocket);
}

bool SvrSendSvr(char *pcSvrName, char *pcBuf, int iLen)
{
	return SeMagicNetCSendSvr(&kMagicNetSvr, (const char *)pcSvrName, (const char *)pcBuf, iLen);
}

enum MAGIC_STATE SvrRead(HSOCKET *rkRecvHSocket, char *pcBuf, int *riBufLen)
{
	char *pcRecvBuf;
	enum MAGIC_STATE result;

	pcRecvBuf = 0;
	result = SeMagicNetCRead(&kMagicNetSvr, rkRecvHSocket, &pcRecvBuf, riBufLen);
	memcpy(pcBuf, pcRecvBuf, *riBufLen);
	return result;
}

void TimeSleep(unsigned long ulMillisecond)
{
	SeTimeSleep(ulMillisecond);
}

unsigned long long TickCount()
{
	return SeTimeGetTickCount();
}
