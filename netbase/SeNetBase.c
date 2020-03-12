#include "SeNetBase.h"
#include "SeTool.h"

HSOCKET SeGetValidHSocket()
{
	HSOCKET ret;

	ret = 0x7FFFFFFFFFFFFFFF;
	return ret;
}

HSOCKET SeGetHSocket(unsigned short usIndex, unsigned long long ullTime)
{
	HSOCKET ret;
	unsigned long long ullIndex, ullTmp;

	ullIndex = usIndex;
	ullTmp = ((ullTime << 18) >> 18);
	ullTmp = (ullIndex <= 0 && ullTmp <= 0) ? (ullIndex + 1) : ullTmp;
	ret = (HSOCKET)((ullIndex << 46) | ullTmp);
	return ((ret << 2) >> 2) & 0x3FFFFFFFFFFFFFFF;
}

unsigned short SeGetIndexByHScoket(HSOCKET kHSocket)
{
	return (unsigned short)(kHSocket >> 46);
}

bool SeLocalIsLittleEndian()
{
	unsigned int uiTest = 0x12345678;
	unsigned char *pcTest = (unsigned char*)&uiTest;
	return pcTest[0] != 0x12;
}

unsigned int SeBigToLittleEndianL(unsigned int uiValue)
{
	return ntohl(uiValue);
}

unsigned int SeLittleToBigEndianL(unsigned int uiValue)
{
	return htonl(uiValue);
}

unsigned short SeBigToLittleEndianS(unsigned short usValue)
{
	return ntohs(usValue);
}

unsigned short SeLittleToBigEndianS(unsigned short usValue)
{
	return htons(usValue);
}

#define SWAP_LONGLONG(l)					  \
	((((l) >> 56) & 0x00000000000000FFLL) 	| \
	(((l) >> 40) & 0x000000000000FF00LL) 	| \
	(((l) >> 24) & 0x0000000000FF0000LL) 	| \
	(((l) >> 8) & 0x00000000FF000000LL) 	| \
	(((l) << 8) & 0x000000FF00000000LL) 	| \
	(((l) << 24) & 0x0000FF0000000000LL) 	| \
	(((l) << 40) & 0x00FF000000000000LL) 	| \
	(((l) << 56) & 0xFF00000000000000LL))

unsigned long long SeBigToLittleEndianLL(unsigned long long llValue)
{
	return SWAP_LONGLONG(llValue);
}

unsigned long long SeLittleToBigEndianLL(unsigned long long llValue)
{
	return SWAP_LONGLONG(llValue);
}

unsigned long long SeLittleToBigEndianDF(double dfValue) 
{ 
	unsigned long long Tempval;
	unsigned long long Retval;
	unsigned long long* p;

	p = (unsigned long long*)(&dfValue);
	Tempval = *p;
	Retval = SWAP_LONGLONG(Tempval);
	return Retval;
}

double SeBigToLittleEndianDF(unsigned long long ullValue) 
{ 
	double Retval;
	unsigned long long* p;
	const unsigned long long Tempval = SWAP_LONGLONG(ullValue);
	
	Retval = 0.0;
	p = (unsigned long long*)&Retval;
	*p = Tempval;
	return Retval;
}

#define SWAP_LONG(l)					\
	( ( ((l) >> 24) & 0x000000FFL )	|	\
	( ((l) >>  8) & 0x0000FF00L )	|	\
	( ((l) <<  8) & 0x00FF0000L )	|	\
	( ((l) << 24) & 0xFF000000L )	)

unsigned int SeLittleToBigEndianF(float fValue) 
{ 
	unsigned int Tempval;
	unsigned int Retval;
	unsigned int* p;
	
	p = (unsigned int*)(&fValue);
	Tempval = *p;
	Retval = SWAP_LONG(Tempval);
	return Retval;
}

float SeBigToLittleEndianF(unsigned int uiValue) 
{ 
	float Retval;
	unsigned int* p;
	const unsigned int Tempval = SWAP_LONG(uiValue);
	
	Retval = 0.0;
	p = (unsigned int*)&Retval;
	*p = Tempval;
	return Retval;
}

