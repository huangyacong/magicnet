#include "ccmoduleLua.h"
#include "SeCrashDump.h"
#include "SeTimer.h"
#include <math.h>
#include <string>
#include <map>

static SeTimer g_kSeTimer;
static int g_iCoreWaitTime;
static char g_acBuf[1024*1024*4];
static struct SENETCORE g_kNetore;
static struct MsgIDStat g_kMsgIDStat;
static std::list<std::string> g_kLinkFile;


static void ResetMsgIDStat()
{
	g_kMsgIDStat.iSendNum = 0;
	g_kMsgIDStat.ullSendByteNum = 0;
	g_kMsgIDStat.iRecvNum = 0;
	g_kMsgIDStat.ullRecvByteNum = 0;
	g_kMsgIDStat.iPrintNum = 0;
	g_kMsgIDStat.ullPrintByteNum = 0;

	g_kMsgIDStat.ullDelayStatTime = 5000;
	g_kMsgIDStat.ullStatTime = SeTimeGetTickCount();
}

extern "C" int CoreNetInit(lua_State *L)
{
	int iLogLV;
	bool bPrint;
	size_t seplen;
	int iTimerCnt;
	const char *pcLogName;
	unsigned short usMax;
	
	seplen = 0;
	pcLogName = luaL_checklstring(L, 1, &seplen);
	if (!pcLogName) { luaL_error(L, "pcLogName is NULL!"); return 0; }

	usMax = (unsigned short)luaL_checkinteger(L, 2);
	iTimerCnt = (int)luaL_checkinteger(L, 3);
	bPrint = lua_toboolean(L, 4) == 1 ? true : false;

	iLogLV = LT_SPLIT | LT_ERROR | LT_WARNING | LT_INFO | LT_DEBUG | LT_CRITICAL | LT_SOCKET | LT_RESERROR | LT_NOHEAD;
	iLogLV |= bPrint ? (iLogLV | LT_PRINT) : iLogLV;

	ResetMsgIDStat();
	
	g_iCoreWaitTime = -1;
	SeCrashDump(string(pcLogName) + string(".dmp"));
	SeNetCoreInit(&g_kNetore, (char*)pcLogName, usMax, iTimerCnt, iLogLV);
	SeNetCoreSetWaitTime(&g_kNetore, g_iCoreWaitTime);

	lua_pushnil(L);
	return 1;
}

extern "C" int CoreNetFin(lua_State *L)
{
#if defined(__linux)
	std::list<std::string>::iterator itr = g_kLinkFile.begin();
	for (; itr != g_kLinkFile.end(); itr++)
	{
		unlink((*itr).c_str());
		SeLogWrite(&g_kNetore.kLog, LT_INFO, true, "unlink %s", (*itr).c_str());
	}
#endif
	g_kLinkFile.clear();
	SeNetCoreFin(&g_kNetore);
	lua_pushnil(L);
	return 1;
}

extern "C" int CoreNetTCPListen(lua_State *L)
{
	int iDoMain;
	int iTimeOut;
	bool bNoDelay;
	bool bReusePort;
	size_t seplen;
	bool bBigHeader;
	const char *pcIP;
	HSOCKET kHoscket;
	unsigned short usPort;

	seplen = 0;
	pcIP = luaL_checklstring(L, 1, &seplen);
	if (!pcIP) { luaL_error(L, "pcIP is NULL!"); return 0; }

	usPort = (unsigned short)luaL_checkinteger(L, 2);
	iTimeOut = (int)luaL_checkinteger(L, 3);
	bBigHeader = lua_toboolean(L, 4) == 1 ? true : false;
	iDoMain = (int)luaL_checkinteger(L, 5);
	bReusePort = lua_toboolean(L, 6) == 1 ? true : false;
	bNoDelay = lua_toboolean(L, 7) == 1 ? true : false;
	
	kHoscket = SeNetCoreTCPListen(&g_kNetore, iDoMain, bReusePort, pcIP, usPort, bBigHeader ? 4 : 2, bNoDelay, iTimeOut, SeNetSreamGetHeader, SeNetSreamSetHeader);

	if (kHoscket != 0 && iDoMain == SE_DOMAIN_UNIX)
	{
		g_kLinkFile.push_back(std::string(pcIP));
		SeLogWrite(&g_kNetore.kLog, LT_INFO, true, "link %s", pcIP);
	}

	lua_pushinteger(L, kHoscket);
	return 1;
}

