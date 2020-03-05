#include "ServiceAgent.h"

char ServiceAgent::m_acBuff[1024 * 1024 * 4] = {};
ServiceForRemote ServiceAgent::m_kServiceForRemote;
ServiceForAgent ServiceAgent::m_kServiceForAgentIPSocket;
ServiceForAgent ServiceAgent::m_kServiceForAgentUnixSocket;
ServiceAgenttEngine ServiceAgent::m_kServiceAgenttEngine;
std::string ServiceAgent::m_kWatchDogName = std::string(".watchdog");
std::map< std::string, std::pair<ServiceForAgent*, HSOCKET> > ServiceAgent::m_kRegSvrList;

AgentServicePacket::AgentServicePacket()
{
	memset(acSrcName, 0, (int)sizeof(acSrcName));
	memset(acDstName, 0, (int)sizeof(acDstName));
	memset(acProto, 0, (int)sizeof(acProto));
}

int NetPack(const AgentServicePacket& kPacket, unsigned char* pcBuff, int iSize)
{
	int iIndex = 0;
	int iTotalSize = 0;

	iTotalSize += (int)sizeof(kPacket.acSrcName);
	iTotalSize += (int)sizeof(kPacket.acDstName);
	iTotalSize += (int)sizeof(kPacket.acProto);
	iTotalSize += (int)sizeof(kPacket.eType);
	iTotalSize += (int)sizeof(kPacket.ullSessionId);

	if (sizeof(kPacket.eType) != sizeof(int))
		return 0;

	if (iTotalSize > iSize)
		return 0;

	SeBuffWriteChar(pcBuff, (const unsigned char*)kPacket.acSrcName, (int)sizeof(kPacket.acSrcName), &iIndex);
	SeBuffWriteChar(pcBuff, (const unsigned char*)kPacket.acDstName, (int)sizeof(kPacket.acDstName), &iIndex);
	SeBuffWriteChar(pcBuff, (const unsigned char*)kPacket.acProto, (int)sizeof(kPacket.acProto), &iIndex);
	SeBuffWriteInt(pcBuff, kPacket.eType, &iIndex);
	SeBuffWriteLongLong(pcBuff, kPacket.ullSessionId, &iIndex);

	return iIndex;
}

int NetUnPack(AgentServicePacket& kPacket, const unsigned char* pcBuff, int iSize)
{
	int iIndex = 0;
	int iTotalSize = 0;

	iTotalSize += (int)sizeof(kPacket.acSrcName);
	iTotalSize += (int)sizeof(kPacket.acDstName);
	iTotalSize += (int)sizeof(kPacket.acProto);
	iTotalSize += (int)sizeof(kPacket.eType);
	iTotalSize += (int)sizeof(kPacket.ullSessionId);

	if (sizeof(kPacket.eType) != sizeof(int))
		return 0;

	if (iSize < iTotalSize)
		return 0;

	SeBuffReadChar(pcBuff, (unsigned char*)kPacket.acSrcName, (int)sizeof(kPacket.acSrcName), &iIndex);
	SeBuffReadChar(pcBuff, (unsigned char*)kPacket.acDstName, (int)sizeof(kPacket.acDstName), &iIndex);
	SeBuffReadChar(pcBuff, (unsigned char*)kPacket.acProto, (int)sizeof(kPacket.acProto), &iIndex);
	kPacket.eType = (AGENTSERVICE_PTYPE)SeBuffReadInt(pcBuff, &iIndex);
	kPacket.ullSessionId = SeBuffReadLongLong(pcBuff, &iIndex);

	kPacket.acSrcName[(int)sizeof(kPacket.acSrcName) - 1] = '\0';
	kPacket.acDstName[(int)sizeof(kPacket.acDstName) - 1] = '\0';
	kPacket.acProto[(int)sizeof(kPacket.acProto) - 1] = '\0';

	return iIndex;
}

std::string GenRegToken(const std::string kKey, const std::string kName)
{
	char acMD5[33] = {};
	std::string kBuff = kKey + kName;
	SeMD5(acMD5, kBuff.c_str(), (unsigned int)kBuff.length());
	return std::string(acMD5);
}

