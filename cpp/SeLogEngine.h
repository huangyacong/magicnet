#ifndef __SE_LOGENGINE_H__
#define __SE_LOGENGINE_H__

#include "SeLog.h"
#include "SeLock.h"

class SeLogEngine
{
public:
	SeLogEngine();
	SeLogEngine(const char *pkFileName, int iLogLv, SELOGCONTEXT pkLogContxtFunc, void *pkLogContect);
	virtual ~SeLogEngine();
public:
	void SetFileName(const char *pkFileName);
	void AddLogLV(int iLogLv);
	void SetLogSetLogContextFunc(SELOGCONTEXT pkLogContxtFunc, void *pkLogContect);
	SELOG *GetLogIntance();
private:
	bool m_bInit;
	SELOG m_kLog;
};

#define LOGENGINE_LOG(Log, lv, fmt, ...) SeLogWrite(Log.GetLogIntance(), lv, true, fmt, ##__VA_ARGS__)

#define LOGENGINE_CACHE_LOG(Log, lv, fmt, ...) SeLogWrite(Log.GetLogIntance(), lv, false, fmt, ##__VA_ARGS__)

#define LOGENGINE_FLUSH_CACHE_LOG(Log) SeLogFlushToDisk(Log.GetLogIntance())

#endif
