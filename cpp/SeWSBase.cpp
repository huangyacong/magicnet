#include "SeWSBase.h"
#include "SeCommon.h"
#include "SeBase64.h"
#include "SeSha1.h"

SeWSBase::SeWSBase(SeNetEngine* pkSeNetEngine, HSOCKET kHSocket, const string& strIP)
{
	m_kHSocket = kHSocket;
	m_bHandShake = false;
	m_pkSeNetEngine = pkSeNetEngine;
	SeNetSreamInit(&m_kRecvNetStream);
	m_strIP = strIP;
	m_pkWSFrame = new SeWSFrame(m_pkSeNetEngine);
}

SeWSBase::~SeWSBase()
{
	SESOCKETMGR *pkNetSocketMgr = &m_pkSeNetEngine->GetNetScore().kSocketMgr;
	
	SENETSTREAMNODE *pkNetStreamNode = SeNetSreamHeadPop(&m_kRecvNetStream);
	while(pkNetStreamNode)
	{
		SeNetSreamHeadAdd(&(pkNetSocketMgr->kNetStreamIdle), pkNetStreamNode);
		pkNetStreamNode = SeNetSreamHeadPop(&m_kRecvNetStream);
	}

	while (!m_vecFrameData.empty())
	{
		SeWSFrame* pkSeWSFrame = m_vecFrameData.back();
		delete pkSeWSFrame;
		m_vecFrameData.pop_back();
	}

	if (m_pkWSFrame)
	{
		delete m_pkWSFrame;
		m_pkWSFrame = NULL;
	}

	m_pkSeNetEngine = NULL;
}

