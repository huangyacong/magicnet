#include "SeNetBase.h"
#include "SeTool.h"

HSOCKET SeGetHSocket(unsigned short usCounter, unsigned short usIndex, SOCKET kSocket)
{
	HSOCKET ret;

	ret = (HSOCKET)(((HSOCKET)((unsigned int)usCounter << 16 | (unsigned int)usIndex)) << 32 | ((HSOCKET)kSocket));
	return (ret & 0x7FFFFFFFFFFFFFFF);
}

unsigned short SeGetIndexByHScoket(HSOCKET kHSocket)
{
	return (unsigned short)(((kHSocket>>32)<<16|0)>>16);
}

void SeCloseHandle(HANDLE kHandle)
{
#ifdef __linux
	close(kHandle);
#else
	CloseHandle(kHandle);
#endif
}

SOCKET SeSocket(int iType)
{
	return socket(AF_INET, iType, (iType == SOCK_STREAM ? IPPROTO_TCP : IPPROTO_UDP));
}

SOCKET SeAccept(SOCKET kSocket, struct sockaddr *pkAddr)
{
	SOCK_LEN kLen = sizeof(struct sockaddr);
	return accept(kSocket, pkAddr, &kLen);
}

int SeCloseSocket(SOCKET kSocket)
{
#ifdef __linux
	int iRet = close(kSocket);
#else
	int iRet = closesocket(kSocket);
#endif
	return iRet;
}

int SeConnect(SOCKET kSocket, const struct sockaddr *pkAddr)
{
	SOCK_LEN kLen = sizeof(struct sockaddr);
	return connect(kSocket, pkAddr, kLen);
}

int SeShutDown(SOCKET kSocket)
{
#ifdef __linux
	int iHow = SHUT_RDWR;
#else
	int iHow = SD_BOTH;
#endif
	return shutdown(kSocket, iHow);
}

int SeBind(SOCKET kSocket, const struct sockaddr *pkAddr)
{
	SOCK_LEN kLen = sizeof(struct sockaddr);
	return bind(kSocket, pkAddr, kLen);
}

int SeListen(SOCKET kSocket, int iCount)
{
	return listen(kSocket, iCount);
}

int SeSelect(SOCKET kSocket, fd_set* pkReadFD, fd_set* pkWriteFD, fd_set* pkErrFD)
{
	struct timeval kTimeval = {0, 0};

	if(pkReadFD)
	{
		FD_ZERO(pkReadFD);
		FD_SET(kSocket, pkReadFD);
	}
	if(pkWriteFD)
	{
		FD_ZERO(pkWriteFD);
		FD_SET(kSocket, pkWriteFD);
	}
	if(pkErrFD)
	{
		FD_ZERO(pkErrFD);
		FD_SET(kSocket, pkErrFD);
	}
	if(!pkReadFD && !pkWriteFD && !pkErrFD)
	{
		return -1;
	}

	return select((int)kSocket + 1, pkReadFD, pkWriteFD, pkErrFD, &kTimeval);
}

int SeErrno(void)
{
#ifdef __linux
	int iRet = errno;
#else
	int iRet = WSAGetLastError();
#endif
	return iRet;
}

int SeSend(SOCKET kSocket, const void *pkBuf, int iSize, int iFlag)
{
	return send(kSocket, (char*)pkBuf, iSize, iFlag);
}

int SeRecv(SOCKET kSocket, void *pkBuf, int iSize, int iFlag)
{
	return recv(kSocket, (char*)pkBuf, iSize, iFlag);
}

int SeSendTo(SOCKET kSocket, const void *pkBuf, int iSize, const struct sockaddr *pkAddr, int iFlag)
{
	SOCK_LEN kLen = sizeof(struct sockaddr);
	return sendto(kSocket, (char*)pkBuf, iSize, iFlag, pkAddr, kLen);
}

int SeRecvFrom(SOCKET kSocket, void *pkBuf, int iSize, struct sockaddr *pkAddr, int iFlag)
{
	SOCK_LEN kLen = sizeof(struct sockaddr);
	return recvfrom(kSocket, (char*)pkBuf, iSize, iFlag, pkAddr, &kLen);
}

bool SeFDIsSet(SOCKET kSocket, fd_set* pkFD)
{
	return FD_ISSET(kSocket,pkFD) == 0 ? false : true;
}

