#include "SeLogEngine.h"
#include <stdio.h>
#include <stdarg.h>

SeLogEngine::SeLogEngine()
{
	m_bInit = false;
}

SeLogEngine::SeLogEngine(const char *pkFileName, int iLogLv, SELOGCONTEXT pkLogContxtFunc, void *pkLogContect)
{
	m_bInit = true;
	SeInitLog(&m_kLog, pkFileName);
	SeAddLogLV(&m_kLog, iLogLv);
	SeLogSetLogContextFunc(&m_kLog, pkLogContxtFunc, pkLogContect);
}

void SeLogEngine::SetFileName(const char *pkFileName)
{
	if(!m_bInit)
	{
		m_bInit = true;
		SeInitLog(&m_kLog, pkFileName);
	}
}

void SeLogEngine::SetLogSetLogContextFunc(SELOGCONTEXT pkLogContxtFunc, void *pkLogContect)
{
	if(m_bInit)
	{
		SeLogSetLogContextFunc(&m_kLog, pkLogContxtFunc, pkLogContect);
	}
}

SeLogEngine::~SeLogEngine()
{
	if(m_bInit)
	{
		SeFinLog(&m_kLog);
	}
}

void SeLogEngine::AddLogLV(int iLogLv)
{
	if(m_bInit)
	{
		SeAddLogLV(&m_kLog, iLogLv);
	}
}

SELOG *SeLogEngine::GetLogIntance()
{
	if(!m_bInit) 
	{ 
		return 0; 
	}

	return &m_kLog;
}

