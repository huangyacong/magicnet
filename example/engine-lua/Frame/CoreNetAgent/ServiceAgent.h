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
#include "SeThread.h"
#include "SeNetEngine.h"

enum AGENTSERVICE_PTYPE
{
	PTYPE_RESPONSE = 0,				//��ӦЭ����Ϣ
	PTYPE_CALL = 1,					//Э����Ϣ
	PTYPE_REMOTE = 2,				//���͸�Զ��Ŀ����������
	PTYPE_COMMON = 3,				//��ͨ����
	PTYPE_REGISTER_KEY = 4,			//ע��Key����
	PTYPE_REGISTER = 5,				//ע������
	PTYPE_REG_ADD_SERVICE = 6,		//���ӷ����б�����
	PTYPE_REG_DEL_SERVICE = 7,		//ɾ�������б�����
	PTYPE_PING = 8,					//Ping����
	PTYPE_EXIT = 9,					//�˳�
	PTYPE_REMOTE_CONNECT = 10,		//�µ�����
	PTYPE_REMOTE_DISCONNECT = 11,	//���ӶϿ�
	PTYPE_REMOTE_RECV_DATA = 12,	//���ӶϿ�
	PTYPE_REMOTE_CLOSE = 13,		//�����Ͽ�����
	PTYPE_REGISTER_BACK = 14,		//ע�����ͷ���
};

struct AgentServicePacket
{
	AgentServicePacket();
	char acSrcName[64];
	char acDstName[64];
	char acProto[32];
	AGENTSERVICE_PTYPE eType;
	unsigned long long ullSessionId;
};

struct ServiceSocket
{
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
public:
	bool SendRemoteData(HSOCKET kHSocket, unsigned short usProto, const char *pcBuf, int iLen);
	void CloseRemote(HSOCKET kHSocket);
private:
	std::map<HSOCKET, std::string> m_kRemoteList;
};

class ServiceForAgent : public SeServer
{
	void OnServerConnect(HSOCKET kHSocket, const char *pcIP, int iLen);
	void OnServerDisConnect(HSOCKET kHSocket);
	void OnServerRecv(HSOCKET kHSocket, const char *pcBuf, int iLen, int iSendSize, int iRecvSize);
	void SendCommonData(const std::string& rkDstName, const char *pcBuf, int iLen);
	void SendRemoteData(HSOCKET kHSocket, unsigned short usProto, const char *pcBuf, int iLen);
	void SendRegKey(HSOCKET kHSocket);
	void RegisterService(HSOCKET kHSocket, const std::string& rkName, const std::string& rkMD5);
	void SendPing(HSOCKET kHSocket);
	void CloseRemote(HSOCKET kHSocket);
	void Exit(HSOCKET kHSocket);
	void OnServerUpdate() {};
private:
	bool IsReg(HSOCKET kHSocket);
	void SendWatchdogAddRegService(const std::string& rkRegName);
	void SendWatchdogDelRegService(const std::string& rkRegName);
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
	ServiceAgent();
	~ServiceAgent();
public:
	void StopServiceAgent();
	void StartServiceAgent();
	bool Init(const char *pcLogName, int iLogLV, unsigned short usMax, int iTimerCnt);
	bool Listen(const char* IPRemote, int PortRemote, const char* IPService, int PortService, const char* IPServiceUnix, int iTimeOut);
public:
	static std::string m_kWatchDogName;
	static ServiceForRemote m_kServiceForRemote;
	static ServiceForAgent m_kServiceForAgentIPSocket;
	static ServiceForAgent m_kServiceForAgentUnixSocket;
	static ServiceAgenttEngine m_kServiceAgenttEngine;
	static std::map< std::string, std::pair<ServiceForAgent*, HSOCKET> > m_kRegSvrList;
private:
	std::string m_kLinkFile;
};

#endif
