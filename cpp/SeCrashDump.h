#ifndef __SE_CRASH_DUMP_H__
#define __SE_CRASH_DUMP_H__

#include <string>
#include "SeTime.h"

using namespace std;

// DbgHelp.dll and Dbgcore.dll
#if (defined(_WIN32) || defined(WIN32))

#include <windows.h>
#include <DbgHelp.h>

#pragma comment(lib, "Dbghelp.lib")

#endif

void SeCrashDump(string kLogName);

#endif