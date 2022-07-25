#ifndef __SE_TIMER_H__
#define __SE_TIMER_H__

#include <map>
#include <list>

class SeTimer
{
public:
	SeTimer();
	~SeTimer();
public:
	int GetTimerCount();
	long long GetTimeOutId(unsigned long long llNowTime);
	long long SetTimer(unsigned long long ullDelayTtimeMillSec, long long ullTImeOffset);
	void DelTimer(long long llTimerId);
private:
	long long m_llIdCount;
	std::map<long long, unsigned long long> m_kTimerIdList;
	std::map<unsigned long long, std::list<long long> > m_kTimerMgr;
};

#endif


