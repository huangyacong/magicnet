#include "ccmoduleLua.h"
#include "ServiceAgent.h"
#include "SeTimer.h"
#include <math.h>
#include <string>
#include <map>

static SeTimer g_kSeTimer;
static int g_iCoreWaitTime;
static char g_acBuf[1024*1024*4];
static struct SENETCORE g_kNetore;
static struct MsgIDStat g_kMsgIDStat;
static ServiceAgent* g_pkServiceAgentGate;
static std::list<std::string> g_kLinkFile;

static void CoreNet_Init()
{
	g_pkServiceAgentGate = NULL;
}

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


extern "C" int CoreNetInitAgentGate(lua_State *L)
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

	g_pkServiceAgentGate = new ServiceAgent();
	g_pkServiceAgentGate->Init(pcLogName, iLogLV, usMax, iTimerCnt);

	lua_pushnil(L);
	return 1;
}

extern "C" int CoreNetFinAgentGate(lua_State *L)
{
	if (g_pkServiceAgentGate)
	{
		g_pkServiceAgentGate->StopServiceAgent();
		delete g_pkServiceAgentGate;
		g_pkServiceAgentGate = NULL;
	}

	lua_pushnil(L);
	return 1;
}

extern "C" int CoreNetStartAgentGate(lua_State *L)
{
	if (!g_pkServiceAgentGate) 
	{ 
		luaL_error(L, "g_pkServiceAgentGate is NULL!"); 
		lua_pushboolean(L, false);
		return 0; 
	}

	g_pkServiceAgentGate->CreateThreadAndRunServiceAgent();

	lua_pushboolean(L, true);
	return 1;
}

