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
	long long GetTimeOutId(unsigned long long llNowTime);
	long long SetTimer(int iDelayTtimeMillSec);
private:
	long long m_llIdCount;
	std::map<unsigned long long, std::list<long long> > m_kTimerMgr;
};

#endif


