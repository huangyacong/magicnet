#include "SeMagicNet.h"
#include "SeTime.h"

#if (defined(_WIN32) || defined(WIN32))
#define DLLEXPORT __declspec(dllexport)
#elif defined(__linux)
#define DLLEXPORT 
#endif

DLLEXPORT bool GateInit(char *pcLogName, int iTimeOut, unsigned short usMax, unsigned short usOutPort, unsigned short usInPort);

DLLEXPORT void GateFin();

DLLEXPORT void GateProcess();

DLLEXPORT bool SvrInit(char *pcLogName, int iTimeOut, unsigned short usInPort);

DLLEXPORT void SvrFin();

DLLEXPORT bool RegSvr(char *pcSvrName);

DLLEXPORT bool SvrSendClient(HSOCKET kHSocket, char *pcBuf, int iLen);

DLLEXPORT void SvrBindClient(HSOCKET kHSocket, char *pcSvrName);

DLLEXPORT void SvrCloseClient(HSOCKET kHSocket);

DLLEXPORT bool SvrSendSvr(char *pcSvrName, char *pcBuf, int iLen);

DLLEXPORT enum MAGIC_STATE SvrRead(HSOCKET *rkRecvHSocket, char *pcBuf, int *riBufLen);

DLLEXPORT void TimeSleep(unsigned long ulMillisecond);

DLLEXPORT unsigned long long TickCount();

