#ifndef __SE_NETBASE_H__
#define __SE_NETBASE_H__

/**********************************************************************
 *
 * 
 * 
 * NOTE: base net for win or linux
 * 
 *
 **********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

#if defined(__linux)
	#include <errno.h>
	#include <netdb.h>
	#include <sys/types.h>
	#include <string.h>
	#include <unistd.h>
	#include <sys/time.h>
	#include <sys/socket.h>
	#include <sys/epoll.h>
	#include <netinet/tcp.h>
	#include <netinet/in.h>
	#include <sys/ioctl.h>
	#include <arpa/inet.h>
	#include <sys/poll.h>

	#define SOCKET                  int
	#define SOCK_LEN                socklen_t
	#define HANDLE                  int

	#define SE_EWOULDBLOCK          EAGAIN
	#define SE_EINPROGRESS          EINPROGRESS
	#define SE_EINTR                EINTR
	#define SE_INVALID_SOCKET       -1
	#define SE_SOCKET_ERROR         -1
	#define SE_INVALID_HANDLE       -1

	#define SE_CONTAINING_RECORD(ptr, type, member) \
		((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

#elif (defined(_WIN32) || defined(WIN32))
	
	#include <winsock2.h>

	#ifdef _MSC_VER
		#pragma comment(lib, "ws2_32.lib")
	#endif

	#define SOCK_LEN                int

	#define SE_EWOULDBLOCK          WSAEWOULDBLOCK
	#define SE_EINPROGRESS          WSAEINPROGRESS
	#define SE_EINTR                WSAEINTR
	#define SE_INVALID_SOCKET       INVALID_SOCKET
	#define SE_SOCKET_ERROR         SOCKET_ERROR
	#define SE_INVALID_HANDLE       NULL

	#define SE_CONTAINING_RECORD(address, type, field) \
		((type *)((PCHAR)(address) - (ULONG_PTR)(&((type *)0)->field)))

#endif

typedef	unsigned long long HSOCKET;

HSOCKET SeGetHSocket(unsigned short usCounter,unsigned short usIndex,SOCKET kSocket);

unsigned short SeGetIndexByHScoket(HSOCKET kHSocket);

SOCKET SeGetSocketByHScoket(HSOCKET kHSocket);

void SeCloseHandle(HANDLE kHandle);

SOCKET SeSocket(int iType /*= SOCK_STREAM*/);

SOCKET SeAccept(SOCKET kSocket, struct sockaddr *pkAddr);

int SeCloseSocket(SOCKET kSocket);

int SeConnect(SOCKET kSocket, const struct sockaddr *pkAddr);

int SeShutDown(SOCKET kSocket);

int SeBind(SOCKET kSocket, const struct sockaddr *pkAddr);

int SeListen(SOCKET kSocket, int iCount);

int SeSelect(SOCKET kSocket, fd_set* pkReadFD, fd_set* pkWriteFD,fd_set* pkErrFD); // 不支持集合操作

int SeErrno(void);

int SeSend(SOCKET kSocket, const void *pkBuf, int iSize, int iFlag /*= 0*/);

int SeRecv(SOCKET kSocket, void *pkBuf, int iSize, int iFlag /*= 0*/);

int SeSendTo(SOCKET kSocket, const void *pkBuf, int iSize, const struct sockaddr *pkAddr, int iFlag /*= 0*/);

int SeRecvFrom(SOCKET kSocket, void *pkBuf, int iSize, struct sockaddr *pkAddr, int iFlag /*= 0*/);

bool SeFDIsSet(SOCKET kSocket, fd_set* pkFD);

void SeFdClr(SOCKET kSocket, fd_set* pkFD);

int SeIoCtl(SOCKET kSocket, long lCmd, unsigned long *pulArgp);

int SeSetSockOpt(SOCKET kSocket, int iLevel, int iOptname, const char *pcOptval, SOCK_LEN iOptlen);

int SeGetSockOpt(SOCKET kSocket, int iLevel, int iOptname, char *pcOptval, SOCK_LEN *piOptlen);

int SeGetSockName(SOCKET kSocket, struct sockaddr *pkAddr);

int SeGetPeerName(SOCKET kSocket, struct sockaddr *pkAddr);

void SeSetSockAddr(struct sockaddr *pkAddr, const char *pcIP, unsigned short usPort);

void SeErrStr(int iErrno, char *pcMsg, unsigned long ulSize);

int SeSetReuseAddr(SOCKET kSocket);

int SeSetNoBlock(SOCKET kSocket,bool bBlock /*= true*/);

int SeNetBaseInit(void);

void SeNetBaseEnd(void);

#endif