void ServiceForRemote::OnServerConnect(HSOCKET kHSocket, const char *pcIP, int iLen)
{
	if (ServiceAgent::m_kRegSvrList.find(ServiceAgent::m_kWatchDogName) != ServiceAgent::m_kRegSvrList.end())
		m_kRemoteList[kHSocket] = std::string(pcIP);
	else
	{
		DisConnect(kHSocket);
		NETENGINE_FLUSH_LOG(ServiceAgent::m_kServiceAgenttEngine, LT_ERROR, "socket=%llx disconnect, watchdog not connect", kHSocket);
	}

	if (m_kRemoteList.find(kHSocket) != m_kRemoteList.end() && ServiceAgent::m_kRegSvrList.find(ServiceAgent::m_kWatchDogName) != ServiceAgent::m_kRegSvrList.end())
	{
		AgentServicePacket kPacket;
		kPacket.eType = PTYPE_REMOTE_CONNECT;
		kPacket.ullSessionId = kHSocket;
		SeStrNcpy(kPacket.acSrcName, (int)sizeof(kPacket.acSrcName), pcIP);
		int iLen = NetPack(kPacket, (unsigned char*)ServiceAgent::m_acBuff, (int)sizeof(ServiceAgent::m_acBuff));
		std::pair<ServiceForAgent*, HSOCKET>& rkObj = ServiceAgent::m_kRegSvrList[ServiceAgent::m_kWatchDogName];
		rkObj.first->SendData(rkObj.second, ServiceAgent::m_acBuff, iLen);
	}
}

void ServiceForRemote::OnServerDisConnect(HSOCKET kHSocket)
{
	if (m_kRemoteList.find(kHSocket) != m_kRemoteList.end() && ServiceAgent::m_kRegSvrList.find(ServiceAgent::m_kWatchDogName) != ServiceAgent::m_kRegSvrList.end())
	{
		AgentServicePacket kPacket;
		kPacket.eType = PTYPE_REMOTE_DISCONNECT;
		kPacket.ullSessionId = kHSocket;
		int iLen = NetPack(kPacket, (unsigned char*)ServiceAgent::m_acBuff, (int)sizeof(ServiceAgent::m_acBuff));
		std::pair<ServiceForAgent*, HSOCKET>& rkObj = ServiceAgent::m_kRegSvrList[ServiceAgent::m_kWatchDogName];
		rkObj.first->SendData(rkObj.second, ServiceAgent::m_acBuff, iLen);
	}
	m_kRemoteList.erase(kHSocket);
}

void ServiceForRemote::OnServerRecv(HSOCKET kHSocket, const char *pcBuf, int iLen, int iSendSize, int iRecvSize)
{
	int iSize = 0;
	const int iHeaderLen = 2;
	unsigned short usProto = 0;

	if (iHeaderLen > iLen)
	{
		DisConnect(kHSocket);
		NETENGINE_FLUSH_LOG(ServiceAgent::m_kServiceAgenttEngine, LT_ERROR, "socket=%llx ServiceForRemote::OnServerRecv header error", kHSocket);
		return;
	}

	if (!SeNetSreamGetHeader((const unsigned char*)pcBuf, iHeaderLen, &iSize))
	{
		DisConnect(kHSocket);
		NETENGINE_FLUSH_LOG(ServiceAgent::m_kServiceAgenttEngine, LT_ERROR, "socket=%llx ServiceForRemote::OnServerRecv SeGetProtoHeader error", kHSocket);
		return;
	}

	usProto = (unsigned short)iSize;
	if (m_kRemoteList.find(kHSocket) != m_kRemoteList.end() && ServiceAgent::m_kRegSvrList.find(ServiceAgent::m_kWatchDogName) != ServiceAgent::m_kRegSvrList.end())
	{
		AgentServicePacket kPacket;
		kPacket.eType = PTYPE_REMOTE_RECV_DATA;
		kPacket.ullSessionId = kHSocket;
		SeStrNcpy(kPacket.acProto, (int)sizeof(kPacket.acProto), SeUnSignedShortToA(usProto).c_str());
		int iPacketLen = NetPack(kPacket, (unsigned char*)ServiceAgent::m_acBuff, (int)sizeof(ServiceAgent::m_acBuff));
		std::pair<ServiceForAgent*, HSOCKET>& rkObj = ServiceAgent::m_kRegSvrList[ServiceAgent::m_kWatchDogName];
		rkObj.first->SendData(rkObj.second, ServiceAgent::m_acBuff, iPacketLen, &pcBuf[iHeaderLen], iLen - iHeaderLen);
	}
}

