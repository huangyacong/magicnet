#ifndef __SERVICE_AGENT_H__
#define __SERVICE_AGENT_H__

#ifdef	__cplusplus
extern "C" {
#endif

#include <lua.h>
#include <lauxlib.h>
#include <time.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <lualib.h>
#include "SeMD5.h"
#include "SeTime.h"
#include "SeNetCore.h"
#include "SeBuffWR.h"

#ifdef	__cplusplus
}
#endif

#include "SeTime.h"
#include "SeTool.h"
#include "SeServer.h"
#include "SeCommon.h"
#include "SeNetEngine.h"

enum AGENTSERVICE_PTYPE
{
	PTYPE_RESPONSE = 0,			//回应协程消息
	PTYPE_CALL = 1,				//协程消息
	PTYPE_REMOTE = 2,			//发送给远程目标数据类型
	PTYPE_COMMON = 3,			//普通类型
	PTYPE_REGISTER_KEY = 4,		//注册Key类型
	PTYPE_REGISTER = 5,			//注册类型
	PTYPE_PING = 6,				//Ping类型
	PTYPE_REMOTE_CONNECT = 7,	//新的链接
	PTYPE_REMOTE_DISCONNECT = 8,//链接断开
	PTYPE_REMOTE_RECV_DATA = 9,	//链接断开
};

struct AgentServicePacket
{
	AgentServicePacket();
	char acSrcName[64];
	char acDstName[64];
	char acProto[30];
	AGENTSERVICE_PTYPE eType;
	unsigned long long ullSessionId;
};

struct ServiceSocket
{
	bool m_bSendKey;
	std::string m_kIP;
	std::string m_kTokenKey;
	std::string m_kRegName;
};

int NetPack(const AgentServicePacket& kPacket, unsigned char* pcBuff, int iSize);
int NetUnPack(AgentServicePacket& kPacket, const unsigned char* pcBuff, int iSize);

std::string GenRegToken(const std::string kKey, const std::string kName);

class ServiceForRemote : public SeServer
{
	void OnServerConnect(HSOCKET kHSocket, const char *pcIP, int iLen);
	void OnServerDisConnect(HSOCKET kHSocket);
	void OnServerRecv(HSOCKET kHSocket, const char *pcBuf, int iLen, int iSendSize, int iRecvSize);
	void OnServerUpdate() {};
private:
	std::map<HSOCKET, std::string> m_kRemoteList;
};

class ServiceForAgent : public SeServer
{
	void OnServerConnect(HSOCKET kHSocket, const char *pcIP, int iLen);
	void OnServerDisConnect(HSOCKET kHSocket);
	void OnServerRecv(HSOCKET kHSocket, const char *pcBuf, int iLen, int iSendSize, int iRecvSize);
	void SendCommonData(const std::string& rkDstName, const char *pcBuf, int iLen);
	void SendRemoteData(HSOCKET kHSocket, const char *pcBuf, int iLen);
	void SendRegKey(HSOCKET kHSocket);
	void RegisterService(HSOCKET kHSocket, const std::string& rkName, const std::string& rkMD5);
	void SendPing(HSOCKET kHSocket);
	void OnServerUpdate() {};
private:
	std::map<HSOCKET, ServiceSocket> m_kServiceList;
};

class ServiceAgenttEngine : public SeNetEngine
{
	void OnUpdate(){};
	void OnPrintStat(int iSendNum, unsigned long long ullSendSpeed, int iRecvNum, unsigned long long ullRecvSpeed);
};

class ServiceAgent
{
public:
	bool Init(const char *pcLogName, int iLogLV, unsigned short usMax, int iTimerCnt);
	bool Listen(const char* IPRemote, int PortRemote, const char* IPService, int PortService, const char* IPServiceUnix, int iTimeOut);
public:
	static std::string m_kWatchDogName;
	static char m_acBuff[1024 * 1024 * 4];
	static ServiceForRemote m_kServiceForRemote;
	static ServiceForAgent m_kServiceForAgentIPSocket;
	static ServiceForAgent m_kServiceForAgentUnixSocket;
	static ServiceAgenttEngine m_kServiceAgenttEngine;
	static std::map< std::string, std::pair<ServiceForAgent*, HSOCKET> > m_kRegSvrList;
};

#endif
