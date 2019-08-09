#include "SeFastSql.h"

SeFastSql::SeFastSql()
{

}

SeFastSql::~SeFastSql()
{

}

bool SeFastSql::ExecuteSql(SeLogEngine *pkLog, SeSql* pkSql, const char *pcQuerySql, unsigned long ulLen, const char *FuncName)
{
	bool bRet = pkSql->ExecuteSql(pcQuerySql, ulLen);
	if(!bRet)
	{
		LOGENGINE_LOG((*pkLog), LT_ERROR, "[SQL] ExecuteSql Failed. funcname=%s errormsg=%s.sql=%s", FuncName, pkSql->GetErrorStr(), pcQuerySql);
	}
	if(bRet)
	{
		return bRet;
	}

	if(!pkSql->IsConnect())
	{
		pkSql->DisConnect();
		LOGENGINE_LOG((*pkLog), LT_WARNING, "[SQL] dbsvr is disconnect, now reconnect it. funcname=%s", FuncName);

		if(!pkSql->TryConnect())
		{
			LOGENGINE_LOG((*pkLog), LT_ERROR, "[SQL] dbsvr connect error. funcname=%s errormsg=%s.sql=%s", FuncName, pkSql->GetErrorStr(), pcQuerySql);
			return false;
		}

		LOGENGINE_LOG((*pkLog), LT_WARNING, "[SQL] dbsvr is disconnect, reconnect ok. funcname=%s", FuncName);

		if(!pkSql->ExecuteSql(pcQuerySql, ulLen))
		{
			LOGENGINE_LOG((*pkLog), LT_ERROR, "[SQL] ExecuteSql Failed. funcname=%s errormsg=%s.sql=%s", FuncName, pkSql->GetErrorStr(), pcQuerySql);
			return false;
		}

		return true;
	}

	return bRet;
}

bool SeFastSql::ModifySql(SeLogEngine *pkLog, SeSql* pkSql, const char *pcQuerySql, unsigned long ulLen, const char *FuncName, unsigned long long *pkullLastInsertId, bool *pkbAffectedRows)
{
	if (!ExecuteSql(pkLog, pkSql, pcQuerySql, ulLen, FuncName))
	{
		return false;
	}

	if (pkullLastInsertId)
	{
		*pkullLastInsertId = pkSql->GetLastInsertId();
	}

	if (pkbAffectedRows)
	{
		*pkbAffectedRows = pkSql->GetAffectedRows() > 0 ? true : false;
	}

	if (pkbAffectedRows && !(*pkbAffectedRows))
	{
		LOGENGINE_LOG((*pkLog), LT_CRITICAL, "[SQLWARNNING] ModifySql. no row affected. funcname=%s sql=%s", FuncName, pcQuerySql);
	}

	return true;
}

bool SeFastSql::SelectSql(SeLogEngine *pkLog, SeSql* pkSql, const char *pcQuerySql, unsigned long ulLen, SeSqlResult* pkSeSqlResult, const char *FuncName)
{
	if (!ExecuteSql(pkLog, pkSql, pcQuerySql, ulLen, FuncName))
	{
		return false;
	}

	if(!pkSql->StoreResult(pkSeSqlResult))
	{
		LOGENGINE_LOG((*pkLog), LT_ERROR, "[SQL] SelectSql store data error errormsg=%s. funcname=%s", pkSql->GetErrorStr(), FuncName);
		return false;
	}

	return true;
}

const string SeFastSql::EscapeSql(SeLogEngine *pkLog, SeSql* pkSql, const char *pcSrc, unsigned long ulSrcLen, const char *FuncName)
{
	string kSql = pkSql->Escape(pcSrc, ulSrcLen);

	if(ulSrcLen > 0 && kSql.length() <= 0)
	{
		LOGENGINE_LOG((*pkLog), LT_ERROR, "[SQL] dbsvr EscapeSql error funcname=%s. strlen=%d", FuncName, (int)ulSrcLen);
	}

	return kSql;
}