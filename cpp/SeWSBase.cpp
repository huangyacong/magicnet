#include "SeWSBase.h"

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

void SeWSBase::PushRecvData(const char *pcBuf, int iLen)
{
	
}