bool ServiceForRemote::SendRemoteData(HSOCKET kHSocket, unsigned short usProto, const char *pcBuf, int iLen)
{
	const int iHeaderLen = 2;
	char acHeader[iHeaderLen] = {};
	
	if (!SeNetSreamSetHeader((unsigned char*)acHeader, iHeaderLen, usProto))
	{
		DisConnect(kHSocket);
		NETENGINE_FLUSH_LOG(ServiceAgent::m_kServiceAgenttEngine, LT_ERROR, "socket=%llx ServiceForRemote::OnServerRecv SeSetProtoHeader error", kHSocket);
		return false;
	}

	return SendData(kHSocket, acHeader, iHeaderLen, pcBuf, iLen);
}

void ServiceForAgent::OnServerConnect(HSOCKET kHSocket, const char *pcIP, int iLen)
{
	m_kServiceList[kHSocket] = ServiceSocket();
	m_kServiceList[kHSocket].m_kIP = pcIP;
	m_kServiceList[kHSocket].m_bSendKey = false;
	m_kServiceList[kHSocket].m_kTokenKey = SeLongLongToA(kHSocket) + SeLongLongToA(SeTimeGetTickCount());
}

void ServiceForAgent::OnServerDisConnect(HSOCKET kHSocket)
{
	if (m_kServiceList.find(kHSocket) == m_kServiceList.end())
		return;
	const ServiceSocket& rkServiceSocket = m_kServiceList[kHSocket];
	if (ServiceAgent::m_kRegSvrList.find(rkServiceSocket.m_kRegName) != ServiceAgent::m_kRegSvrList.end())
	{
		if (ServiceAgent::m_kRegSvrList[rkServiceSocket.m_kRegName].second == kHSocket)
		{
			ServiceAgent::m_kRegSvrList.erase(rkServiceSocket.m_kRegName);
			NETENGINE_FLUSH_LOG(ServiceAgent::m_kServiceAgenttEngine, LT_INFO, "Service %s Unregister", rkServiceSocket.m_kRegName.c_str());
		}
	}
	m_kServiceList.erase(kHSocket);
}

void ServiceForAgent::OnServerRecv(HSOCKET kHSocket, const char *pcBuf, int iLen, int iSendSize, int iRecvSize)
{
	AgentServicePacket kPacket;
	int iPacketLen = NetUnPack(kPacket, (const unsigned char*)pcBuf, iLen);
	if (iPacketLen <= 0)
	{
		NETENGINE_FLUSH_LOG(ServiceAgent::m_kServiceAgenttEngine, LT_ERROR, "socket=%llx ServiceForAgent::OnServerRecv NetUnPack error", kHSocket);
		return;
	}
	if (iPacketLen > iLen)
	{
		NETENGINE_FLUSH_LOG(ServiceAgent::m_kServiceAgenttEngine, LT_ERROR, "socket=%llx ServiceForAgent::OnServerRecv iPacketLen=%d > iLen=%d error", kHSocket, iPacketLen, iLen);
		return;
	}

	switch (kPacket.eType)
	{
	case PTYPE_RESPONSE:			//回应协程消息
		SendCommonData(kPacket.acDstName, pcBuf, iLen);
		break;
	case PTYPE_CALL:				//协程消息
		SendCommonData(kPacket.acDstName, pcBuf, iLen);
		break;
	case PTYPE_REMOTE:				//发送给远程目标数据类型
		SendRemoteData((HSOCKET)kPacket.ullSessionId, (unsigned short)SeAToInt(kPacket.acProto), &pcBuf[iPacketLen], iLen - iPacketLen);
		break;
	case PTYPE_COMMON:				//普通类型
		SendCommonData(kPacket.acDstName, pcBuf, iLen);
		break;
	case PTYPE_REGISTER_KEY:		//注册Key类型
		SendRegKey(kHSocket);
		break;
	case PTYPE_REGISTER:			//注册类型
		RegisterService(kHSocket, kPacket.acSrcName, kPacket.acDstName);
		break;
	case PTYPE_PING:				//Ping类型
		SendPing(kHSocket);
		break;
	default:
		break;
	}
}

