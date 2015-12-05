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
	iTimeOut = luaL_checkinteger(L, 2);
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
	return 0;
}

static int MagicNetGateProcess(lua_State *L)
{
	SeMagicNetSProcess(&kMagicNetGate);
	return 0;
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
	iTimeOut = (unsigned short)luaL_checkinteger(L, 2);
	usInPort = (unsigned short)luaL_checkinteger(L, 3);

	bResult = SeMagicNetCInit(&kMagicNetSvr, (char*)pcLogName, iTimeOut, usInPort);
	lua_pushboolean(L, bResult);
	return 1;
}

static int MagicNetSvrFin(lua_State *L)
{
	SeMagicNetCFin(&kMagicNetSvr);
	return 0;
}

static int MagicNetSvrReg(lua_State *L)
{
	bool bResult;
	size_t seplen;
	const char *pcSvrName;

	seplen = 0;
	pcSvrName = luaL_checklstring(L, 1, &seplen);

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

	SeMagicNetCBindClientToSvr(&kMagicNetSvr, kHSocket, pcSvrName);
	return 0;
}

static int MagicNetSvrCloseClient(lua_State *L)
{
	HSOCKET kHSocket;

	kHSocket = luaL_checkinteger(L, 1);

	SeMagicNetCCloseClient(&kMagicNetSvr, kHSocket);
	return 0;
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
	seplen = 0;
	pcBuf = luaL_checklstring(L, 2, &seplen);
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

static const luaL_Reg magicnet[] = {
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
		{ NULL, NULL },
	};

int luaopen_magicnet(lua_State *L)
{
	luaL_newlib(L, magicnet);
	return 1;
}
