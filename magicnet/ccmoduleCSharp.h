#include "SeMagicNet.h"
#include "SeTime.h"

bool GateInit(char *pcLogName, int iTimeOut, unsigned short usMax, unsigned short usOutPort, unsigned short usInPort);

void GateFin();

void GateProcess();

bool SvrInit(char *pcLogName, int iTimeOut, unsigned short usInPort);

void SvrFin();

bool RegSvr(char *pcSvrName);

bool SvrSendClient(HSOCKET kHSocket, char *pcBuf, int iLen);

void SvrBindClient(HSOCKET kHSocket, char *pcSvrName);

void SvrCloseClient(HSOCKET kHSocket);

bool SvrSendSvr(char *pcSvrName, char *pcBuf, int iLen);

enum MAGIC_STATE SvrRead(HSOCKET *rkRecvHSocket, char *pcBuf, int *riBufLen);

void TimeSleep(unsigned long ulMillisecond);

unsigned long long TickCount();

