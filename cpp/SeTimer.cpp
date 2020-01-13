#include "SeTimer.h"
#include "SeTime.h"

SeTimer::SeTimer()
{
	m_llIdCount = 0;
}

SeTimer::~SeTimer()
{

}

long long SeTimer::GetTimeOutId(unsigned long long llNowTime)
{
	if (m_kTimerMgr.empty())
	{
		return 0;
	}

	std::map<unsigned long long, std::list<long long> >::iterator itr = m_kTimerMgr.begin();
	if (itr->first < llNowTime)
	{
		return 0;
	}

	long long llTimerId = *itr->second.begin();
	itr->second.pop_front();

	if (itr->second.empty())
	{
		m_kTimerMgr.erase(itr);
	}

	return llTimerId;
}

long long SeTimer::SetTimer(int iDelayTtimeMillSec)
{
	m_llIdCount++;
	m_llIdCount = m_llIdCount == 0 ? (m_llIdCount + 1) : m_llIdCount;
	
	unsigned long long ullTime = SeTimeGetTickCount() + iDelayTtimeMillSec*1000;
	if (m_kTimerMgr.find(ullTime) == m_kTimerMgr.end())
	{
		m_kTimerMgr[ullTime] = std::list<long long>();
	}
	m_kTimerMgr[ullTime].push_back(m_llIdCount);
	return m_llIdCount;
}