void ServiceForAgent::SendRemoteData(HSOCKET kHSocket, unsigned short usProto, const char *pcBuf, int iLen)
{
	ServiceAgent::m_kServiceForRemote.SendRemoteData(kHSocket, usProto, pcBuf, iLen);
}

void ServiceForAgent::SendCommonData(const std::string& rkDstName, const char *pcBuf, int iLen)
{
	if (ServiceAgent::m_kRegSvrList.find(rkDstName) != ServiceAgent::m_kRegSvrList.end())
	{
		std::pair<ServiceForAgent*, HSOCKET>& rkObj = ServiceAgent::m_kRegSvrList[rkDstName];
		rkObj.first->SendData(rkObj.second, pcBuf, iLen);
		return;
	}
	NETENGINE_FLUSH_LOG(ServiceAgent::m_kServiceAgenttEngine, LT_ERROR, "ServiceForAgent::SendCommonData not find target=%s", rkDstName.c_str());
}

void ServiceForAgent::SendRegKey(HSOCKET kHSocket)
{
	if (m_kServiceList.find(kHSocket) == m_kServiceList.end())
		return;
	ServiceSocket& rkServiceSocket = m_kServiceList[kHSocket];
	if (rkServiceSocket.m_kRegName.length() > 0 || rkServiceSocket.m_bSendKey == true)
	{
		DisConnect(kHSocket);
		NETENGINE_FLUSH_LOG(ServiceAgent::m_kServiceAgenttEngine, LT_ERROR, "Service send register error srcRegName=%s SendKey=%s", rkServiceSocket.m_kRegName.c_str(), rkServiceSocket.m_bSendKey ? "get more":"");
		return;
	}
	rkServiceSocket.m_bSendKey = true;

	AgentServicePacket kPacket;
	kPacket.eType = PTYPE_REGISTER_KEY;
	SeStrNcpy(kPacket.acSrcName, (int)sizeof(kPacket.acSrcName), rkServiceSocket.m_kTokenKey.c_str());
	int iLen = NetPack(kPacket, (unsigned char*)ServiceAgent::m_acBuff, (int)sizeof(ServiceAgent::m_acBuff));
	SendData(kHSocket, ServiceAgent::m_acBuff, iLen);
	NETENGINE_FLUSH_LOG(ServiceAgent::m_kServiceAgenttEngine, LT_DEBUG, "Service send register key=%s %llx", rkServiceSocket.m_kTokenKey.c_str(), kHSocket);
}

void ServiceForAgent::RegisterService(HSOCKET kHSocket, const std::string& rkName, const std::string& rkMD5)
{
	if (m_kServiceList.find(kHSocket) == m_kServiceList.end())
		return;
	ServiceSocket& rkServiceSocket = m_kServiceList[kHSocket];
	if (rkServiceSocket.m_kRegName.length() > 0)
	{
		DisConnect(kHSocket);
		NETENGINE_FLUSH_LOG(ServiceAgent::m_kServiceAgenttEngine, LT_ERROR, "Service [%s] register more reg", rkName.c_str());
		return;
	}
	if (rkName.length() <= 0)
	{
		DisConnect(kHSocket);
		NETENGINE_FLUSH_LOG(ServiceAgent::m_kServiceAgenttEngine, LT_ERROR, "Service [%s] register regName is empty", rkName.c_str());
		return;
	}
	if (ServiceAgent::m_kRegSvrList.find(rkName) != ServiceAgent::m_kRegSvrList.end())
	{
		DisConnect(kHSocket);
		NETENGINE_FLUSH_LOG(ServiceAgent::m_kServiceAgenttEngine, LT_ERROR, "Service %s register regName error", rkName.c_str());
		return;
	}
	if (GenRegToken(rkServiceSocket.m_kTokenKey, rkName) != rkMD5)
	{
		DisConnect(kHSocket);
		NETENGINE_FLUSH_LOG(ServiceAgent::m_kServiceAgenttEngine, LT_ERROR, "Service %s register md5 error", rkName.c_str());
		return;
	}
	rkServiceSocket.m_kRegName = rkName;
	ServiceAgent::m_kRegSvrList[rkName] = std::pair<ServiceForAgent*, HSOCKET>(this, kHSocket);
	NETENGINE_FLUSH_LOG(ServiceAgent::m_kServiceAgenttEngine, LT_INFO, "Service %s register", rkName.c_str());
}