extern "C" int CoreNetTCPClient(lua_State *L)
{
	int iDoMain;
	int iTimeOut;
	size_t seplen;
	bool bNoDelay;
	bool bBigHeader;
	const char *pcIP;
	HSOCKET kHoscket;
	int iConnectTimeOut;
	unsigned short usPort;

	seplen = 0;
	pcIP = luaL_checklstring(L, 1, &seplen);
	if (!pcIP) { luaL_error(L, "pcIP is NULL!"); return 0; }

	usPort = (unsigned short)luaL_checkinteger(L, 2);
	iTimeOut = (int)luaL_checkinteger(L, 3);
	iConnectTimeOut = (int)luaL_checkinteger(L, 4);
	bBigHeader = lua_toboolean(L, 5) == 1 ? true : false;
	iDoMain = (int)luaL_checkinteger(L, 6);
	bNoDelay = lua_toboolean(L, 7) == 1 ? true : false;

	kHoscket = SeNetCoreTCPClient(&g_kNetore, iDoMain, pcIP, usPort, bBigHeader ? 4 : 2, bNoDelay, iTimeOut, iConnectTimeOut, SeNetSreamGetHeader, SeNetSreamSetHeader);

	lua_pushinteger(L, kHoscket);
	return 1;
}

extern "C" int CoreNetTCPSend(lua_State *L)
{
	bool bResult;
	size_t seplen;
	HSOCKET kHSocket;
	const char *pcBuf;
	size_t seplenHead;
	const char *pcBufHead;
	struct SENETSTREAMBUF kBuf[2];

	kHSocket = luaL_checkinteger(L, 1);

	seplenHead = 0;
	pcBufHead = luaL_checklstring(L, 2, &seplenHead);
	if (!pcBufHead) { luaL_error(L, "pcBufHead is NULL!"); return 0; }

	seplen = 0;
	pcBuf = luaL_checklstring(L, 3, &seplen);
	if (!pcBuf) { luaL_error(L, "pcBuf is NULL!"); return 0; }

	kBuf[0].pcBuf = pcBufHead;
	kBuf[0].iBufLen = (int)seplenHead;
	kBuf[1].pcBuf = pcBuf;
	kBuf[1].iBufLen = (int)seplen;
	bResult = SeNetCoreSendExtend(&g_kNetore, kHSocket, kBuf, (int)(sizeof(kBuf) / sizeof(struct SENETSTREAMBUF)));

	g_kMsgIDStat.iSendNum++;
	g_kMsgIDStat.ullSendByteNum += (int)seplen + (int)seplenHead;

	lua_pushboolean(L, bResult);
	return 1;
}

extern "C" int CoreNetTCPBroadcast(lua_State *L)
{
	int index;
	size_t seplen;
	const char *pcBuf;
	size_t seplenHead;
	const char *pcBufHead;
	struct SENETSTREAMBUF kBuf[2];

	index = 1;
	if (!lua_istable(L, index)){ luaL_error(L, "#arg1 must be table!"); return 0; }

	seplenHead = 0;
	pcBufHead = luaL_checklstring(L, 2, &seplenHead);
	if (!pcBufHead) { luaL_error(L, "pcBufHead is NULL!"); return 0; }

	seplen = 0;
	pcBuf = luaL_checklstring(L, 3, &seplen);
	if (!pcBuf) { luaL_error(L, "pcBuf is NULL!"); return 0; }

	kBuf[0].pcBuf = pcBufHead;
	kBuf[0].iBufLen = (int)seplenHead;
	kBuf[1].pcBuf = pcBuf;
	kBuf[1].iBufLen = (int)seplen;

	/* table is in the stack at index 'index' */
	lua_pushnil(L);  /* first key  如果index是负数，此时table的索引位置变化了：假如原来是-1，现在变成-2了*/
	while (lua_next(L, index) != 0) {
		/* uses 'key' (at index -2) and 'value' (at index -1) */
		//printf("%d - %s\n", lua_tointeger(L, -1), lua_tostring(L, -2));
		if (lua_isinteger(L, -1) != 0)
		{
			SeNetCoreSendExtend(&g_kNetore, lua_tointeger(L, -1), kBuf, (int)(sizeof(kBuf) / sizeof(struct SENETSTREAMBUF)));
		}
		/* removes 'value'; keeps 'key' for next iteration */
		lua_pop(L, 1);
	}

	lua_pushnil(L);
	return 1;
}

