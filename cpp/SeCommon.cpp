#include "SeCommon.h"
#include "SeTool.h"
#include "SeTime.h"
#include "SeMD5.h"

string SeShortToA(short sShort)
{
	char acBuf[256] = { 0 };
	SeSnprintf(acBuf, int(sizeof(acBuf)), "%d", sShort);
	return string(acBuf);
}

string SeUnSignedShortToA(unsigned short usShort)
{
	char acBuf[256] = { 0 };
	SeSnprintf(acBuf, int(sizeof(acBuf)), "%u", usShort);
	return string(acBuf);
}

string SeIntToA(int iInt)
{
	char acBuf[256] = { 0 };
	SeSnprintf(acBuf, int(sizeof(acBuf)), "%d", iInt);
	return string(acBuf);
}

string SeUnsignedIntToA(unsigned int uiInt)
{
	char acBuf[256] = { 0 };
	SeSnprintf(acBuf, int(sizeof(acBuf)), "%u", uiInt);
	return string(acBuf);
}

string SeLongLongToA(long long llLongLong)
{
	char acBuf[256] = { 0 };
	SeSnprintf(acBuf, int(sizeof(acBuf)), "%lld", llLongLong);
	return string(acBuf);
}

string SeUnsignedLongLongToA(unsigned long long ullLongLong)
{
	char acBuf[256] = { 0 };
	SeSnprintf(acBuf, int(sizeof(acBuf)), "%llu", ullLongLong);
	return string(acBuf);
}

string SeTimeToString(time_t kTime)
{
	char acNowStr[20] = { 0 };
	SeTimeFormatTime(kTime, acNowStr, (int)(sizeof(acNowStr) / sizeof(char)));
	return string(acNowStr);
}

