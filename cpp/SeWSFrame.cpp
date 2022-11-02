#include "SeWSBase.h"
#include "SeCommon.h"
#include "SeBase64.h"
#include "SeSha1.h"

SeWSFrame::SeWSFrame(SeNetEngine* pkSeNetEngine)
{
	m_iFin = 0;
	m_iRSV1 = 0;
	m_iRSV2 = 0;
	m_iRSV3 = 0;
	m_iOpcode = 0;
	m_iMask = 0;
	m_iPayloadLen = 0;
	m_iRealPayloadLen = 0;
	m_uiMaskingKey = 0;
	m_eState = FRAME_STATE::STATE_BASE_HEADER_ING;
	m_pkSeNetEngine = pkSeNetEngine;
	SeNetSreamInit(&m_kRecvNetStream);
}

SeWSFrame::~SeWSFrame()
{
	SESOCKETMGR *pkNetSocketMgr = &m_pkSeNetEngine->GetNetScore().kSocketMgr;
	
	SENETSTREAMNODE *pkNetStreamNode = SeNetSreamHeadPop(&m_kRecvNetStream);
	while(pkNetStreamNode)
	{
		SeNetSreamHeadAdd(&(pkNetSocketMgr->kNetStreamIdle), pkNetStreamNode);
		pkNetStreamNode = SeNetSreamHeadPop(&m_kRecvNetStream);
	}
}

bool SeWSFrame::PushData(const char *pcBuf, int iLen)
{
	if (!pcBuf || iLen <= 0)
		return false;

	struct SENETSTREAM *pkNetStreamIdle = &m_pkSeNetEngine->GetNetScore().kSocketMgr.kNetStreamIdle;

	if (!pkNetStreamIdle)
		return false;

	if (m_eState < FRAME_STATE::STATE_HEADER_COMPELET)
	{
		m_strFrame.append(pcBuf, iLen);
		SetFrameBaseHeader();
		return true;
	}
	else
	{
		if (!SeNetSocketMgrUpdateNetStreamIdle(&m_pkSeNetEngine->GetNetScore().kSocketMgr, 0, iLen))
			return false;

		if (!SeNetSreamWrite(&m_kRecvNetStream, pkNetStreamIdle, NULL, 0, pcBuf, iLen))
			return false;

		return true;
	}

	return true;
}

bool SeWSFrame::IsFrameCompelet()
{
	return m_eState == FRAME_STATE::STATE_FRAME_COMPELET;
}

void SeWSFrame::SetFrameBaseHeader()
{
	if ((int)m_strFrame.length() < MIN_FRAME_LEN)
		return;

	// 头部
	unsigned char ucFirst = m_strFrame.at(0);
	m_iFin = __GetFin(ucFirst);
	m_iRSV1 = __GetRsv1(ucFirst);
	m_iRSV2 = __GetRsv2(ucFirst);
	m_iRSV3 = __GetRsv3(ucFirst);
	m_iOpcode = __GetOpcode(ucFirst);

	// 负载以及掩码
	unsigned char ucSecond = m_strFrame.at(1);
	m_iMask = __GetMask(ucSecond);
	m_iPayloadLen = __GetPayloadLen(ucSecond);

	// 扩展长度
	int iExtendLen = 0;
	if (m_iPayloadLen == PAYLOAD_LENGTH_16_LIMIT)
		iExtendLen += MIN_FRAME_ENGTH_16_LEN;
	else if (m_iPayloadLen == PAYLOAD_LENGTH_64_LIMIT)
		iExtendLen += MIN_FRAME_ENGTH_64_LEN;
	if (m_iMask == 1)
		iExtendLen += MIN_FRAME_MASK_LEN;

	// 包长度不够
	if ((int)m_strFrame.length() < (iExtendLen + MIN_FRAME_LEN))
	{
		m_eState = FRAME_STATE::STATE_OTHER_HEADER_ING;
		return;
	}

	// 出错
	if ((int)m_strFrame.length() != (iExtendLen + MIN_FRAME_LEN))
		return;

	// 真实的负载长度
	if (!GetExtendPayloadLen(m_iRealPayloadLen))
		return;
	
	// 掩码key
	rkWsFrame.uiMaskingKey = GetMaskKey();

	printf(" iFin=%d iRSV1=%d iRSV2=%d  iRSV3=%d iOpcode=%d iMask=%d iPayloadLen=%d iRealPayloadLen=%d vector=%d uiMaskingKey=%lld \n", \
		m_iFin, m_iRSV1, m_iRSV2, m_iRSV3, m_iOpcode, m_iMask, m_iPayloadLen, m_iRealPayloadLen,(int)m_strFrame.length(), m_uiMaskingKey);

	m_eState = FRAME_STATE::STATE_HEADER_COMPELET;
}

