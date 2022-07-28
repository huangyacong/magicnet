#ifndef __SECOMMON_H__
#define __SECOMMON_H__

#include <map>
#include <list>
#include <vector>
#include <string>
#include <sstream>
#include "SeMD5.h"
#include "SeTime.h"

using namespace std;

string SeShortToA(short sShort);

string SeUnSignedShortToA(unsigned short usShort);

string SeIntToA(int iInt);

string SeUnsignedIntToA(unsigned int uiInt);

string SeLongLongToA(long long llLongLong);

string SeUnsignedLongLongToA(unsigned long long ullLongLong);

string SeTimeToString(time_t kTime);

void SeStrSplit(const string& src, const string& separator, vector<string>& dest);

#endif