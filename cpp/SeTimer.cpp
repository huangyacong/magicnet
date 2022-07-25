#include "SeTimer.h"
#include "SeTime.h"

SeTimer::SeTimer()
{
	m_llIdCount = 0;
}

SeTimer::~SeTimer()
{

}

int SeTimer::GetTimerCount()
{
	return (int)m_kTimerIdList.size();
}

long long SeTimer::GetTimeOutId(unsigned long long llNowTime)
{
	if (m_kTimerMgr.empty())
	{
		return 0;
	}

	std::map<unsigned long long, std::list<long long> >::iterator itr = m_kTimerMgr.begin();
	if (itr->first > llNowTime)
	{
		return 0;
	}

	if (itr->second.empty())
	{
		return 0;
	}

	long long llTimerId = *itr->second.begin();
	itr->second.pop_front();

	if (itr->second.empty())
	{
		m_kTimerMgr.erase(itr);
	}

	m_kTimerIdList.erase(llTimerId);
	return llTimerId;
}

long long SeTimer::SetTimer(unsigned long long ullDelayTtimeMillSec, long long ullTImeOffset)
{
	m_llIdCount++;
	long long llTimerId = m_llIdCount == 0 ? (m_llIdCount + 1) : m_llIdCount;
	
	unsigned long long ullTime = SeTimeGetTickCount() + ullDelayTtimeMillSec + ullTImeOffset;
	if (m_kTimerMgr.find(ullTime) == m_kTimerMgr.end())
	{
		m_kTimerMgr[ullTime] = std::list<long long>();
	}
	m_kTimerIdList[llTimerId] = ullTime;
	m_kTimerMgr[ullTime].push_back(llTimerId);
	return llTimerId;
}

void SeTimer::DelTimer(long long llTimerId)
{
	if (m_kTimerIdList.find(llTimerId) == m_kTimerIdList.end())
	{
		return;
	}

	unsigned long long ullTime = m_kTimerIdList[llTimerId];
	m_kTimerIdList.erase(llTimerId);

	std::map<unsigned long long, std::list<long long> >::iterator itr = m_kTimerMgr.find(ullTime);
	if (itr == m_kTimerMgr.end())
	{
		return;
	}

	for (std::list<long long>::iterator itrID = itr->second.begin(); itrID != itr->second.end(); itrID++)
	{
		if (*itrID == llTimerId)
		{
			itr->second.erase(itrID);
			break;
		}
	}

	if (itr->second.empty())
	{
		m_kTimerMgr.erase(itr);
	}
}



