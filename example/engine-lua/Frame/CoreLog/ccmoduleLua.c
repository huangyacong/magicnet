#include "ccmoduleLua.h"

static struct SELOG g_Log;

static int CoreLogInit(lua_State *L)
{
	size_t seplen = 0;
	const char *pcLogName = NULL;

	pcLogName = luaL_checklstring(L, 1, &seplen);
	if (!pcLogName) { luaL_error(L, "pcLogName is NULL!"); return 0; }

	SeInitLog(&g_Log, (char*)pcLogName);
	lua_pushnil(L); 
	return 1;
}

static int CoreLogFin(lua_State *L)
{
	SeFinLog(&g_Log);
	lua_pushnil(L);
	return 1;
}

static int CoreLogAddAllLogLV(lua_State *L)
{
	int iLogLv = 0;

	iLogLv = LT_SPLIT | LT_PRINT | LT_NOHEAD | LT_ERROR | LT_WARNING | LT_INFO | LT_DEBUG | LT_CRITICAL | LT_SOCKET | LT_RESERROR;
	SeAddLogLV(&g_Log, iLogLv);

	lua_pushnil(L);
	return 1;
}

static int CoreLogAddLogLV(lua_State *L)
{
	int iLogLv;

	iLogLv = (int)luaL_checkinteger(L, 1);

	SeAddLogLV(&g_Log, iLogLv);
	lua_pushnil(L);
	return 1;
}

static int CoreLogClearLogLV(lua_State *L)
{
	int iLogLv;

	iLogLv = (int)luaL_checkinteger(L, 1);

	SeClearLogLV(&g_Log, iLogLv);
	lua_pushnil(L);
	return 1;
}

static int CoreLogWrite(lua_State *L)
{
	int iLogLv;
	size_t seplen = 0;
	const char *pcText;
	bool bFlushToDisk;

	iLogLv = (int)luaL_checkinteger(L, 1);
	
	pcText = luaL_checklstring(L, 2, &seplen);
	if (!pcText) { luaL_error(L, "pcText is NULL!"); return 0; }

	bFlushToDisk = lua_toboolean(L, 3);

	SeLogWrite(&g_Log, iLogLv, bFlushToDisk, "%s", pcText);
	lua_pushnil(L);
	return 1;
}

static int CoreLogFlush(lua_State *L)
{
	SeLogFlushToDisk(&g_Log);
	lua_pushnil(L);
	return 1;
}

static int CoreLogPrint(lua_State *L)
{
	int iLogLv;
	size_t seplen = 0;
	const char *pcText;

	iLogLv = (int)luaL_checkinteger(L, 1);

	pcText = luaL_checklstring(L, 2, &seplen);
	if (!pcText) { luaL_error(L, "pcText is NULL!"); return 0; }

	SePrintf(iLogLv, "", pcText);

	lua_pushnil(L);
	return 1;
}

#if (defined(_WIN32) || defined(WIN32))
__declspec(dllexport) int luaopen_CoreLog(lua_State *L)
#elif defined(__linux)
int luaopen_CoreLog(lua_State *L)
#endif
{
	// must use int64 number
	if(LUA_VERSION_NUM < 503) { luaL_error(L, "Lua ver must > 5.3"); return 0; }
	if(sizeof(lua_Integer) != 8) { luaL_error(L, "must use int64 for lua_Integer"); return 0; }
	luaL_checkversion(L);

	luaL_Reg reg[] = {
		{ "Init", CoreLogInit },
		{ "Fin", CoreLogFin },
		{ "AddAllLogLV", CoreLogAddAllLogLV },
		{ "AddLogLV", CoreLogAddLogLV },
		{ "ClearLogLV", CoreLogClearLogLV },
		{ "Write", CoreLogWrite },
		{ "Flush", CoreLogFlush }, 
		{ "Print", CoreLogPrint },
		{ NULL, NULL },
	};

	luaL_newlib(L, reg);

	lua_pushinteger(L, LT_SPLIT);
	lua_setfield(L, -2, "LOG_LV_SPLIT");

	lua_pushinteger(L, LT_PRINT);
	lua_setfield(L, -2, "LOG_LV_PRINT");

	lua_pushinteger(L, LT_NOHEAD);
	lua_setfield(L, -2, "LOG_LV_NOHEAD");

	lua_pushinteger(L, LT_ERROR);
	lua_setfield(L, -2, "LOG_LV_ERROR");

	lua_pushinteger(L, LT_WARNING);
	lua_setfield(L, -2, "LOG_LV_WARNING");

	lua_pushinteger(L, LT_INFO);
	lua_setfield(L, -2, "LOG_LV_INFO");

	lua_pushinteger(L, LT_DEBUG);
	lua_setfield(L, -2, "LOG_LV_DEBUG");

	lua_pushinteger(L, LT_CRITICAL);
	lua_setfield(L, -2, "LOG_LV_CRITICAL");

	lua_pushinteger(L, LT_SOCKET);
	lua_setfield(L, -2, "LOG_LV_SOCKET");

	lua_pushinteger(L, LT_RESERROR);
	lua_setfield(L, -2, "LOG_LV_RESERROR");

	return 1;
}
