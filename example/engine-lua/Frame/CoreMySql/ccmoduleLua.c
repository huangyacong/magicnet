#include "ccmoduleLua.h"
#include "SeMysql.h"

static struct SEMYSQL g_SQL;
static char g_acCharacterSet[128];
static char g_acEscape[1024 * 1024 * 4];

static int CoreMySqlLibraryInit(lua_State *L)
{
	SeMysqlLibraryInit();
	lua_pushnil(L);
	return 1;
}

static int CoreMySqlLibraryEnd(lua_State *L)
{
	SeMysqlLibraryEnd();
	lua_pushnil(L);
	return 1;
}

static int CoreMySqlInit(lua_State *L)
{
	size_t seplen;
	const char* pcHost;
	unsigned int uiPort;
	const char* pcDBName;
	const char* pcUser;
	const char* pcPasswd;
	const char* pcCharacterSet;

	seplen = 0;
	pcHost = luaL_checklstring(L, 1, &seplen);
	if (!pcHost) { luaL_error(L, "pcHost is NULL!"); return 0; }
	uiPort = (unsigned int)luaL_checkinteger(L, 2);

	seplen = 0;
	pcDBName = luaL_checklstring(L, 3, &seplen);
	if (!pcDBName) { luaL_error(L, "pcDBName is NULL!"); return 0; }

	seplen = 0;
	pcUser = luaL_checklstring(L, 4, &seplen);
	if (!pcUser) { luaL_error(L, "pcUser is NULL!"); return 0; }

	seplen = 0;
	pcPasswd = luaL_checklstring(L, 5, &seplen);
	if (!pcPasswd) { luaL_error(L, "pcPasswd is NULL!"); return 0; }

	seplen = 0;
	pcCharacterSet = luaL_checklstring(L, 6, &seplen);
	if (!pcCharacterSet) { luaL_error(L, "pcCharacterSet is NULL!"); return 0; }

	SeStrNcpy(g_acCharacterSet, sizeof(g_acCharacterSet), pcCharacterSet);
	SeMysqlInit(&g_SQL, pcHost, uiPort, pcDBName, pcUser, pcPasswd);
	lua_pushnil(L);
	return 1;
}

static int CoreMySqlTryConnect(lua_State *L)
{
	bool bResult;

	bResult = SeMysqlTryConnect(&g_SQL);
	if (bResult)
	{
		SeMysqlSetAutoCommit(&g_SQL, true);
		SeSetCharacterSet(&g_SQL, (const char *)g_acCharacterSet);
	}

	lua_pushboolean(L, bResult);
	return 1;
}


static int CoreMySqlExecuteSql(lua_State *L)
{
	bool bResult;
	size_t seplen;
	const char* pcQuerySql;
	unsigned long ulLen;

	seplen = 0;
	pcQuerySql = luaL_checklstring(L, 1, &seplen);
	if (!pcQuerySql) { luaL_error(L, "pcQuerySql is NULL!"); return 0; }
	ulLen = (unsigned int)seplen;

	bResult = SeMysqlExecuteSql(&g_SQL, pcQuerySql, ulLen);
	lua_pushboolean(L, bResult);
	return 1;
}

static int CoreMySqlStoreResult(lua_State *L)
{
	bool bResult;
	unsigned int i;
	unsigned int iBegin;
	unsigned int uiCount;
	struct SEMYSQLRESULT kMySqlResult;

	SeMysqlResultInit(&kMySqlResult);
	bResult = SeMysqlStoreResult(&g_SQL, &kMySqlResult);
	if (!bResult) { SeMysqlResultFin(&kMySqlResult); lua_pushnil(L); return 1; }
	while (SeMysqlNextResult(&g_SQL)) {}
	
	iBegin = 1;
	lua_newtable(L);

	while (SeMysqlResultNextRecord(&kMySqlResult))
	{
		lua_newtable(L);

		uiCount = SeMysqlResultGetFieldCount(&kMySqlResult);
		for (i = 0; i < uiCount; i++)
		{
			lua_pushstring(L, SeMysqlResultGetFieldName(&kMySqlResult, i));
			if (!SeMysqlResultGetFieldValue(&kMySqlResult, i)) { lua_pushnil(L); }
			else { lua_pushlstring(L, SeMysqlResultGetFieldValue(&kMySqlResult, i), SeMysqlResultGetFieldLen(&kMySqlResult, i)); }
			lua_settable(L, -3);
		}

		lua_rawseti(L, -2, iBegin);
		iBegin++;
	}

	SeMysqlResultFin(&kMySqlResult);
	return 1;
}