void SeCloseHandle(HANDLE kHandle)
{
#ifdef __linux
	close(kHandle);
#else
	CloseHandle(kHandle);
#endif
}

SOCKET SeSocket(int domain, int iType)
{
	return socket(domain, iType, 0);
}

SOCKET SeAccept(SOCKET kSocket, struct sockaddr *pkAddr, SOCK_LEN *riLen)
{
	return accept(kSocket, pkAddr, riLen);
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

int SeConnect(SOCKET kSocket, const struct sockaddr *pkAddr, SOCK_LEN kLen)
{
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

int SeBind(SOCKET kSocket, const struct sockaddr *pkAddr, SOCK_LEN iLen)
{
	return bind(kSocket, pkAddr, iLen);
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

SOCK_LEN SeSetSockAddr(int iDoMain, struct sockaddr_storage *pkAddr, const char *pcIP, unsigned short usPort)
{
	struct sockaddr_in *pkAddrIn;
	struct sockaddr_in6 *pkAddrIn6;

#ifdef __linux
	struct sockaddr_un *pkUn;

	if (iDoMain == SE_DOMAIN_UNIX)
	{
		assert(sizeof(struct sockaddr_storage) >= sizeof(struct sockaddr_un));
		pkUn = (struct sockaddr_un*)pkAddr;
		pkUn->sun_family  = iDoMain;
		SeStrNcpy(pkUn->sun_path, (int)sizeof(pkUn->sun_path), pcIP);
		return (int)sizeof(struct sockaddr_un);
	}

#endif

	if (iDoMain == SE_DOMAIN_INET)
	{
		assert(sizeof(struct sockaddr_storage) >= sizeof(struct sockaddr_in));
		pkAddrIn = (struct sockaddr_in*)pkAddr;
		pkAddrIn->sin_family = iDoMain;
		pkAddrIn->sin_addr.s_addr = inet_addr(pcIP);
		pkAddrIn->sin_port = htons(usPort);
		return (int)sizeof(struct sockaddr_storage);
	}

	if (iDoMain == SE_DOMAIN_INET6)
	{
		assert(sizeof(struct sockaddr_storage) >= sizeof(struct sockaddr_in6));
		pkAddrIn6 = (struct sockaddr_in6*)pkAddr;
		pkAddrIn6->sin6_family = iDoMain;
		inet_pton(iDoMain, pcIP, &pkAddrIn6->sin6_addr);
		pkAddrIn6->sin6_port = htons(usPort);
		return (int)sizeof(struct sockaddr_storage);
	}

	assert(true);
	return 0;
}

void SeSetAddrToBuf(int iDoMain, struct sockaddr_storage *pkAddr, char* pcIpBuf, int iLen, int* piPort)
{
	struct sockaddr_in *pkAddrIn;
	struct sockaddr_in6 *pkAddrIn6;

#ifdef __linux
	struct sockaddr_un *pkUn;

	if (iDoMain == SE_DOMAIN_UNIX)
	{
		pkUn = (struct sockaddr_un*)pkAddr;
		SeStrNcpy(pcIpBuf, iLen, pkUn->sun_path);
		*piPort = 0;
		return;
	}
#endif

	if (iDoMain == SE_DOMAIN_INET)
	{
		pkAddrIn = (struct sockaddr_in*)pkAddr;
		SeStrNcpy(pcIpBuf, iLen, inet_ntoa(pkAddrIn->sin_addr));
		*piPort = ntohs(pkAddrIn->sin_port);
		return;
	}

	if (iDoMain == SE_DOMAIN_INET6)
	{
		pkAddrIn6 = (struct sockaddr_in6*)pkAddr;
		inet_ntop(iDoMain, &pkAddrIn6->sin6_addr, pcIpBuf, iLen);
		*piPort = ntohs(pkAddrIn6->sin6_port);
		return;
	}

	assert(true);
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
