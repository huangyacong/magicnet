#ifndef __CCMODULE_CORETOOL_LUA_H__
#define __CCMODULE_CORETOOL_LUA_H__

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
#include <math.h>

static int CoreNetSysSessionId(lua_State *L);

static int CoreToolSleep(lua_State *L);

static int CoreToolGetTickCount(lua_State *L);

static int CoreToolMD5(lua_State *L);

static int CoreToolStr2Hash(lua_State *L);

static int CoreToolSchedSetaffinity(lua_State *L);

static int CoreToolGetCpuNum(lua_State *L);

static int CoreToolBase64Encode(lua_State *L);

static int CoreToolBase64Decode(lua_State *L);

static int CoreToolSHA1(lua_State *L);

static int CoreToolMacSHA1(lua_State *L);

static int CoreToolCreateUniqueID(lua_State* L);

static int CoreToolGetServerIDByUniqueID(lua_State* L);

#ifdef	__cplusplus
}
#endif

#endif
