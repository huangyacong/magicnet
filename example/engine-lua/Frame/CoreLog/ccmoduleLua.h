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
#include "SeTime.h"
#include "SeLog.h"

static int CoreLogInit(lua_State *L);

static int CoreLogFin(lua_State *L);

static int CoreLogAddAllLogLV(lua_State *L);

static int CoreLogAddLogLV(lua_State *L);

static int CoreLogClearLogLV(lua_State *L);

static int CoreLogWrite(lua_State *L);

static int CoreLogFlush(lua_State *L);

static int CoreLogPrint(lua_State *L);

#ifdef	__cplusplus
}
#endif

#endif
