#include "SeShareMemory.h"

HANDLE SeCreateShareMemory(const char *pcName, unsigned long long ullSize)
{
	char acName[128];
#if (defined(WIN32) || defined(_WIN32))
	sprintf(acName, "Global\\%s", pcName);
	return CreateFileMapping(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, ullSize>>32, ullSize<<32>>32, acName);
#elif defined(__linux)
#endif
}

HANDLE SeOpenShareMemory(const char *pcName)
{
	char acName[128];
#if (defined(WIN32) || defined(_WIN32))
	sprintf(acName, "Global\\%s", pcName);
	return OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, acName);
#elif defined(__linux)
#endif
}

void SeCloseShareMemory(HANDLE kHandle)
{
#if (defined(WIN32) || defined(_WIN32))
	CloseHandle(kHandle);
#elif defined(__linux)
#endif
}

void *SeViewShareMemory(HANDLE kHandle)
{
#if (defined(WIN32) || defined(_WIN32))
	return (void*)MapViewOfFile(kHandle, FILE_MAP_ALL_ACCESS, 0, 0, 0);
#elif defined(__linux)
#endif
}

bool SeUnViewShareMemory(void *pvBuf)
{
#if (defined(WIN32) || defined(_WIN32))
	return UnmapViewOfFile(pvBuf);
#elif defined(__linux)
#endif
}