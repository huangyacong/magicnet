#ifndef __SE_FAST_SQL_H__
#define __SE_FAST_SQL_H__

#include "SeSql.h"
#include "SeLogEngine.h"

class SeFastSql
{
public:
	SeFastSql();
	virtual ~SeFastSql();
public:
	// ��ʹ�� SeTool.h �е� SeSnprintf ������SQL���
	static bool ModifySql(SeLogEngine *pkLog, SeSql* pkSql, const char *pcQuerySql, unsigned long ulLen, const char *FuncName, unsigned long long *pkullLastInsertId = NULL, bool *pkbAffectedRows = NULL);
	// ��ʹ�� SeTool.h �е� SeSnprintf ������SQL���
	static bool SelectSql(SeLogEngine *pkLog, SeSql* pkSql, const char *pcQuerySql, unsigned long ulLen, SeSqlResult* pkSeSqlResult, const char *FuncName);
public:
	static const string EscapeSql(SeLogEngine *pkLog, SeSql* pkSql, const char *pcSrc, unsigned long ulSrcLen, const char *FuncName);
protected:
	static bool ExecuteSql(SeLogEngine *pkLog, SeSql* pkSql, const char *pcQuerySql, unsigned long ulLen, const char *FuncName);
};

#endif