bool SeWSFrame::GetExtendPayloadLen(int& riLen)
{
	if (m_iPayloadLen == PAYLOAD_LENGTH_16_LIMIT)
	{
		union UHEADER_EXTEND_16
		{
			unsigned char cBuf[MIN_FRAME_ENGTH_16_LEN];
			unsigned short usLen;
		};
		UHEADER_EXTEND_16 *pHeaderExtend = (UHEADER_EXTEND_16*)&m_strFrame.data()[MIN_FRAME_LEN];
		riLen = ntohs(pHeaderExtend->usLen);

		return true;
	}
	else if (m_iPayloadLen == PAYLOAD_LENGTH_64_LIMIT)
	{
		union UHEADER_EXTEND_64
		{
			unsigned char cBuf[MIN_FRAME_ENGTH_64_LEN];
			unsigned long long llLen;
		};
		UHEADER_EXTEND_64 *pHeaderExtend = (UHEADER_EXTEND_64*)&m_strFrame.data()[MIN_FRAME_LEN];
		unsigned long long llLen = ntohll(pHeaderExtend->llLen);

		if (llLen >= 1024 * 1024 * 3)
			return false;

		riLen = (int)llLen;
		return true;
	}

	riLen = m_iPayloadLen;
	return true;
}

unsigned int SeWSFrame::GetMaskKey()
{
	if (m_iMask == 0)
		return 0;

	union UHEADER_EXTEND_MASK
	{
		unsigned char cBuf[MIN_FRAME_MASK_LEN];
		unsigned int uiLen;
	};

	UHEADER_EXTEND_MASK *pHeaderExtend = (UHEADER_EXTEND_MASK*)&m_strFrame.data()[MIN_FRAME_LEN];
	if (rkWsFrame.iPayloadLen == PAYLOAD_LENGTH_16_LIMIT)
		pHeaderExtend = (UHEADER_EXTEND_MASK*)&m_strFrame.data()[MIN_FRAME_LEN + MIN_FRAME_ENGTH_16_LEN];
	else if (rkWsFrame.iPayloadLen == PAYLOAD_LENGTH_64_LIMIT)
		pHeaderExtend = (UHEADER_EXTEND_MASK*)&m_strFrame.data()[MIN_FRAME_LEN + MIN_FRAME_ENGTH_64_LEN];
	return pHeaderExtend->uiLen;
}

bool SeWSFrame::__GetFin(unsigned char ucHeader) 
{ 
	return ((unsigned int)ucHeader & BASE_FRAME_BIT::FIN) > 0; 
}

bool SeWSFrame::__GetRsv1(unsigned char ucHeader) 
{ 
	return ((unsigned int)ucHeader & BASE_FRAME_BIT::RSV1) > 0; 
}

bool SeWSFrame::__GetRsv2(unsigned char ucHeader) 
{ 
	return ((unsigned int)ucHeader & BASE_FRAME_BIT::RSV2) > 0; 
}

bool SeWSFrame::__GetRsv3(unsigned char ucHeader) 
{ 
	return ((unsigned int)ucHeader & BASE_FRAME_BIT::RSV3) > 0; 
}

unsigned char SeWSFrame::__GetOpcode(unsigned char ucHeader) 
{ 
	return (unsigned char)((unsigned int)ucHeader & BASE_FRAME_BIT::OPCODE); 
}

bool SeWSFrame::__GetMask(unsigned char ucHeader) 
{ 
	return ((unsigned int)ucHeader & BASE_FRAME_BIT::MASK) > 0; 
}

unsigned char SeWSFrame::__GetPayloadLen(unsigned char ucHeader) 
{ 
	return (unsigned char)((unsigned int)ucHeader & BASE_FRAME_BIT::PAYLOAD_LEN); 
}

