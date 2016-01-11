#include "SeMagicNet.h"
#include "SeTime.h"

__declspec(dllexport) bool GateInit(char *pcLogName, int iTimeOut, unsigned short usMax, unsigned short usOutPort, unsigned short usInPort);

__declspec(dllexport) void GateFin();

__declspec(dllexport) void GateProcess();

__declspec(dllexport) bool SvrInit(char *pcLogName, int iTimeOut, unsigned short usInPort);

__declspec(dllexport) void SvrFin();

__declspec(dllexport) bool RegSvr(char *pcSvrName);

__declspec(dllexport) bool SvrSendClient(HSOCKET kHSocket, char *pcBuf, int iLen);

__declspec(dllexport) void SvrBindClient(HSOCKET kHSocket, char *pcSvrName);

__declspec(dllexport) void SvrCloseClient(HSOCKET kHSocket);

__declspec(dllexport) bool SvrSendSvr(char *pcSvrName, char *pcBuf, int iLen);

__declspec(dllexport) enum MAGIC_STATE SvrRead(HSOCKET *rkRecvHSocket, char *pcBuf, int *riBufLen);

__declspec(dllexport) void TimeSleep(unsigned long ulMillisecond);

__declspec(dllexport) unsigned long long TickCount();

