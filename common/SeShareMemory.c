#include "SeShareMemory.h"

HANDLE SeCreateShareMemory(const char *pcName, unsigned long long ullSize)
{
	char acName[128];

#if (defined(WIN32) || defined(_WIN32))
	HANDLE kHandle;
	
	sprintf(acName, "Global\\%s", pcName);
	kHandle = CreateFileMapping(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, ullSize>>32, ullSize<<32>>32, acName);
	if(kHandle != SE_INVALID_HANDLE)
	{
		if(SeGetShareMemoryErrorID() == ERROR_ALREADY_EXISTS)
		{
			return SE_INVALID_HANDLE;
		}
	}

	return kHandle;
#elif defined(__linux)
	key_t kKey;

	sprintf(acName, "%s", pcName);
	kKey = ftok(acName, 0);
	return shmget(kKey, ullSize, IPC_CREAT|IPC_EXCL|0666);
#endif
}

HANDLE SeOpenShareMemory(const char *pcName)
{
	char acName[128];

#if (defined(WIN32) || defined(_WIN32))
	sprintf(acName, "Global\\%s", pcName);
	return OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, acName);
#elif defined(__linux)
	key_t kKey;
	sprintf(acName, "%s", pcName);
	kKey = ftok(acName, 0);
	return shmget(kKey, 0, 0);
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
	char *shmadd;

	shmadd = (char*)shmat(kHandle, 0, !SHM_RDONLY);
	if(shmadd == -1) { return 0; }
	return (void*)shmadd;
#endif
}

bool SeUnViewShareMemory(void *pvBuf)
{
#if (defined(WIN32) || defined(_WIN32))
	return UnmapViewOfFile(pvBuf) == TRUE ? true : false;
#elif defined(__linux)
	return shmdt((const void*)pvBuf) == 0 ? true : false;
#endif
}

int SeGetShareMemoryErrorID()
{
	return SeErrno();
}