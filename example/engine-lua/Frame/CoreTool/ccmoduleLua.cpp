#include "ccmoduleLua.h"
#include "SeThread.h"
#include "SeTime.h"
#include "SeBase64.h"
#include "SeSha1.h"
#include "SeTool.h"
#include "SeMD5.h"
#include "SeSort.h"
#include "SeTimer.h"

static int g_iShiftBit;
static unsigned long long g_ullSysSessionTickCount;
static unsigned long long g_ullMaxSysySessionCount;
static unsigned long long g_ullSysySessionCount;
static std::map<int, SeSort> g_kSort;
static long long g_ullTimeOffSet;
static SeTimer g_kSeTimer;

static void CoreToolInit()
{
	g_iShiftBit = 21;
	g_ullSysSessionTickCount = SeTimeGetTickCount();
	g_ullMaxSysySessionCount = ((unsigned long long)pow(2, g_iShiftBit) - 1);
	g_ullSysySessionCount = 0;
	g_ullTimeOffSet = 0;
}

static SeSort& CoreToolGetChartByType(int iChartType)
{
	std::map<int, SeSort>::iterator itr = g_kSort.find(iChartType);
	if (itr == g_kSort.end())
		g_kSort[iChartType] = SeSort();
	return g_kSort[iChartType];
}

extern "C" int CoreNetSysSessionId(lua_State *L)
{
	unsigned long long timer, count, ret;

	g_ullSysySessionCount++;
	if (g_ullSysySessionCount >= g_ullMaxSysySessionCount)
	{
		g_ullSysySessionCount = 0;
		g_ullSysSessionTickCount++;
	}

	timer = SeTimeGetTickCount();
	if (timer > g_ullSysSessionTickCount)
	{
		g_ullSysySessionCount = 0;
		g_ullSysSessionTickCount = timer;
	}

	timer = g_ullSysSessionTickCount;
	count = g_ullSysySessionCount;
	ret = ((timer << g_iShiftBit) | count) & 0x7FFFFFFFFFFFFFFF;

	lua_pushinteger(L, ret);
	return 1;
}

extern "C" int CoreToolSleep(lua_State *L)
{
	int iCounter;

	iCounter = (int)luaL_checkinteger(L, 1);
	SeTimeSleep(iCounter);

	lua_pushnil(L);
	return 1;
}

extern "C" int CoreToolGetTickCount(lua_State *L)
{
	unsigned long long ullTime;

	ullTime = SeTimeGetTickCount() + g_ullTimeOffSet * 1000;
	lua_pushinteger(L, ullTime);
	return 1;
}

extern "C" int CoreToolMD5(lua_State *L)
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

extern "C" int CoreToolStr2Hash(lua_State *L)
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

extern "C" int CoreToolSchedSetaffinity(lua_State *L)
{
	bool bRet;
	int iCpu;

	iCpu = (int)luaL_checkinteger(L, 1);

	bRet = SeSchedSetaffinity(iCpu);
	lua_pushboolean(L, bRet);
	return 1;
}

extern "C" int CoreToolGetCpuNum(lua_State *L)
{
	long lCUPNum;

	lCUPNum = SeGetCpuNum();
	lua_pushinteger(L, lCUPNum);
	return 1;
}

extern "C" int CoreToolBase64Encode(lua_State *L)
{
	unsigned int uiLen;
	size_t seplen;
	const char *pcText;
	char *buffer = NULL;

	seplen = 0;
	pcText = luaL_checklstring(L, 1, &seplen);
	if (!pcText) { luaL_error(L, "pcText is NULL!"); return 0; }

	buffer = (char*)lua_newuserdata(L, BASE64_ENCODE_OUT_SIZE(seplen));
	uiLen = SeBase64Encode((const unsigned char *)pcText, (unsigned int)seplen, buffer);

	lua_pushlstring(L, (const char*)buffer, uiLen);
	return 1;
}

extern "C" int CoreToolBase64Decode(lua_State *L)
{
	unsigned int uiLen;
	size_t seplen;
	const char *pcText;
	char *buffer = NULL;

	seplen = 0;
	pcText = luaL_checklstring(L, 1, &seplen);
	if (!pcText) { luaL_error(L, "pcText is NULL!"); return 0; }

	buffer = (char*)lua_newuserdata(L, BASE64_DECODE_OUT_SIZE(seplen));
	uiLen = SeBase64Decode((const char *)pcText, (unsigned int)seplen, (unsigned char*)buffer);

	lua_pushlstring(L, (const char*)buffer, uiLen);
	return 1;
}

extern "C" int CoreToolSHA1(lua_State *L)
{
	size_t seplen;
	const char *pcText;
	char buffer[41];
	bool bHex;

	seplen = 0;
	pcText = luaL_checklstring(L, 1, &seplen);
	if (!pcText) { luaL_error(L, "pcText is NULL!"); return 0; }

	bHex = lua_toboolean(L, 2) == 1 ? true : false;

	SeSHA1((const char *)pcText, (int)seplen, buffer, bHex);

	lua_pushlstring(L, (const char*)buffer, sizeof(buffer));
	return 1;
}

