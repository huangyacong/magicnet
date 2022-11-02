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
	SeNetSreamInit(&m_kPacketNetStream);
	m_strIP = strIP;
}

SeWSBase::~SeWSBase()
{
	SESOCKETMGR *pkNetSocketMgr = &m_pkSeNetEngine->GetNetScore().kSocketMgr;
	
	SENETSTREAMNODE *pkNetStreamNode = SeNetSreamHeadPop(&m_kRecvNetStream);
	while(pkNetStreamNode)
	{
		SeNetSreamNodeZero(pkNetStreamNode);
		SeNetSreamHeadAdd(&(pkNetSocketMgr->kNetStreamIdle), pkNetStreamNode);
		pkNetStreamNode = SeNetSreamHeadPop(&m_kRecvNetStream);
	}

	pkNetStreamNode = SeNetSreamHeadPop(&m_kPacketNetStream);
	while(pkNetStreamNode)
	{
		SeNetSreamNodeZero(pkNetStreamNode);
		SeNetSreamHeadAdd(&(pkNetSocketMgr->kNetStreamIdle), pkNetStreamNode);
		pkNetStreamNode = SeNetSreamHeadPop(&m_kPacketNetStream);
	}
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
	SeNetSreamNodeZero(pkNetStreamNode);
	SeNetSreamTailAdd(&(m_pkSeNetEngine->GetNetScore().kSocketMgr.kNetStreamIdle), pkNetStreamNode);
	return true;
}

bool SeWSBase::PushRecvData(const char *pcBuf, int iLen)
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

void SeWSBase::PushFrameHeader(const char *pcBuf, int iLen)
{
	for(int i = 0; i < iLen; i++)
	{
		m_vecFrame.push_back((unsigned char)pcBuf[i]);
	}
}

bool SeWSBase::IsFrameHeaderCompelet(WSFrame &rkWsFrame, int& riNeedLen)
{
	if ((int)m_vecFrame.size() < MIN_FRAME_LEN)
		return false;

	// 头部
	unsigned char ucFirst = m_vecFrame[0];
	rkWsFrame.iFin = __GetFin(ucFirst);
	rkWsFrame.iRSV1 = __GetRsv1(ucFirst);
	rkWsFrame.iRSV2 = __GetRsv2(ucFirst);
	rkWsFrame.iRSV3 = __GetRsv3(ucFirst);
	rkWsFrame.iOpcode = __GetOpcode(ucFirst);

	// 负载以及掩码
	unsigned char ucSecond = m_vecFrame[1];
	rkWsFrame.iMask = __GetMask(ucSecond);
	rkWsFrame.iPayloadLen = __GetPayloadLen(ucSecond);

	// 扩展长度
	int iExtendLen = 0;
	if (rkWsFrame.iPayloadLen == PAYLOAD_LENGTH_16_LIMIT)
		iExtendLen += MIN_FRAME_ENGTH_16_LEN;
	else if (rkWsFrame.iPayloadLen == PAYLOAD_LENGTH_64_LIMIT)
		iExtendLen += MIN_FRAME_ENGTH_64_LEN;
	if (rkWsFrame.iMask == 1)
		iExtendLen += MIN_FRAME_MASK_LEN;

	// 包长度不够
	if ((int)m_vecFrame.size() < (iExtendLen + MIN_FRAME_LEN))
	{
		riNeedLen = iExtendLen + MIN_FRAME_LEN - (int)m_vecFrame.size();
		return false;
	}
	if ((int)m_vecFrame.size() != (iExtendLen + MIN_FRAME_LEN))
		return false;

	// 真实的负载长度
	rkWsFrame.iRealPayloadLen = rkWsFrame.iPayloadLen;

	if (rkWsFrame.iPayloadLen == PAYLOAD_LENGTH_16_LIMIT)
	{
		union UHEADER_EXTEND_16
		{
			unsigned char cBuf[MIN_FRAME_ENGTH_16_LEN];
			unsigned short usLen;
		};
		UHEADER_EXTEND_16 *pHeaderExtend = (UHEADER_EXTEND_16*)&m_vecFrame[MIN_FRAME_LEN];
		rkWsFrame.iRealPayloadLen = ntohs(pHeaderExtend->usLen);
	}
	else if (rkWsFrame.iPayloadLen == PAYLOAD_LENGTH_64_LIMIT)
	{
		union UHEADER_EXTEND_64
		{
			unsigned char cBuf[MIN_FRAME_ENGTH_64_LEN];
			unsigned long long llLen;
		};
		UHEADER_EXTEND_64 *pHeaderExtend = (UHEADER_EXTEND_64*)&m_vecFrame[MIN_FRAME_LEN];
		unsigned long long llLen = ntohll(pHeaderExtend->llLen);
		if (llLen >= 1024 * 1024 * 3)
			return false;
		rkWsFrame.iRealPayloadLen = (int)llLen;
	}

	// 掩码key
	rkWsFrame.uiMaskingKey = 0;

	if (rkWsFrame.iMask == 1)
	{
		union UHEADER_EXTEND_MASK
		{
			unsigned char cBuf[MIN_FRAME_MASK_LEN];
			unsigned int uiLen;
		};

		UHEADER_EXTEND_MASK *pHeaderExtend = (UHEADER_EXTEND_MASK*)&m_vecFrame[MIN_FRAME_LEN];
		if (rkWsFrame.iPayloadLen == PAYLOAD_LENGTH_16_LIMIT)
			pHeaderExtend = (UHEADER_EXTEND_MASK*)&m_vecFrame[MIN_FRAME_LEN + MIN_FRAME_ENGTH_16_LEN];
		else if (rkWsFrame.iPayloadLen == PAYLOAD_LENGTH_64_LIMIT)
			pHeaderExtend = (UHEADER_EXTEND_MASK*)&m_vecFrame[MIN_FRAME_LEN + MIN_FRAME_ENGTH_64_LEN];
		rkWsFrame.uiMaskingKey = pHeaderExtend->uiLen;
	}

	printf(" iFin=%d iRSV1=%d iRSV2=%d  iRSV3=%d iOpcode=%d iMask=%d iPayloadLen=%d iRealPayloadLen=%d vector=%d uiMaskingKey=%lld \n", rkWsFrame.iFin, rkWsFrame.iRSV1, rkWsFrame.iRSV2, rkWsFrame.iRSV3, \
			rkWsFrame.iOpcode, rkWsFrame.iMask, rkWsFrame.iPayloadLen, rkWsFrame.iRealPayloadLen,(int)m_vecFrame.size(), rkWsFrame.uiMaskingKey);

	return true;
}