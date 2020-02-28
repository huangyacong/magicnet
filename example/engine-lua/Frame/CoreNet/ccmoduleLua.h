#ifndef __CCMODULE_CORENET_LUA_H__
#define __CCMODULE_CORENET_LUA_H__

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
#include "SeNetCore.h"

#ifdef	__cplusplus
}
#endif

//Í³¼Æ
struct MsgIDStat
{
	int iSendNum;
	unsigned long long ullSendByteNum;
	int iRecvNum;
	unsigned long long ullRecvByteNum;
	int iPrintNum;
	unsigned long long ullPrintByteNum;

	unsigned long long ullStatTime;
	unsigned long long ullDelayStatTime;
};

#endif