extern "C" int CoreToolMacSHA1(lua_State *L)
{
	size_t seplen, keylen;
	const char *pcText, *pcKey;
	char buffer[41];
	bool bHex;

	seplen = 0;
	pcText = luaL_checklstring(L, 1, &seplen);
	if (!pcText) { luaL_error(L, "pcText is NULL!"); return 0; }

	keylen = 0;
	pcKey = luaL_checklstring(L, 2, &keylen);
	if (!pcKey) { luaL_error(L, "pcKey is NULL!"); return 0; }

	bHex = lua_toboolean(L, 3) == 1 ? true : false;

	SeMacSHA1((const char *)pcKey, keylen, (const char *)pcText, seplen, buffer, bHex);

	lua_pushlstring(L, (const char*)buffer, sizeof(buffer));
	return 1;
}

extern "C" int CoreToolCreateUniqueID(lua_State* L)
{
	int iServerID;
	unsigned int uiCount;
	unsigned long long ullResult;

	iServerID = (int)luaL_checkinteger(L, 1);
	uiCount = (unsigned int)luaL_checkinteger(L, 2);

	ullResult = CreateUniqueID(iServerID, uiCount);

	lua_pushinteger(L, ullResult);
	return 1;
}

extern "C" int CoreToolGetServerIDByUniqueID(lua_State* L)
{
	int iServerID;
	unsigned long long ullUniqueID;

	ullUniqueID = luaL_checkinteger(L, 1);
	iServerID = GetServerIDByUniqueID(ullUniqueID);

	lua_pushinteger(L, iServerID);
	return 1;
}

extern "C" int CoreToolCreateMailID(lua_State* L)
{
	int iID;
	unsigned int uiCount;
	unsigned long long ullResult;

	iID = (int)luaL_checkinteger(L, 1);
	uiCount = (unsigned int)luaL_checkinteger(L, 2);

	ullResult = CreateMailID((unsigned short)iID, uiCount);

	lua_pushinteger(L, ullResult);
	return 1;
}

extern "C" int CoreToolChartAddOrModify(lua_State * L)
{
	int iChartType;
	unsigned long long ullID;
	long long llScore;

	iChartType = (int)luaL_checkinteger(L, 1);
	ullID = luaL_checkinteger(L, 2);
	llScore = luaL_checkinteger(L, 3);

	SeSort& rkChart = CoreToolGetChartByType(iChartType);
	rkChart.ModifySortItem(ullID, llScore);

	lua_pushboolean(L, true);
	return 1;
}

extern "C" int CoreToolChartGetScoreNoByID(lua_State * L)
{
	int iChartType;
	int iScoreNo;
	unsigned long long ullID;

	iChartType = (int)luaL_checkinteger(L, 1);
	ullID = luaL_checkinteger(L, 2);

	SeSort& rkChart = CoreToolGetChartByType(iChartType);
	iScoreNo = rkChart.GetScoreNoByID(ullID);

	lua_pushinteger(L, iScoreNo);
	return 1;
}

extern "C" int CoreToolChartGetRank(lua_State * L)
{
	int iChartType;
	int iScoreNo;
	unsigned long long ullID;

	iChartType = (int)luaL_checkinteger(L, 1);
	iScoreNo = (int)luaL_checkinteger(L, 2);

	SeSort& rkChart = CoreToolGetChartByType(iChartType);
	ullID = rkChart.GetRank(iScoreNo);

	lua_pushinteger(L, ullID);
	return 1;
}

extern "C" int CoreToolChartSort(lua_State * L)
{
	int iChartType;

	iChartType = (int)luaL_checkinteger(L, 1);

	SeSort& rkChart = CoreToolGetChartByType(iChartType);
	rkChart.Sort();

	lua_pushboolean(L, true);
	return 1;
}

extern "C" int CoreToolChartClear(lua_State * L)
{
	int iChartType;

	iChartType = (int)luaL_checkinteger(L, 1);

	SeSort& rkChart = CoreToolGetChartByType(iChartType);
	rkChart.Clear();

	lua_pushboolean(L, true);
	return 1;
}

extern "C" int CoreToolChartGetAllRank(lua_State * L)
{
	int iChartType;

	iChartType = (int)luaL_checkinteger(L, 1);

	lua_newtable(L);

	int iIndex = 0;
	SeSort& rkChart = CoreToolGetChartByType(iChartType);
	rkChart.Sort();
	const std::vector<unsigned long long>& rkResult = rkChart.GetAllRank();
	for (std::vector<unsigned long long>::const_iterator itr = rkResult.begin(); itr != rkResult.end(); itr++)
	{	
		iIndex++;
		lua_pushinteger(L, *itr);
		lua_rawseti(L, -2, iIndex);
	}
	
	return 1;
}

