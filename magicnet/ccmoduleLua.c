#include "ccmoduleLua.h"

static int MagicNetGateInit(lua_State *L)
{
	bool bResult;
	size_t seplen;
	const char *pcLogName;
	int iTimeOut;
	unsigned short usMax;
	unsigned short usOutPort;
	unsigned short usInPort;
	
	seplen = 0;
	pcLogName = luaL_checklstring(L, 1, &seplen);
	if(!pcLogName) { luaL_error(L, "pcLogName is NULL!"); return 0;}
	iTimeOut = (int)luaL_checkinteger(L, 2);
	usMax = (unsigned short)luaL_checkinteger(L, 3);
	usOutPort = (unsigned short)luaL_checkinteger(L, 4);
	usInPort = (unsigned short)luaL_checkinteger(L, 5);

	bResult = SeMagicNetSInit(&kMagicNetGate, (char*)pcLogName, iTimeOut, usMax, usOutPort, usInPort);
	lua_pushboolean(L, bResult);
	return 1;
}

static int MagicNetGateFin(lua_State *L)
{
	SeMagicNetSFin(&kMagicNetGate);
	lua_pushboolean(L, true);
	return 1;
}

static int MagicNetGateProcess(lua_State *L)
{
	SeMagicNetSProcess(&kMagicNetGate);
	lua_pushboolean(L, true);
	return 1;
}

static int MagicNetSvrInit(lua_State *L)
{
	bool bResult;
	size_t seplen;
	const char *pcLogName;
	int iTimeOut;
	unsigned short usInPort;

	seplen = 0;
	pcLogName = luaL_checklstring(L, 1, &seplen);
	if(!pcLogName) { luaL_error(L, "pcLogName is NULL!"); return 0;}
	iTimeOut = (unsigned short)luaL_checkinteger(L, 2);
	usInPort = (unsigned short)luaL_checkinteger(L, 3);

	bResult = SeMagicNetCInit(&kMagicNetSvr, (char*)pcLogName, iTimeOut, usInPort);
	lua_pushboolean(L, bResult);
	return 1;
}

static int MagicNetSvrFin(lua_State *L)
{
	SeMagicNetCFin(&kMagicNetSvr);
	lua_pushboolean(L, true);
	return 1;
}

static int MagicNetSvrReg(lua_State *L)
{
	bool bResult;
	size_t seplen;
	const char *pcSvrName;

	seplen = 0;
	pcSvrName = luaL_checklstring(L, 1, &seplen);
	if(!pcSvrName) { luaL_error(L, "pcSvrName is NULL!"); return 0;}

	bResult = SeMagicNetCReg(&kMagicNetSvr, pcSvrName);
	lua_pushboolean(L, bResult);
	return 1;
}

static int MagicNetSvrSendClient(lua_State *L)
{
	bool bResult;
	size_t seplen;
	HSOCKET kHSocket;
	const char *pcBuf;
	int iLen;

	kHSocket = luaL_checkinteger(L, 1);
	seplen = 0;
	pcBuf = luaL_checklstring(L, 2, &seplen);
	if(!pcBuf) { luaL_error(L, "pcBuf is NULL!"); return 0;}
	iLen = (int)seplen;

	bResult = SeMagicNetCSendClient(&kMagicNetSvr, kHSocket, pcBuf, iLen);
	lua_pushboolean(L, bResult);
	return 1;
}

static int MagicNetSvrBindClient(lua_State *L)
{
	size_t seplen;
	HSOCKET kHSocket;
	const char *pcSvrName;

	kHSocket = luaL_checkinteger(L, 1);
	seplen = 0;
	pcSvrName = luaL_checklstring(L, 2, &seplen);
	if(!pcSvrName) { luaL_error(L, "pcSvrName is NULL!"); return 0;}

	SeMagicNetCBindClientToSvr(&kMagicNetSvr, kHSocket, pcSvrName);
	lua_pushboolean(L, true);
	return 1;
}

static int MagicNetSvrCloseClient(lua_State *L)
{
	HSOCKET kHSocket;

	kHSocket = luaL_checkinteger(L, 1);

	SeMagicNetCCloseClient(&kMagicNetSvr, kHSocket);
	lua_pushboolean(L, true);
	return 1;
}

