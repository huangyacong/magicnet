#include "ccmoduleLua.h"
#include "ServiceAgent.h"
#include "SeCrashDump.h"
#include "SeTimer.h"
#include <math.h>
#include <string>
#include <map>

static ServiceAgent* g_pkServiceAgentGate;

static void CoreNetInit()
{
	g_pkServiceAgentGate = NULL;
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

	if (g_pkServiceAgentGate)
		return 0;

	SeCrashDump(string(pcLogName) + string(".CrashDump"));
	g_pkServiceAgentGate = new ServiceAgent();
	g_pkServiceAgentGate->Init(pcLogName, iLogLV, usMax, iTimerCnt);

	lua_pushnil(L);
	return 1;
}

extern "C" int CoreNetFinAgentGate(lua_State *L)
{
	if (!g_pkServiceAgentGate)
		return 0;
	g_pkServiceAgentGate->StopServiceAgent();
	delete g_pkServiceAgentGate;
	g_pkServiceAgentGate = NULL;
	lua_pushnil(L);
	return 1;
}

extern "C" int CoreNetStartAgentGate(lua_State *L)
{
	if (!g_pkServiceAgentGate)
		return 0;
	g_pkServiceAgentGate->StartServiceAgent();
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
		return 1;
	}

	if (!IPService)
	{
		luaL_error(L, "IPService is NULL!");
		lua_pushboolean(L, false);
		return 1;
	}

	if (!IPServiceUnix)
	{
		luaL_error(L, "IPServiceIPServiceUnix is NULL!");
		lua_pushboolean(L, false);
		return 1;
	}

	if (!g_pkServiceAgentGate)
	{
		luaL_error(L, "not init  g_pkServiceAgentGate!");
		lua_pushboolean(L, false);
		return 1;
	}

	bResult = g_pkServiceAgentGate->Listen(IPRemote, PortRemote, IPService, PortService, IPServiceUnix, iTimeOut);

	if (!bResult)
	{
		luaL_error(L, "Listen failed!");
		lua_pushboolean(L, false);
		return 1;
	}

	lua_pushboolean(L, true);
	return 1;
}

extern "C" int CoreNetNetPack(lua_State *L)
{
	int iLen;
	size_t seplen;
	char acBuff[256];
	const char *pcBuf;
	unsigned short usProto;
	AgentServicePacket kPacket;

	seplen = 0;
	pcBuf = luaL_checklstring(L, 1, &seplen);
	if (!pcBuf) { luaL_error(L, "CoreNetNetPack pcBuf is NULL!"); return 0; }
	SeStrNcpy(kPacket.acSrcName, (int)sizeof(kPacket.acSrcName), pcBuf);

	seplen = 0;
	pcBuf = luaL_checklstring(L, 2, &seplen);
	if (!pcBuf) { luaL_error(L, "CoreNetNetPack pcBuf is NULL!"); return 0; }
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
		if (!pcBuf) { luaL_error(L, "CoreNetNetPack pcBuf is NULL!"); return 0; }
		SeStrNcpy(kPacket.acProto, (int)sizeof(kPacket.acProto), pcBuf);
	}

	iLen = NetPack(kPacket, (unsigned char*)acBuff, (int)sizeof(acBuff));
	if (iLen <= 0){ luaL_error(L, "CoreNetNetPack iLen is %d!", iLen); return 0; }
	lua_pushlstring(L, acBuff, iLen);
	return 1;
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
	if (!pcBuf) { luaL_error(L, "CoreNetNetUnPack pcBuf is NULL!"); return 0; }
	if ((int)seplen <= 0) { luaL_error(L, "CoreNetNetUnPack pcBuf is seplen=%d!", seplen); return 0; }

	iLen = NetUnPack(kPacket, (const unsigned char*)pcBuf, (int)seplen);
	if (iLen > (int)seplen || iLen <= 0) { luaL_error(L, "CoreNetNetUnPack pcBuf is error! %d", iLen); return 0; }

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

	seplen_key = 0;
	pcBuf_key = luaL_checklstring(L, 1, &seplen_key);
	if (!pcBuf_key) { luaL_error(L, "pcBuf_key is NULL!"); return 0; }

	seplen_name = 0;
	pcBuf_name = luaL_checklstring(L, 2, &seplen_name);
	if (!pcBuf_name) { luaL_error(L, "pcBuf_name is NULL!"); return 0; }

	std::string kMD5 = GenRegToken(pcBuf_key, pcBuf_name);

	lua_pushstring(L, kMD5.c_str());
	return 1;
}

#if (defined(_WIN32) || defined(WIN32))
extern "C" __declspec(dllexport) int luaopen_CoreNetAgent(lua_State *L)
#elif defined(__linux)
extern "C" int luaopen_CoreNetAgent(lua_State *L)
#endif
{
	CoreNetInit();

	// must use int64 number
	if(LUA_VERSION_NUM < 503) { luaL_error(L, "Lua ver must > 5.3"); return 0; }
	if(sizeof(lua_Integer) != 8) { luaL_error(L, "must use int64 for lua_Integer"); return 0; }
	luaL_checkversion(L);

	luaL_Reg reg[] = {
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

	lua_pushinteger(L, PTYPE_REG_ADD_SERVICE);
	lua_setfield(L, -2, "PTYPE_REG_ADD_SERVICE");

	lua_pushinteger(L, PTYPE_REG_DEL_SERVICE);
	lua_setfield(L, -2, "PTYPE_REG_DEL_SERVICE");

	lua_pushinteger(L, PTYPE_PING);
	lua_setfield(L, -2, "PTYPE_PING");

	lua_pushinteger(L, PTYPE_EXIT);
	lua_setfield(L, -2, "PTYPE_EXIT");
	
	lua_pushinteger(L, PTYPE_REMOTE_CONNECT);
	lua_setfield(L, -2, "PTYPE_REMOTE_CONNECT");

	lua_pushinteger(L, PTYPE_REMOTE_DISCONNECT);
	lua_setfield(L, -2, "PTYPE_REMOTE_DISCONNECT");

	lua_pushinteger(L, PTYPE_REMOTE_RECV_DATA);
	lua_setfield(L, -2, "PTYPE_REMOTE_RECV_DATA");

	lua_pushinteger(L, PTYPE_REMOTE_CLOSE);
	lua_setfield(L, -2, "PTYPE_REMOTE_CLOSE");

	lua_pushinteger(L, PTYPE_REGISTER_BACK);
	lua_setfield(L, -2, "PTYPE_REGISTER_BACK");

	return 1;
}
