#ifndef __SE_NETBASE_H__
#define __SE_NETBASE_H__

#ifdef	__cplusplus
extern "C" {
#endif

/**********************************************************************
 *
 * 
 * 
 * NOTE: base net for win or linux
   网路字节是大端表示 H 是主机字节 S表示网络字节
   内存的高低地址从左到右依次是低内存到高内存 低---->高 布局
   0x12345678 十六进制前面表示从高位到低位 高位--->低位 布局
   大端模式就是指把数据的高字节保存在内存的低地址中，数据的低字节保存在内存的高地址中
   小端模式则相反，数据的高字节位置保存在内存的高地址处，数据的低字节保存在内存的低地址处
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
	#include <sys/timerfd.h>
	#include <sys/eventfd.h>
	#include <sys/un.h>

	#define SE_DOMAIN_INET			AF_INET
	#define SE_DOMAIN_UNIX			AF_UNIX

	#define SOCKET					int
	#define SOCK_LEN				socklen_t
	#define HANDLE 					int

	#define SE_EWOULDBLOCK			EAGAIN
	#define SE_EINPROGRESS			EINPROGRESS
	#define SE_EINTR				EINTR
	#define SE_INVALID_SOCKET		-1
	#define SE_SOCKET_ERROR			-1
	#define SE_INVALID_HANDLE		-1

#elif (defined(_WIN32) || defined(WIN32))
	
	#include <winsock2.h>
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#include <mswsock.h>
	#include <stdio.h>

	#ifdef _MSC_VER
		#pragma comment(lib, "ws2_32.lib")
	#endif

	#define SE_DOMAIN_INET			AF_INET
	#define SE_DOMAIN_UNIX			AF_UNSPEC

	#define SOCK_LEN				int

	#define SE_EWOULDBLOCK			WSAEWOULDBLOCK
	#define SE_EINPROGRESS			WSAEINPROGRESS
	#define SE_EINTR				WSAEINTR
	#define SE_INVALID_SOCKET		INVALID_SOCKET
	#define SE_SOCKET_ERROR			SOCKET_ERROR
	#define SE_INVALID_HANDLE		INVALID_HANDLE_VALUE

#endif

typedef	unsigned long long HSOCKET;

HSOCKET SeGetValidHSocket();

HSOCKET SeGetHSocket(unsigned short usIndex, unsigned long long ullTime);

unsigned short SeGetIndexByHScoket(HSOCKET kHSocket);

bool SeLocalIsLittleEndian();

unsigned int SeBigToLittleEndianL(unsigned int uiValue);

unsigned int SeLittleToBigEndianL(unsigned int uiValue);

unsigned short SeBigToLittleEndianS(unsigned short usValue);

unsigned short SeLittleToBigEndianS(unsigned short usValue);

unsigned long long SeBigToLittleEndianLL(unsigned long long llValue);

unsigned long long SeLittleToBigEndianLL(unsigned long long llValue);

unsigned long long SeLittleToBigEndianDF(double dfValue);

double SeBigToLittleEndianDF(unsigned long long ullValue);

unsigned int SeLittleToBigEndianF(float fValue);

float SeBigToLittleEndianF(unsigned int uiValue);

void SeCloseHandle(HANDLE kHandle);

SOCKET SeSocket(int domain /*=SE_DOMAIN_INET,SE_DOMAIN_UNIX*/, int iType /*= SOCK_STREAM*/);

SOCKET SeAccept(SOCKET kSocket, struct sockaddr *pkAddr, SOCK_LEN *riLen);

int SeCloseSocket(SOCKET kSocket);

int SeConnect(SOCKET kSocket, const struct sockaddr *pkAddr, SOCK_LEN kLen);

int SeShutDown(SOCKET kSocket);

int SeBind(SOCKET kSocket, const struct sockaddr *pkAddr, SOCK_LEN iLen);

int SeListen(SOCKET kSocket, int iCount);

int SeSelect(SOCKET kSocket, fd_set* pkReadFD, fd_set* pkWriteFD, fd_set* pkErrFD); // 不支持集合操作

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

int SeSetNoDelay(SOCKET kSocket);

int SeGetSockName(SOCKET kSocket, struct sockaddr *pkAddr);

int SeGetPeerName(SOCKET kSocket, struct sockaddr *pkAddr);

void SeSetSockAddr(int iDoMain, void *pkAddr, const char *pcIP, unsigned short usPort);

void SeSetAddrToBuf(int iDoMain, void *pkAddr, char* pcIpBuf, int iLen, int* piPort);

void SeErrStr(int iErrno, char *pcMsg, unsigned long ulSize);

int SeSetReuseAddr(SOCKET kSocket);

int SeSetReusePort(SOCKET kSocket);

int SeSetExclusiveAddruse(SOCKET kSocket);

int SeSetNoBlock(SOCKET kSocket, bool bBlock /*= true*/);

int SeNetBaseInit(void);

void SeNetBaseEnd(void);

#ifdef	__cplusplus
}
#endif

#endif