static int MagicNetSvrSendSvr(lua_State *L)
{	
	bool bResult;
	size_t seplen;
	const char *pcBuf;
	const char *pcSvrName;
	int iLen;
	
	seplen = 0;
	pcSvrName = luaL_checklstring(L, 1, &seplen);
	if(!pcSvrName) { luaL_error(L, "pcSvrName is NULL!"); return 0;}
	seplen = 0;
	pcBuf = luaL_checklstring(L, 2, &seplen);
	if(!pcBuf) { luaL_error(L, "pcBuf is NULL!"); return 0;}
	iLen = (int)seplen;

	bResult = SeMagicNetCSendSvr(&kMagicNetSvr, pcSvrName, pcBuf, iLen);
	lua_pushboolean(L, bResult);
	return 1;
}

static int MagicNetSvrRead(lua_State *L)
{
	enum MAGIC_STATE result;
	HSOCKET rkRecvHSocket;
	char *pcBuf;
	int riBufLen;
	
	rkRecvHSocket = 0;
	riBufLen = 0;
	pcBuf = 0;
	result = SeMagicNetCRead(&kMagicNetSvr, &rkRecvHSocket, &pcBuf, &riBufLen);
	
	lua_pushinteger(L, result);
	lua_pushinteger(L, rkRecvHSocket);
	lua_pushlstring(L, riBufLen > 0 ? pcBuf : "", riBufLen);
	return 3;
}

static int MagicTimeSleep(lua_State *L)
{
	int iCounter;

	iCounter = luaL_checkinteger(L, 1);
	SeTimeSleep(iCounter);
	
	lua_pushboolean(L, true);
	return 1;
}

static int MagicTimeGetTickCount(lua_State *L)
{
	unsigned long long ullTime;
	ullTime = SeTimeGetTickCount();
	lua_pushinteger(L, ullTime);
	return 1;
}

#if (defined(_WIN32) || defined(WIN32))
__declspec(dllexport) int luaopen_magicnet(lua_State *L)
#elif defined(__linux)
int luaopen_magicnet(lua_State *L)
#endif
{
	// must use int64 number
	if(LUA_VERSION_NUM < 503) { luaL_error(L, "Lua ver must > 5.3"); return 0; }
	if(sizeof(lua_Integer) != 8) { luaL_error(L, "must use int64 for lua_Integer"); return 0; }
	luaL_checkversion(L);
	luaL_Reg l[] = {
		{"GateInit", MagicNetGateInit},
		{"GateFin", MagicNetGateFin},
		{"GateProcess", MagicNetGateProcess},
		{"SvrInit", MagicNetSvrInit},
		{"SvrFin", MagicNetSvrFin},
		{"RegSvr", MagicNetSvrReg},
		{"SvrSendClient", MagicNetSvrSendClient},
		{"SvrBindClient", MagicNetSvrBindClient},
		{"SvrCloseClient", MagicNetSvrCloseClient},
		{"SvrSendSvr", MagicNetSvrSendSvr},
		{"SvrRead", MagicNetSvrRead},
		{"TimeSleep", MagicTimeSleep},
		{"TimeGetTickCount", MagicTimeGetTickCount},
		{ NULL, NULL },
	};

	luaL_newlib(L, l);

	lua_pushnumber(L, MAGIC_SHUTDOWN_SVR);
	lua_setfield(L, -2, "MAGIC_SHUTDOWN_SVR");

	lua_pushnumber(L, MAGIC_IDLE_SVR_DATA);
	lua_setfield(L, -2, "MAGIC_IDLE_SVR_DATA");

	lua_pushnumber(L, MAGIC_CLIENT_CONNECT);
	lua_setfield(L, -2, "MAGIC_CLIENT_CONNECT");

	lua_pushnumber(L, MAGIC_CLIENT_DISCONNECT);
	lua_setfield(L, -2, "MAGIC_CLIENT_DISCONNECT");

	lua_pushnumber(L, MAGIC_RECV_DATA_FROM_SVR);
	lua_setfield(L, -2, "MAGIC_RECV_DATA_FROM_SVR");

	lua_pushnumber(L, MAGIC_RECV_DATA_FROM_CLIENT);
	lua_setfield(L, -2, "MAGIC_RECV_DATA_FROM_CLIENT");
	return 1;
}
