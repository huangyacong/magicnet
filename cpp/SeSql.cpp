#include "SeSql.h"
#include <stdlib.h>

const int MYSQL_ESCAPE_MAX_LEN = 1024 * 1024 * 4;

bool SeInitMysqlLibrary()
{
	return SeMysqlLibraryInit();
}

void SeEndMysqlLibrary()
{
	SeMysqlLibraryEnd();
}

SeSqlResult::SeSqlResult()
{
	SeMysqlResultInit(&m_kMySqlResult);
}

SeSqlResult::~SeSqlResult()
{
	SeMysqlResultFin(&m_kMySqlResult);
}

bool SeSqlResult::NextRecord()
{
	return SeMysqlResultNextRecord(&m_kMySqlResult);
}

const char* SeSqlResult::GetFieldValue(unsigned int iIndex)
{
	return SeMysqlResultGetFieldValue(&m_kMySqlResult, iIndex);
}

int SeSqlResult::GetFieldValueToInt(unsigned int iIndex)
{
	return SeAToInt(GetFieldValue(iIndex));
}

long long SeSqlResult::GetFieldValueToLongLong(unsigned int iIndex)
{
	return SeAToLongLong(GetFieldValue(iIndex));
}

bool SeSqlResult::IsFieldValueNumType(unsigned int iIndex)
{
	return SeMysqlResultIsFieldValueNumType(&m_kMySqlResult, iIndex);
}

const char* SeSqlResult::GetFieldName(unsigned int iIndex)
{
	return SeMysqlResultGetFieldName(&m_kMySqlResult, iIndex);
}

unsigned long SeSqlResult::GetFieldLen(unsigned int iIndex)
{
	return SeMysqlResultGetFieldLen(&m_kMySqlResult, iIndex);
}

unsigned int SeSqlResult::GetFieldCount()
{
	return SeMysqlResultGetFieldCount(&m_kMySqlResult);
}

SeSql::SeSql(const char* pcHost, unsigned int uiPort, const char* pcDBName, const char* pcUser, const char* pcPasswd, const char* pcCharacterSet)
{
	m_pcEscape = (char*)SeMallocMem(MYSQL_ESCAPE_MAX_LEN);
	SeStrNcpy(m_acCharacterSet, sizeof(m_acCharacterSet), pcCharacterSet);
	SeMysqlInit(&m_kMySql, pcHost, uiPort, pcDBName, pcUser, pcPasswd);
}

SeSql::~SeSql()
{
	SeFreeMem(m_pcEscape);
	SeMysqlFin(&m_kMySql);
}

bool SeSql::TryConnect()
{
	bool ret = SeMysqlTryConnect(&m_kMySql);
	if(ret)
	{
		SetAutoCommit(true);
		SeSetCharacterSet(&m_kMySql, (const char *)m_acCharacterSet);
	}
	return ret;
}

bool SeSql::ExecuteSql(const char *pcQuerySql, unsigned long ulLen)
{
	return SeMysqlExecuteSql(&m_kMySql, pcQuerySql, ulLen);
}

bool SeSql::StoreResult(SeSqlResult *pkMysqlResult)
{
	bool ret = SeMysqlStoreResult(&m_kMySql, &pkMysqlResult->m_kMySqlResult);
	if(ret) 
	{ 
		while (NextResult()) 
		{
		} 
	}
	return ret;
}

unsigned long long SeSql::GetLastInsertId()
{
	return SeMysqlInsertId(&m_kMySql);
}

unsigned long long SeSql::GetAffectedRows()
{
	return SeMysqlAffectedRows(&m_kMySql);
}

bool SeSql::NextResult()
{
	return SeMysqlNextResult(&m_kMySql);
}

const string SeSql::Escape(const char *pcSrc, unsigned long ulSrcLen)
{
	if(ulSrcLen <= 0)
	{
		return string("");
	}

	if((ulSrcLen * 2 + 1) >= (unsigned long)MYSQL_ESCAPE_MAX_LEN)
	{
		return string("");
	}

	unsigned long ulRet = SeMysqlEscape(&m_kMySql, m_pcEscape, pcSrc, ulSrcLen);
	return ulRet >= ulSrcLen ? string(m_pcEscape, ulRet) : string("");
}

bool SeSql::SetAutoCommit(bool bAutoCommit)
{
	return SeMysqlSetAutoCommit(&m_kMySql, bAutoCommit);
}

bool SeSql::Commit()
{
	return SeMysqlCommit(&m_kMySql);
}

bool SeSql::Rollback()
{
	return SeMysqlRollback(&m_kMySql);
}

unsigned int SeSql::GetErrorCode()
{
	return SeMysqlGetErrorCode(&m_kMySql);
}

const char* SeSql::GetErrorStr()
{
	return SeMysqlGetErrorStr(&m_kMySql);
}

bool SeSql::IsConnect()
{
	return SeMysqlIsConnect(&m_kMySql);
}

void SeSql::DisConnect()
{
	SeMysqlDisConnect(&m_kMySql);
}
