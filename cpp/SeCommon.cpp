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
	char acNowStr[256] = { 0 };
	SeTimeFormatTime(kTime, acNowStr, (int)(sizeof(acNowStr) / sizeof(char)));
	return string(acNowStr);
}

void SeStrSplit(const string& src, const string& separator, vector<string>& dest)
{
	string substring;
	int iSrcStrLen = (int)src.length();
	string::size_type start = 0, index = 0;

	if (iSrcStrLen <= 0)
		return;

	if (separator.length() <= 0){
		dest.push_back(src);
		return;
	}

	do {
		index = src.find(separator, start);
		if (index == string::npos)
			break;
		substring = src.substr(start, index - start);
		if (substring.size() > 0)
			dest.push_back(substring);
		start = index + separator.length();
	} while ((int)start < iSrcStrLen);

	if ((int)start >= iSrcStrLen)
		return;

	//the last token
	substring = src.substr(start);
	if (substring.size() > 0)
		dest.push_back(substring);
}

bool isSpace(char cChar)
{
	/*
	0x20 空格
	0x09 水平制表符
	0x0A 换行符
	0x0B 垂直制表符
	0x0C 换页键
	0x0D 回车
	*/
	return cChar == 0x20 || (cChar <= 0x0d && cChar >= 0x09);
}

string SeStringTrim(const string& str)
{
	if (str.empty())
		return string("");
	string::const_iterator begin = str.begin();
	string::const_iterator end = str.end();
	while ((begin < end) && isSpace(end[-1]))
		--end;
	while ((begin < end) && isSpace(*begin))
		begin++;
	return string(begin, end);
}

