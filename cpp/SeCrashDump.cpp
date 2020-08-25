#include "SeCrashDump.h"
#include "SeNetBase.h"
#include "SeTool.h"

#if (defined(_WIN32) || defined(WIN32))

static char acNameCrashDump[1024] = { 0 };

void SeCreateDumpFile(LPCSTR lpstrDumpFilePathName, EXCEPTION_POINTERS *pkException)
{ 
	HANDLE hDumpFile = CreateFile(lpstrDumpFilePathName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hDumpFile == SE_INVALID_HANDLE)
	{
		return;
	}

	MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
	dumpInfo.ExceptionPointers = pkException;
	dumpInfo.ThreadId = GetCurrentThreadId();
	dumpInfo.ClientPointers = TRUE;

	MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpNormal, &dumpInfo, NULL, NULL);

	CloseHandle(hDumpFile);
}

LONG SeApplicationCrashHandler(EXCEPTION_POINTERS *pkException)
{
	SeCreateDumpFile(acNameCrashDump, pkException);
	return EXCEPTION_EXECUTE_HANDLER;
}

#endif

void SeCrashDump(string kLogName)
{
#if (defined(_WIN32) || defined(WIN32))
	SeStrNcpy(acNameCrashDump, (int)sizeof(acNameCrashDump), kLogName.c_str());
	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)SeApplicationCrashHandler);
#endif
}