extern "C" int CoreNetTCPClose(lua_State *L)
{
	HSOCKET kHSocket;

	kHSocket = luaL_checkinteger(L, 1);

	SeNetCoreDisconnect(&g_kNetore, kHSocket);

	lua_pushnil(L);
	return 1;
}

extern "C" int CoreNetRead(lua_State *L)
{
	int iRecvSize;
	int iSSize = 0;
	int iRSize = 0;
	HSOCKET kHSocket = 0;
	bool bResult = false;
	bool bHasData = false;
	HSOCKET kListenHSocket = 0;
	int iLen = (int)sizeof(g_acBuf);
	int iEvent = (int)SENETCORE_EVENT_SOCKET_IDLE;

	do {
		bResult = SeNetCoreRead(&g_kNetore, &iEvent, &kListenHSocket, &kHSocket, g_acBuf, &iLen, &iSSize, &iRSize);

		if (bResult && iEvent == SENETCORE_EVENT_SOCKET_IDLE)
		{
			if (g_iCoreWaitTime == 0)
			{
				SeTimeSleep(1);
			}
			continue;
		}
	} while (!bResult);

	bHasData = (iEvent != SENETCORE_EVENT_SOCKET_CONNECT && iEvent != SENETCORE_EVENT_SOCKET_RECV_DATA) ? false : true;
	iRecvSize = !bHasData ? 0 : iLen;

	if (iRecvSize < 0 || iLen < 0 || iLen > (int)sizeof(g_acBuf)) {
		luaL_error(L, "CoreNetRead error. iEvent=%d iRecvSize=%d iLen=%d", iEvent, iRecvSize, iLen);
		return 0;
	}

	g_kMsgIDStat.iRecvNum++;
	g_kMsgIDStat.ullRecvByteNum += iRecvSize;

	lua_newtable(L);
	lua_pushinteger(L, iEvent);
	lua_rawseti(L, -2, 1);
	lua_pushinteger(L, kListenHSocket);
	lua_rawseti(L, -2, 2);
	lua_pushinteger(L, kHSocket);
	lua_rawseti(L, -2, 3);

	if (bHasData){
		lua_pushlstring(L, g_acBuf, iRecvSize);
		lua_rawseti(L, -2, 4);
	}

	return 1;
}

extern "C" int CoreNetReport(lua_State *L)
{
	int iTime = 1;
	unsigned long long ullStatTime = 0;
	unsigned long long ullNow = SeTimeGetTickCount();

	int iSendNum = 0;
	unsigned long long ullSendSpeed = 0;
	int iRecvNum = 0;
	unsigned long long ullRecvSpeed = 0;
	int iPrintNum = 0;
	unsigned long long ullPrintByteNum = 0;

	ullStatTime = (unsigned long long)luaL_checkinteger(L, 1);
	ullStatTime = ullStatTime <= 0 ? g_kMsgIDStat.ullDelayStatTime : ullStatTime;

	if (ullNow > g_kMsgIDStat.ullStatTime && (ullStatTime + g_kMsgIDStat.ullStatTime) <= ullNow)
	{
		iTime = (int)(ullNow - g_kMsgIDStat.ullStatTime);
		iSendNum = (g_kMsgIDStat.iSendNum * 1000) / (int)iTime;
		ullSendSpeed = (unsigned long long)((unsigned long long)(g_kMsgIDStat.ullSendByteNum * 1000) / (unsigned long long)iTime);
		iRecvNum = (g_kMsgIDStat.iRecvNum * 1000) / (int)iTime;
		ullRecvSpeed = (unsigned long long)((unsigned long long)(g_kMsgIDStat.ullRecvByteNum * 1000) / (unsigned long long)iTime);
		iPrintNum = (g_kMsgIDStat.iPrintNum * 1000) / (int)iTime;
		ullPrintByteNum = (unsigned long long)((unsigned long long)(g_kMsgIDStat.ullPrintByteNum * 1000) / (unsigned long long)iTime);

		ResetMsgIDStat();

		lua_newtable(L);
		lua_pushinteger(L, iSendNum);
		lua_rawseti(L, -2, 1);
		lua_pushinteger(L, ullSendSpeed);
		lua_rawseti(L, -2, 2);
		lua_pushinteger(L, iRecvNum);
		lua_rawseti(L, -2, 3);
		lua_pushinteger(L, ullRecvSpeed);
		lua_rawseti(L, -2, 4);
		lua_pushinteger(L, iPrintNum);
		lua_rawseti(L, -2, 5);
		lua_pushinteger(L, ullPrintByteNum);
		lua_rawseti(L, -2, 6);
		lua_pushinteger(L, g_kSeTimer.GetTimerCount());
		lua_rawseti(L, -2, 7);

		return 1;
	}

	lua_pushnil(L);
	return 1;
}

