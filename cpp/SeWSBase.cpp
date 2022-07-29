#include "SeWSBase.h"
#include "SeCommon.h"

SeWSBase::SeWSBase(SeNetEngine* pkSeNetEngine, HSOCKET kHSocket)
{
	m_kHSocket = kHSocket;
	m_bHandShake = false;
	m_pkSeNetEngine = pkSeNetEngine;
	SeNetSreamInit(&m_kRecvNetStream);
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
}

bool SeWSBase::PushRecvData(const char *pcBuf, int iLen)
{
	if(!SeNetSocketMgrUpdateNetStreamIdle(&m_pkSeNetEngine->GetNetScore().kSocketMgr, 0, iLen))
	{
		SeLogWrite(&m_pkSeNetEngine->GetNetScore().kLog, LT_SOCKET, true, "[PushRecvData] new idle memcahce.socket=0x%llx", m_kHSocket);
		return false;
	}

	int iWritePos = 0;

	while(iWritePos < iLen)
	{
		SENETSTREAMNODE *pkNetStreamNode = SeNetSreamTailPop(&m_kRecvNetStream);
		if(pkNetStreamNode)
		{
			if(pkNetStreamNode->usMaxLen <= pkNetStreamNode->usWritePos)
			{
				SeNetSreamTailAdd(&m_kRecvNetStream, pkNetStreamNode);
				pkNetStreamNode = 0;
			}
		}

		if(!pkNetStreamNode)
		{
			pkNetStreamNode = SeNetSreamHeadPop(&(m_pkSeNetEngine->GetNetScore().kSocketMgr.kNetStreamIdle));
			if(pkNetStreamNode)
			{
				SeNetSreamNodeZero(pkNetStreamNode);
			}
		}

		if(!pkNetStreamNode)
		{
			SeLogWrite(&m_pkSeNetEngine->GetNetScore().kLog, LT_SOCKET, true, "[PushRecvData] no more ilde memcahce.socket=0x%llx", m_kHSocket);
			return false;
		}

		int iCopyLen = SeCopyData(pkNetStreamNode->pkBuf + pkNetStreamNode->usWritePos, pkNetStreamNode->usMaxLen - pkNetStreamNode->usWritePos, pcBuf + iWritePos, iLen - iWritePos);
		pkNetStreamNode->usWritePos += iCopyLen;
		iWritePos += iCopyLen;

		SeNetSreamTailAdd(&m_kRecvNetStream, pkNetStreamNode);

		if (iCopyLen <= 0)
		{
			SeLogWrite(&m_pkSeNetEngine->GetNetScore().kLog, LT_SOCKET, true, "[PushRecvData] copy data error.socket=0x%llx", m_kHSocket);
			return false;
		}
	}

	return true;
}
