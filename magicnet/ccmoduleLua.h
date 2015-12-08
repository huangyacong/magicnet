#include <lua.h>
#include <lauxlib.h>
#include <time.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <lualib.h>
#include "SeMagicNet.h"

static struct SEMAGICNETS kMagicNetGate;
static struct SEMAGICNETC kMagicNetSvr;

static int MagicNetGateInit(lua_State *L);

static int MagicNetGateFin(lua_State *L);

static int MagicNetGateProcess(lua_State *L);

static int MagicNetSvrInit(lua_State *L);

static int MagicNetSvrFin(lua_State *L);

static int MagicNetSvrReg(lua_State *L);

static int MagicNetSvrSendClient(lua_State *L);

static int MagicNetSvrBindClient(lua_State *L);

static int MagicNetSvrCloseClient(lua_State *L);

static int MagicNetSvrSendSvr(lua_State *L);

static int MagicNetSvrRead(lua_State *L);