void ServiceForAgent::SendPing(HSOCKET kHSocket)
{
	if (m_kServiceList.find(kHSocket) == m_kServiceList.end())
		return;
	const ServiceSocket& rkServiceSocket = m_kServiceList[kHSocket];
	if (rkServiceSocket.m_kRegName.length() <= 0)
		return;
	AgentServicePacket kPacket;
	kPacket.eType = PTYPE_PING;
	int iLen = NetPack(kPacket, (unsigned char*)ServiceAgent::m_acBuff, (int)sizeof(ServiceAgent::m_acBuff));
	SendData(kHSocket, ServiceAgent::m_acBuff, iLen);
}

static void ServiceAgentThread(void *)
{
	ServiceAgent::m_kServiceAgenttEngine.StartEngine();
	SeExitThread();
}

ServiceAgent::ServiceAgent()
{
	bCreateThread = false;
}

ServiceAgent::~ServiceAgent()
{
	if (bCreateThread)
	{
		SeJoinThread(m_kThreadHand);
	}
#if defined(__linux)
	unlink(m_kLinkFile.c_str());
	NETENGINE_FLUSH_LOG(m_kServiceAgenttEngine, LT_INFO, "unlink %s", m_kLinkFile.c_str());
#endif
}

void ServiceAgent::StopServiceAgent()
{
	m_kServiceAgenttEngine.StopEngine();
}

void ServiceAgent::CreateThreadAndRunServiceAgent()
{
	bCreateThread = true;
	m_kThreadHand = SeCreateThread(ServiceAgentThread, NULL);
}

bool ServiceAgent::Init(const char *pcLogName, int iLogLV, unsigned short usMax, int iTimerCnt)
{
	m_kServiceAgenttEngine.SetStatDelayTime(5000);
	return m_kServiceAgenttEngine.Init(pcLogName, iLogLV, usMax, iTimerCnt);
}

bool ServiceAgent::Listen(const char* IPRemote, int PortRemote, const char* IPService, int PortService, const char* IPServiceUnix, int iTimeOut)
{
	m_kServiceForRemote.Init(&m_kServiceAgenttEngine, SE_DOMAIN_INET, IPRemote, PortRemote, iTimeOut, false);
	if (!m_kServiceForRemote.Listen())
		return false;
	m_kServiceForAgentIPSocket.Init(&m_kServiceAgenttEngine, SE_DOMAIN_INET, IPService, PortService, iTimeOut, true);
	if (!m_kServiceForAgentIPSocket.Listen())
		return false;
#if defined(__linux)
	m_kServiceForAgentUnixSocket.Init(&m_kServiceEngine, SE_DOMAIN_UNIX, IPServiceUnix, 0, iTimeOut, true);
	if (!m_kServiceForAgentUnixSocket.Listen())
		return false;
	m_kLinkFile = IPServiceUnix;
	NETENGINE_FLUSH_LOG(m_kServiceAgenttEngine, LT_INFO, "link %s", IPServiceUnix);
#endif
	return true;
}

void ServiceAgenttEngine::OnPrintStat(int iSendNum, unsigned long long ullSendSpeed, int iRecvNum, unsigned long long ullRecvSpeed)
{
	NETENGINE_FLUSH_LOG(*this, LT_INFO, "State Report SendNum=%d  SendSpeed=%lld RecvNum=%d RecvSpeed=%lld", iSendNum, ullSendSpeed, iRecvNum, ullRecvSpeed);
}