void SeFdClr(SOCKET kSocket, fd_set* pkFD)
{
	FD_CLR(kSocket,pkFD);
}

int SeIoCtl(SOCKET kSocket, long lCmd, unsigned long *pulArgp)
{
#ifdef __linux
	int iRet = ioctl(kSocket, lCmd, pulArgp);
#else
	int iRet = ioctlsocket(kSocket, lCmd, pulArgp);
#endif
	return iRet;	
}

int SeSetSockOpt(SOCKET kSocket, int iLevel, int iOptname, const char *pcOptval, SOCK_LEN iOptlen)
{
	return setsockopt(kSocket, iLevel, iOptname, pcOptval, iOptlen);
}

int SeGetSockOpt(SOCKET kSocket, int iLevel, int iOptname, char *pcOptval, SOCK_LEN *piOptlen)
{
	return getsockopt(kSocket, iLevel, iOptname, pcOptval, piOptlen);
}

int SeSetNoDelay(SOCKET kSocket)
{
	int iFlag = 1;
	return SeSetSockOpt(kSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&iFlag, sizeof(iFlag));
}

int SeGetSockName(SOCKET kSocket, struct sockaddr *pkAddr)
{
	SOCK_LEN kLen = sizeof(struct sockaddr);
	return getsockname(kSocket, pkAddr, &kLen);
}

int SeGetPeerName(SOCKET kSocket, struct sockaddr *pkAddr)
{
	SOCK_LEN kLen = sizeof(struct sockaddr);
	return getpeername(kSocket, pkAddr, &kLen);
}

void SeSetSockAddr(struct sockaddr *pkAddr, const char *pcIP, unsigned short usPort)
{
	struct sockaddr_in *pkAddrIn = (struct sockaddr_in*)pkAddr;
	pkAddrIn->sin_family = AF_INET;
	pkAddrIn->sin_addr.s_addr = inet_addr(pcIP);
	pkAddrIn->sin_port = htons(usPort);
}

void SeErrStr(int iErrno, char *pcMsg, unsigned long ulSize)
{
#ifdef __linux
	strerror_r(iErrno, pcMsg, ulSize);
#elif (defined(_WIN32) || defined(WIN32))
	LPVOID lpMessageBuf;
	unsigned long ulLen = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, iErrno, MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), (LPTSTR) &lpMessageBuf, 0, NULL);
	SeStrNcpy(pcMsg, (int)ulSize, (char*)lpMessageBuf);
	LocalFree(lpMessageBuf);
#endif
}

int SeSetReuseAddr(SOCKET kSocket)
{
	unsigned long value = 1;
	return SeSetSockOpt(kSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&value, sizeof(value));
}

int SeSetReusePort(SOCKET kSocket)
{
#ifdef __linux
	unsigned long value = 1;
	return SeSetSockOpt(kSocket, SOL_SOCKET, SO_REUSEPORT, (char*)&value, sizeof(value));
#elif (defined(_WIN32) || defined(WIN32))
	return 0;
#endif
}

int SeSetExclusiveAddruse(SOCKET kSocket)
{
#ifdef __linux
	return 0;
#elif (defined(_WIN32) || defined(WIN32))
	int value = 1;
	return SeSetSockOpt(kSocket, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char*)&value, sizeof(value));
#endif
}

int SeSetNoBlock(SOCKET kSocket,bool bBlock)
{
	unsigned long value = 0;
	if(bBlock == true) value = 1;
	else value = 0;
	return SeIoCtl(kSocket, FIONBIO, (unsigned long*)&value);
}

int SeNetBaseInit(void)
{
#if (defined(_WIN32) || defined(WIN32))
	WSADATA WSAData;
	int iRet = WSAStartup(0x202, &WSAData);
	if(iRet != 0|| WSAData.wVersion != 0x202)
	{
		SeNetBaseEnd();
		return -1;
	}
#elif defined(__linux)
	assert(EAGAIN == EWOULDBLOCK);
#endif
	assert(sizeof(HSOCKET) == 8);
	return 0;
}

void SeNetBaseEnd(void)
{
#if (defined(_WIN32) || defined(WIN32))
	WSACleanup();
#endif
}