extern "C" int CoreNetAgentGateListen(lua_State *L)
{
	bool bResult;
	const char* IPRemote;
	int PortRemote;
	const char* IPService;
	int PortService;
	const char* IPServiceUnix;
	int iTimeOut;

	if (!g_pkServiceAgentGate)
	{
		luaL_error(L, "g_pkServiceAgentGate is NULL!");
		lua_pushboolean(L, false);
		return 0;
	}

	IPRemote = luaL_checkstring(L, 1);
	PortRemote = (int)luaL_checkinteger(L, 2);
	IPService = luaL_checkstring(L, 3);
	PortService = (int)luaL_checkinteger(L, 4);
	IPServiceUnix = luaL_checkstring(L, 5);
	iTimeOut = (int)luaL_checkinteger(L, 6);

	if (!IPRemote)
	{
		luaL_error(L, "IPRemote is NULL!");
		lua_pushboolean(L, false);
		return 0;
	}

	if (!IPService)
	{
		luaL_error(L, "IPService is NULL!");
		lua_pushboolean(L, false);
		return 0;
	}

	if (!IPServiceUnix)
	{
		luaL_error(L, "IPServiceIPServiceUnix is NULL!");
		lua_pushboolean(L, false);
		return 0;
	}

	bResult = g_pkServiceAgentGate->Listen(IPRemote, PortRemote, IPService, PortService, IPServiceUnix, iTimeOut);

	if (!bResult)
	{
		luaL_error(L, "Listen failed!");
		lua_pushboolean(L, false);
		return 0;
	}

	lua_pushboolean(L, true);
	return 1;
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

	iRecvSize = (iEvent != SENETCORE_EVENT_SOCKET_CONNECT && iEvent != SENETCORE_EVENT_SOCKET_RECV_DATA) ? 0 : iLen;

	g_kMsgIDStat.iRecvNum++;
	g_kMsgIDStat.ullRecvByteNum += iRecvSize;

	lua_newtable(L);
	lua_pushinteger(L, iEvent);
	lua_rawseti(L, -2, 1);
	lua_pushinteger(L, kListenHSocket);
	lua_rawseti(L, -2, 2);
	lua_pushinteger(L, kHSocket);
	lua_rawseti(L, -2, 3);
	lua_pushlstring(L, g_acBuf, iRecvSize);
	lua_rawseti(L, -2, 4);

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

extern "C" int CoreNetNetPack(lua_State *L)
{
	int iLen;
	size_t seplen;
	const char *pcBuf;
	unsigned short usProto;
	AgentServicePacket kPacket;

	seplen = 0;
	pcBuf = luaL_checklstring(L, 1, &seplen);
	if (!pcBuf) { luaL_error(L, "pcBuf is NULL!"); return 0; }
	SeStrNcpy(kPacket.acSrcName, (int)sizeof(kPacket.acSrcName), pcBuf);

	seplen = 0;
	pcBuf = luaL_checklstring(L, 2, &seplen);
	if (!pcBuf) { luaL_error(L, "pcBuf is NULL!"); return 0; }
	SeStrNcpy(kPacket.acDstName, (int)sizeof(kPacket.acDstName), pcBuf);

	kPacket.eType = (AGENTSERVICE_PTYPE)luaL_checkinteger(L, 3);
	kPacket.ullSessionId = (unsigned long long)luaL_checkinteger(L, 4);

	if (kPacket.eType == PTYPE_REMOTE || kPacket.eType == PTYPE_REMOTE_RECV_DATA)
	{
		usProto = (unsigned short)luaL_checkinteger(L, 5);
		SeStrNcpy(kPacket.acProto, (int)sizeof(kPacket.acProto), SeUnSignedShortToA(usProto).c_str());
	}
	else
	{
		seplen = 0;
		pcBuf = luaL_checklstring(L, 5, &seplen);
		if (!pcBuf) { luaL_error(L, "pcBuf is NULL!"); return 0; }
		SeStrNcpy(kPacket.acProto, (int)sizeof(kPacket.acProto), pcBuf);
	}

	seplen = 0;
	pcBuf = luaL_checklstring(L, 6, &seplen);
	if (!pcBuf) { luaL_error(L, "pcBuf is NULL!"); return 0; }

	iLen = NetPack(kPacket, (unsigned char*)g_acBuf, (int)sizeof(g_acBuf));
	lua_pushlstring(L, g_acBuf, iLen);
	lua_pushlstring(L, pcBuf, seplen);
	return 2;
}

extern "C" int CoreNetNetUnPack(lua_State *L)
{
	int iLen;
	size_t seplen;
	const char *pcBuf;
	unsigned short usProto;
	AgentServicePacket kPacket;

	seplen = 0;
	pcBuf = luaL_checklstring(L, 1, &seplen);
	if (!pcBuf) { luaL_error(L, "pcBuf is NULL!"); return 0; }

	iLen = NetUnPack(kPacket, (const unsigned char*)pcBuf, (int)seplen);
	if (iLen > (int)seplen) { luaL_error(L, "pcBuf is error!"); return 0; }

	pcBuf = pcBuf + iLen;
	iLen = (int)seplen - iLen;

	lua_pushlstring(L, kPacket.acSrcName, (int)strlen(kPacket.acSrcName));
	lua_pushlstring(L, kPacket.acDstName, (int)strlen(kPacket.acDstName));
	lua_pushinteger(L, kPacket.eType);
	lua_pushinteger(L, kPacket.ullSessionId);

	if (kPacket.eType == PTYPE_REMOTE || kPacket.eType == PTYPE_REMOTE_RECV_DATA)
	{
		usProto = (unsigned short)SeAToInt(kPacket.acProto);
		lua_pushinteger(L, usProto);
	}
	else
	{
		lua_pushlstring(L, kPacket.acProto, (int)strlen(kPacket.acProto));
	}

	lua_pushlstring(L, pcBuf, iLen);
	return 6;
}

extern "C" int CoreNetGenRegToken(lua_State *L)
{
	size_t seplen_name, seplen_key;
	const char *pcBuf_name, *pcBuf_key;

	seplen_name = 0;
	pcBuf_name = luaL_checklstring(L, 1, &seplen_name);
	if (!pcBuf_name) { luaL_error(L, "pcBuf_name is NULL!"); return 0; }

	seplen_key = 0;
	pcBuf_key = luaL_checklstring(L, 2, &seplen_key);
	if (!pcBuf_key) { luaL_error(L, "pcBuf_key is NULL!"); return 0; }

	std::string kMD5 = GenRegToken(pcBuf_key, pcBuf_name);

	lua_pushstring(L, kMD5.c_str());
	return 1;
}

#if (defined(_WIN32) || defined(WIN32))
extern "C" __declspec(dllexport) int luaopen_CoreNet(lua_State *L)
#elif defined(__linux)
extern "C" int luaopen_CoreNet(lua_State *L)
#endif
{
	CoreNet_Init();

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
		{ "NetPack", CoreNetNetPack },
		{ "NetUnPack", CoreNetNetUnPack },
		{ "GenRegToken", CoreNetGenRegToken }, 
		{ "InitAgentGate", CoreNetInitAgentGate }, 
		{ "FinAgentGate", CoreNetFinAgentGate }, 
		{ "StartAgentGate", CoreNetStartAgentGate }, 
		{ "AgentGateListen", CoreNetAgentGateListen },
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

	lua_pushinteger(L, SE_DOMAIN_UNIX);
	lua_setfield(L, -2, "DOMAIN_UNIX");

	lua_pushinteger(L, PTYPE_RESPONSE);
	lua_setfield(L, -2, "PTYPE_RESPONSE");

	lua_pushinteger(L, PTYPE_CALL);
	lua_setfield(L, -2, "PTYPE_CALL");

	lua_pushinteger(L, PTYPE_REMOTE);
	lua_setfield(L, -2, "PTYPE_REMOTE");

	lua_pushinteger(L, PTYPE_COMMON);
	lua_setfield(L, -2, "PTYPE_COMMON");

	lua_pushinteger(L, PTYPE_REGISTER_KEY);
	lua_setfield(L, -2, "PTYPE_REGISTER_KEY");

	lua_pushinteger(L, PTYPE_REGISTER);
	lua_setfield(L, -2, "PTYPE_REGISTER");
	
	lua_pushinteger(L, PTYPE_PING);
	lua_setfield(L, -2, "PTYPE_PING");
	
	lua_pushinteger(L, PTYPE_REMOTE_CONNECT);
	lua_setfield(L, -2, "PTYPE_REMOTE_CONNECT");

	lua_pushinteger(L, PTYPE_REMOTE_DISCONNECT);
	lua_setfield(L, -2, "PTYPE_REMOTE_DISCONNECT");

	lua_pushinteger(L, PTYPE_REMOTE_RECV_DATA);
	lua_setfield(L, -2, "PTYPE_REMOTE_RECV_DATA");

	return 1;
}