bool SeWSBase::ServerHandShake(bool& bHandShakeOK)
{
	int iCount = SeNetSreamCount(&m_kRecvNetStream);
	if (iCount > 1)
	{
		SeLogWrite(&m_pkSeNetEngine->GetNetScore().kLog, LT_SOCKET, true, "[ServerHandShake] recv more data in handshake.socket=0x%llx", m_kHSocket);
		return false;
	}

	if(iCount <= 0)
		return true;

	SENETSTREAMNODE *pkNetStreamNode = SeNetSreamGetHead(&m_kRecvNetStream);
	if(!pkNetStreamNode)
	{
		SeLogWrite(&m_pkSeNetEngine->GetNetScore().kLog, LT_SOCKET, true, "[ServerHandShake] get data in failed.socket=0x%llx", m_kHSocket);
		return false;
	}

	char acHeader[16] = {0};
	string strHeadr("\r\n\r\n");
	assert(sizeof(acHeader) > strHeadr.length());

	// 检查头部是否完整
	int iCopyLen = SeCopyData(acHeader, (int)strHeadr.length(), pkNetStreamNode->pkBuf + pkNetStreamNode->usWritePos - (int)strHeadr.length(), (int)strHeadr.length());
	if (string(acHeader) != strHeadr || iCopyLen != (int)strHeadr.length())
	{
		return true;
	}

	vector<string> strDestHead;
	string strCheckBuf(pkNetStreamNode->pkBuf + pkNetStreamNode->usReadPos, pkNetStreamNode->usWritePos - pkNetStreamNode->usReadPos);
	SeStrSplit(strCheckBuf, "\r\n", strDestHead);

	for(int i = 0; i < (int)strDestHead.size(); i++)
	{
		vector<string> strDest;
		SeStrSplit(strDestHead[i], ":", strDest);
		if (strDest.size() < 2)
			continue;
		m_mapHeadrs[SeStringTrim(strDest[0])] = SeStringTrim(strDest[1]);
	}

	map<string, string>::iterator itr = m_mapHeadrs.find("Upgrade");
	if (itr == m_mapHeadrs.end() || itr->second != string("websocket"))
	{
		SeLogWrite(&m_pkSeNetEngine->GetNetScore().kLog, LT_SOCKET, true, "[ServerHandShake] not websocket protocol failed.socket=0x%llx", m_kHSocket);
		return false;
	}

	itr = m_mapHeadrs.find("Connection");
	if (itr == m_mapHeadrs.end() || itr->second != string("Upgrade"))
	{
		SeLogWrite(&m_pkSeNetEngine->GetNetScore().kLog, LT_SOCKET, true, "[ServerHandShake] can not Upgrade to websocket protocol failed.socket=0x%llx", m_kHSocket);
		return false;
	}

	itr = m_mapHeadrs.find("Sec-WebSocket-Version");
	if (itr == m_mapHeadrs.end() || itr->second != SeIntToA(WEB_SOCKET_VERSION))
	{
		SeLogWrite(&m_pkSeNetEngine->GetNetScore().kLog, LT_SOCKET, true, "[ServerHandShake] websocket protocol Version error failed.socket=0x%llx", m_kHSocket);
		return false;
	}

	itr = m_mapHeadrs.find("Sec-WebSocket-Key");
	if (itr == m_mapHeadrs.end())
	{
		SeLogWrite(&m_pkSeNetEngine->GetNetScore().kLog, LT_SOCKET, true, "[ServerHandShake] not find Sec-WebSocket-Key failed.socket=0x%llx", m_kHSocket);
		return false;
	}

	string accept_str = itr->second + string("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

	// pcOut len == 41, include '\0'
	char message_digest[41] = {0};
	SeSHA1(accept_str.c_str(), accept_str.length(), message_digest, false);

	char key[41 * 3] = {0};
	int iEnCodeLen = BASE64_ENCODE_OUT_SIZE((unsigned int)strlen(message_digest));
	if (iEnCodeLen >= (int)sizeof(key))
	{
		SeLogWrite(&m_pkSeNetEngine->GetNetScore().kLog, LT_SOCKET, true, "[ServerHandShake] iEnCodeLen failed.socket=0x%llx", m_kHSocket);
		return false;
	}
	SeBase64Encode((const unsigned char*)message_digest, (unsigned int)strlen(message_digest), key);

	stringstream response;
	response << "HTTP/1.1 101 Switching Protocols\r\n"
			 << "Connection: Upgrade\r\n"
			 << "Upgrade: websocket\r\n"
			 << "Sec-WebSocket-Accept: " << key << "\r\n"
			 << "\r\n";
	m_pkSeNetEngine->SendEngineData(m_kHSocket, response.str().c_str(), (int)response.str().size());

	bHandShakeOK = true;
	pkNetStreamNode = SeNetSreamHeadPop(&m_kRecvNetStream);
	SeNetSreamTailAdd(&(m_pkSeNetEngine->GetNetScore().kSocketMgr.kNetStreamIdle), pkNetStreamNode);
	return true;
}

bool SeWSBase::PushRecvData(const char *pcBuf, int iLen)
{
	if (!IsHandShake())
	{
		struct SENETSTREAM *pkNetStreamIdle = &m_pkSeNetEngine->GetNetScore().kSocketMgr.kNetStreamIdle;

		if (!pkNetStreamIdle)
			return false;

		if (!SeNetSocketMgrUpdateNetStreamIdle(&m_pkSeNetEngine->GetNetScore().kSocketMgr, 0, iLen))
		{
			SeLogWrite(&m_pkSeNetEngine->GetNetScore().kLog, LT_SOCKET, true, "PushRecvData failed no more memcahce.socket=0x%llx", m_kHSocket);
			return false;
		}

		if (!SeNetSreamWrite(&m_kRecvNetStream, pkNetStreamIdle, NULL, 0, pcBuf, iLen))
		{
			SeLogWrite(&m_pkSeNetEngine->GetNetScore().kLog, LT_SOCKET, true, "PushRecvData SeNetSreamWrite failed.socket=0x%llx", m_kHSocket);
			return false;
		}

		return true;
	}

	if (!m_pkWSFrame)
	{
		SeLogWrite(&m_pkSeNetEngine->GetNetScore().kLog, LT_SOCKET, true, "PushRecvData m_pkWSFrame failed.socket=0x%llx", m_kHSocket);
		return false;
	}

	m_pkWSFrame->PushData(pcBuf, iLen);

	if (m_pkWSFrame->IsFrameCompelet())
	{
		m_vecFrameData.push_back(m_pkWSFrame);
		m_pkWSFrame = new SeWSFrame(m_pkSeNetEngine);
	}

	return true;
}

int SeWSBase::GetReadLen()
{
	return m_pkWSFrame->GetLeaveLen2Read();
}
