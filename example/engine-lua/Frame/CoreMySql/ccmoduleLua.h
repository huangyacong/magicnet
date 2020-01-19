#ifndef __CCMODULE_COREMYSQL_LUA_H__
#define __CCMODULE_COREMYSQL_LUA_H__

#ifdef	__cplusplus
extern "C" {
#endif

#include <lua.h>
#include <lauxlib.h>
#include <time.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <lualib.h>
#include "SeTime.h"

static int CoreMySqlLibraryInit(lua_State *L);

static int CoreMySqlLibraryEnd(lua_State *L);

static int CoreMySqlInit(lua_State *L);

static int CoreMySqlTryConnect(lua_State *L);

static int CoreMySqlExecuteSql(lua_State *L);

static int CoreMySqlStoreResult(lua_State *L);

static int CoreMySqlGetLastInsertId(lua_State *L);

static int CoreMySqlGetAffectedRows(lua_State *L);

static int CoreMySqlEscape(lua_State *L);

static int CoreMySqlSetAutoCommit(lua_State *L);

static int CoreMySqlCommit(lua_State *L);

static int CoreMySqlRollback(lua_State *L);

static int CoreMySqlGetErrorCode(lua_State *L);

static int CoreMySqlGetErrorStr(lua_State *L);

static int CoreMySqlIsConnect(lua_State *L);

static int CoreMySqlDisConnect(lua_State *L);

#ifdef	__cplusplus
}
#endif

#endif