extern "C" int CoreToolChartGetRangRank(lua_State * L)
{
	int iChartType;
	int iBeginScoreID;
	int iEndScoreID;

	iChartType = (int)luaL_checkinteger(L, 1);
	iBeginScoreID = (int)luaL_checkinteger(L, 2);
	iEndScoreID = (int)luaL_checkinteger(L, 3);

	iBeginScoreID--;
	iEndScoreID--;

	SeSort& rkChart = CoreToolGetChartByType(iChartType);
	const std::vector<unsigned long long>& rkResult = rkChart.GetAllRank();

	if (iBeginScoreID < 0 || iEndScoreID < 0 || iBeginScoreID > iEndScoreID)
	{
		luaL_error(L, "ChartGetRangRank argvs is NULL! iChartType=%d iBeginScoreID=%d iEndScoreID=%d size=%d", iChartType, iBeginScoreID, iEndScoreID, (int)rkResult.size());
		return 0;
	}

	lua_newtable(L);

	if (rkResult.empty())
	{
		return 1;
	}

	int iIndex = 0;
	for (int i = iBeginScoreID; i <= iEndScoreID && i < (int)rkResult.size(); i++)
	{
		iIndex++;
		lua_pushinteger(L, rkResult.at(i));
		lua_rawseti(L, -2, iIndex);
	}

	return 1;
}

extern "C" int CoreNetAddTimer(lua_State * L)
{
	long long llTimerId;
	unsigned long long ullDealyTimeMillSec;

	ullDealyTimeMillSec = luaL_checkinteger(L, 1);

	llTimerId = g_kSeTimer.SetTimer(ullDealyTimeMillSec, g_ullTimeOffSet * 1000);

	lua_pushinteger(L, llTimerId);
	return 1;
}

extern "C" int CoreNetDelTimer(lua_State * L)
{
	long long llTimerId;

	llTimerId = (long long)luaL_checkinteger(L, 1);

	g_kSeTimer.DelTimer(llTimerId);

	lua_pushnil(L);
	return 1;
}

extern "C" int CoreNetGetTimeOutId(lua_State * L)
{
	long long llTimerId;

	llTimerId = g_kSeTimer.GetTimeOutId(SeTimeGetTickCount() + g_ullTimeOffSet * 1000);

	lua_pushinteger(L, llTimerId);
	return 1;
}

extern "C" int CoreToolResetTimeOffSet(lua_State * L)
{
	g_ullTimeOffSet = 0;
	lua_pushboolean(L, true);
	return 1;
}

extern "C" int CoreToolModifyTimeOffSet(lua_State * L)
{
	int iTimeOffSet;

	iTimeOffSet = (int)luaL_checkinteger(L, 1);

	g_ullTimeOffSet += iTimeOffSet;

	lua_pushboolean(L, true);
	return 1;
}

extern "C" int CoreToolGetTimeOffSet(lua_State * L)
{
	lua_pushinteger(L, g_ullTimeOffSet);
	return 1;
}

#if (defined(_WIN32) || defined(WIN32))
extern "C" __declspec(dllexport) int luaopen_CoreTool(lua_State *L)
#elif defined(__linux)
extern "C" int luaopen_CoreTool(lua_State *L)
#endif
{
	// must use int64 number
	if(LUA_VERSION_NUM < 503) { luaL_error(L, "Lua ver must > 5.3"); return 0; }
	if(sizeof(lua_Integer) != 8) { luaL_error(L, "must use int64 for lua_Integer"); return 0; }
	luaL_checkversion(L);

	CoreToolInit();

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
		{ "SysSessionId", CoreNetSysSessionId },
		{ "CreateUniqueID", CoreToolCreateUniqueID },
		{ "GetServerIDByUniqueID", CoreToolGetServerIDByUniqueID },
		{ "CreateMailID", CoreToolCreateMailID },
		{ "ChartClear", CoreToolChartClear },
		{ "ChartSort", CoreToolChartSort },
		{ "ChartAddOrModify", CoreToolChartAddOrModify },
		{ "ChartGetScoreNoByID", CoreToolChartGetScoreNoByID },
		{ "ChartGetRank", CoreToolChartGetRank },
		{ "ChartGetAllRank", CoreToolChartGetAllRank },
		{ "ChartGetRangRank", CoreToolChartGetRangRank },
		{ "AddTimer", CoreNetAddTimer },
		{ "DelTimer", CoreNetDelTimer },
		{ "GetTimeOutId", CoreNetGetTimeOutId },
		{ "ResetTimeOffSet", CoreToolResetTimeOffSet },
		{ "ModifyTimeOffSet", CoreToolModifyTimeOffSet },
		{ "GetTimeOffSet", CoreToolGetTimeOffSet },
		{ NULL, NULL },
	};

	luaL_newlib(L, reg);

	return 1;
}