static int CoreMySqlGetLastInsertId(lua_State *L)
{
	unsigned long long ullLastID;

	ullLastID = SeMysqlInsertId(&g_SQL);
	lua_pushinteger(L, ullLastID);
	return 1;
}

static int CoreMySqlGetAffectedRows(lua_State *L)
{
	unsigned long long ullRows;

	ullRows = SeMysqlAffectedRows(&g_SQL);
	lua_pushinteger(L, ullRows);
	return 1;
}

static int CoreMySqlEscape(lua_State *L)
{
	size_t seplen;
	const char* pcSrc;
	unsigned long ulSrcLen;

	seplen = 0;
	pcSrc = luaL_checklstring(L, 1, &seplen);
	if (!pcSrc) { luaL_error(L, "pcSrc is NULL!"); return 0; }
	ulSrcLen = (unsigned long)seplen;

	if (ulSrcLen < 0 || (ulSrcLen * 2 + 1) >= (int)sizeof(g_acEscape))
	{
		luaL_error(L, "ulSrcLen error!");
		return 0;
	}

	unsigned long ulRet = SeMysqlEscape(&g_SQL, g_acEscape, pcSrc, ulSrcLen);
	if (ulRet < ulSrcLen)
	{
		luaL_error(L, "SeMysqlEscape error!");
		return 0;
	}

	lua_pushlstring(L, (const char*)g_acEscape, ulRet);
	return 1;
}

static int CoreMySqlSetAutoCommit(lua_State *L)
{
	bool bResult;
	bool bAutoCommit;

	bAutoCommit = lua_toboolean(L, 1);

	bResult = SeMysqlSetAutoCommit(&g_SQL, bAutoCommit);
	lua_pushboolean(L, bResult);
	return 1;
}

static int CoreMySqlCommit(lua_State *L)
{
	bool bResult;

	bResult = SeMysqlCommit(&g_SQL);
	lua_pushboolean(L, bResult);
	return 1;
}

static int CoreMySqlRollback(lua_State *L)
{
	bool bResult;

	bResult = SeMysqlRollback(&g_SQL);
	lua_pushboolean(L, bResult);
	return 1;
}

static int CoreMySqlGetErrorCode(lua_State *L)
{
	unsigned int uiCodeID;

	uiCodeID = SeMysqlGetErrorCode(&g_SQL);
	lua_pushinteger(L, uiCodeID);
	return 1;
}

static int CoreMySqlGetErrorStr(lua_State *L)
{
	const char* pcCodeStr;

	pcCodeStr = SeMysqlGetErrorStr(&g_SQL);
	lua_pushstring(L, pcCodeStr);
	return 1;
}

static int CoreMySqlIsConnect(lua_State *L)
{
	bool bResult;

	bResult = SeMysqlIsConnect(&g_SQL);
	lua_pushboolean(L, bResult);
	return 1;
}

static int CoreMySqlDisConnect(lua_State *L)
{
	SeMysqlDisConnect(&g_SQL);
	lua_pushnil(L);
	return 1;
}

#if (defined(_WIN32) || defined(WIN32))
__declspec(dllexport) int luaopen_CoreMySql(lua_State *L)
#elif defined(__linux)
int luaopen_CoreMySql(lua_State *L)
#endif
{
	// must use int64 number
	if(LUA_VERSION_NUM < 503) { luaL_error(L, "Lua ver must > 5.3"); return 0; }
	if(sizeof(lua_Integer) != 8) { luaL_error(L, "must use int64 for lua_Integer"); return 0; }
	luaL_checkversion(L);

	luaL_Reg reg[] = {
		{ "LibraryInit", CoreMySqlLibraryInit },
		{ "LibraryEnd", CoreMySqlLibraryEnd },
		{ "Init", CoreMySqlInit },
		{ "TryConnect", CoreMySqlTryConnect },
		{ "ExecuteSql", CoreMySqlExecuteSql },
		{ "StoreResult", CoreMySqlStoreResult },
		{ "GetLastInsertId", CoreMySqlGetLastInsertId },
		{ "GetAffectedRows", CoreMySqlGetAffectedRows },
		{ "Escape", CoreMySqlEscape },
		{ "SetAutoCommit", CoreMySqlSetAutoCommit },
		{ "Commit", CoreMySqlCommit },
		{ "Rollback", CoreMySqlRollback },
		{ "GetErrorCode", CoreMySqlGetErrorCode },
		{ "GetErrorStr", CoreMySqlGetErrorStr },
		{ "IsConnect", CoreMySqlIsConnect },
		{ "DisConnect", CoreMySqlDisConnect },
		{ NULL, NULL },
	};

	luaL_newlib(L, reg);

	return 1;
}