extern "C" int CoreNetAddTimer(lua_State *L)
{
	long long llTimerId;
	int iDealyTimeMillSec;

	iDealyTimeMillSec = (int)luaL_checkinteger(L, 1);

	llTimerId = g_kSeTimer.SetTimer(iDealyTimeMillSec);

	lua_pushinteger(L, llTimerId);
	return 1;
}

extern "C" int CoreNetDelTimer(lua_State *L)
{
	long long llTimerId;

	llTimerId = (long long)luaL_checkinteger(L, 1);

	g_kSeTimer.DelTimer(llTimerId);

	lua_pushnil(L);
	return 1;
}

extern "C" int CoreNetGetTimeOutId(lua_State *L)
{
	long long llTimerId;

	llTimerId = g_kSeTimer.GetTimeOutId(SeTimeGetTickCount());

	lua_pushinteger(L, llTimerId);
	return 1;
}

extern "C" int CoreNetGetOS(lua_State *L)
{
#ifdef __linux
	lua_pushstring(L, "Linux");
	return 1;
#elif (defined(_WIN32) || defined(WIN32))
	lua_pushstring(L, "Windows");
	return 1;
#endif
	lua_pushstring(L, "Unknow");
	return 1;
}

#if (defined(_WIN32) || defined(WIN32))
extern "C" __declspec(dllexport) int luaopen_CoreNet(lua_State *L)
#elif defined(__linux)
extern "C" int luaopen_CoreNet(lua_State *L)
#endif
{
	// must use int64 number
	if(LUA_VERSION_NUM < 503) { luaL_error(L, "Lua ver must > 5.3"); return 0; }
	if(sizeof(lua_Integer) != 8) { luaL_error(L, "must use int64 for lua_Integer"); return 0; }
	luaL_checkversion(L);

	luaL_Reg reg[] = {
		{ "Init", CoreNetInit },
		{ "Fin", CoreNetFin },
		{ "TCPListen", CoreNetTCPListen },
		{ "TCPClient", CoreNetTCPClient },
		{ "TCPSend", CoreNetTCPSend },
		{ "TCPBroadcast", CoreNetTCPBroadcast },
		{ "TCPClose", CoreNetTCPClose },
		{ "Read", CoreNetRead },
		{ "Report", CoreNetReport }, 
		{ "AddTimer", CoreNetAddTimer },
		{ "DelTimer", CoreNetDelTimer },
		{ "GetTimeOutId", CoreNetGetTimeOutId }, 
		{ "GetOS", CoreNetGetOS }, 
		{ NULL, NULL },
	};

	luaL_newlib(L, reg);

	lua_pushinteger(L, SENETCORE_EVENT_SOCKET_CONNECT);
	lua_setfield(L, -2, "SOCKET_CONNECT");

	lua_pushinteger(L, SENETCORE_EVENT_SOCKET_CONNECT_FAILED);
	lua_setfield(L, -2, "SOCKET_CONNECT_FAILED");

	lua_pushinteger(L, SENETCORE_EVENT_SOCKET_DISCONNECT);
	lua_setfield(L, -2, "SOCKET_DISCONNECT");

	lua_pushinteger(L, SENETCORE_EVENT_SOCKET_RECV_DATA);
	lua_setfield(L, -2, "SOCKET_RECV_DATA");

	lua_pushinteger(L, SENETCORE_EVENT_SOCKET_TIMER);
	lua_setfield(L, -2, "SOCKET_TIMER");

	lua_pushinteger(L, SE_DOMAIN_INET);
	lua_setfield(L, -2, "DOMAIN_INET");

	lua_pushinteger(L, SE_DOMAIN_INET6);
	lua_setfield(L, -2, "DOMAIN_INET6");

	lua_pushinteger(L, SE_DOMAIN_UNIX);
	lua_setfield(L, -2, "DOMAIN_UNIX");

	return 1;
}
