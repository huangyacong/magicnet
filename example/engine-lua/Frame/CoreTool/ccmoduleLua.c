#include "ccmoduleLua.h"
#include "SeThread.h"
#include "SeBase64.h"
#include "SeSha1.h"
#include "SeTool.h"
#include "SeMD5.h"

static int CoreToolSleep(lua_State *L)
{
	int iCounter;

	iCounter = (int)luaL_checkinteger(L, 1);
	SeTimeSleep(iCounter);

	lua_pushnil(L);
	return 1;
}

static int CoreToolGetTickCount(lua_State *L)
{
	unsigned long long ullTime;

	ullTime = SeTimeGetTickCount();
	lua_pushinteger(L, ullTime);
	return 1;
}

static int CoreToolMD5(lua_State *L)
{
	size_t seplen;
	const char *pcText;
	unsigned char buffer[33];

	seplen = 0;
	pcText = luaL_checklstring(L, 1, &seplen);
	if (!pcText) { luaL_error(L, "pcText is NULL!"); return 0; }

	SeMD5((char*)buffer, (const char*)pcText, (unsigned int)seplen);
	lua_pushstring(L, (const char*)buffer);
	return 1;
}

static int CoreToolStr2Hash(lua_State *L)
{
	size_t seplen;
	const char *pcText;
	unsigned int uiRet;

	seplen = 0;
	pcText = luaL_checklstring(L, 1, &seplen);
	if (!pcText) { luaL_error(L, "pcText is NULL!"); return 0; }

	uiRet = SeStr2Hash(pcText, (int)seplen);
	lua_pushinteger(L, uiRet);
	return 1;
}

static int CoreToolSchedSetaffinity(lua_State *L)
{
	bool bRet;
	int iCpu;

	iCpu = (int)luaL_checkinteger(L, 1);

	bRet = SeSchedSetaffinity(iCpu);
	lua_pushboolean(L, bRet);
	return 1;
}

static int CoreToolGetCpuNum(lua_State *L)
{
	long lCUPNum;

	lCUPNum = SeGetCpuNum();
	lua_pushinteger(L, lCUPNum);
	return 1;
}

static int CoreToolBase64Encode(lua_State *L)
{
	unsigned int uiLen;
	size_t seplen;
	const char *pcText;
	char *buffer = NULL;

	seplen = 0;
	pcText = luaL_checklstring(L, 1, &seplen);
	if (!pcText) { luaL_error(L, "pcText is NULL!"); return 0; }

	buffer = lua_newuserdata(L, BASE64_ENCODE_OUT_SIZE(seplen));
	uiLen = SeBase64Encode((const unsigned char *)pcText, (unsigned int)seplen, buffer);

	lua_pushlstring(L, (const char*)buffer, uiLen);
	return 1;
}

static int CoreToolBase64Decode(lua_State *L)
{
	unsigned int uiLen;
	size_t seplen;
	const char *pcText;
	char *buffer = NULL;

	seplen = 0;
	pcText = luaL_checklstring(L, 1, &seplen);
	if (!pcText) { luaL_error(L, "pcText is NULL!"); return 0; }

	buffer = lua_newuserdata(L, BASE64_DECODE_OUT_SIZE(seplen));
	uiLen = SeBase64Decode((const char *)pcText, (unsigned int)seplen, (unsigned char*)buffer);

	lua_pushlstring(L, (const char*)buffer, uiLen);
	return 1;
}

static int CoreToolSHA1(lua_State *L)
{
	size_t seplen;
	const char *pcText;
	char buffer[41];

	seplen = 0;
	pcText = luaL_checklstring(L, 1, &seplen);
	if (!pcText) { luaL_error(L, "pcText is NULL!"); return 0; }

	SeSHA1((const char *)pcText, (int)seplen, buffer);

	lua_pushlstring(L, (const char*)buffer, sizeof(buffer));
	return 1;
}

static int CoreToolMacSHA1(lua_State *L)
{
	size_t seplen, keylen;
	const char *pcText, *pcKey;
	char buffer[41];

	seplen = 0;
	pcText = luaL_checklstring(L, 1, &seplen);
	if (!pcText) { luaL_error(L, "pcText is NULL!"); return 0; }

	keylen = 0;
	pcKey = luaL_checklstring(L, 2, &keylen);
	if (!pcKey) { luaL_error(L, "pcKey is NULL!"); return 0; }

	SeMacSHA1((const char *)pcKey, keylen, (const char *)pcText, seplen, buffer);

	lua_pushlstring(L, (const char*)buffer, sizeof(buffer));
	return 1;
}

#if (defined(_WIN32) || defined(WIN32))
__declspec(dllexport) int luaopen_CoreTool(lua_State *L)
#elif defined(__linux)
int luaopen_CoreTool(lua_State *L)
#endif
{
	// must use int64 number
	if(LUA_VERSION_NUM < 503) { luaL_error(L, "Lua ver must > 5.3"); return 0; }
	if(sizeof(lua_Integer) != 8) { luaL_error(L, "must use int64 for lua_Integer"); return 0; }
	luaL_checkversion(L);

	luaL_Reg reg[] = {
		{ "Sleep", CoreToolSleep },
		{ "GetTickCount", CoreToolGetTickCount },
		{ "MD5", CoreToolMD5 },
		{ "Str2Hash", CoreToolStr2Hash },
		{ "SchedSetaffinity", CoreToolSchedSetaffinity },
		{ "GetCpuNum", CoreToolGetCpuNum },
		{ "Base64Encode", CoreToolBase64Encode },
		{ "Base64Decode", CoreToolBase64Decode },
		{ "SHA1", CoreToolSHA1 },
		{ "MacSHA1", CoreToolMacSHA1 },
		{ NULL, NULL },
	};

	luaL_newlib(L, reg);

	return 1;
}
