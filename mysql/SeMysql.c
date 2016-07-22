#include "SeMysql.h"
#include <assert.h>

bool TryConnect(struct SEMYSQL *pkMysql)
{
	if(!mysql_init(&pkMysql->kMysql))
	{
		return false;
	}
	return (mysql_real_connect(&pkMysql->kMysql,pkMysql->acHost, pkMysql->acUser, pkMysql->acPasswd, pkMysql->acDBName, pkMysql->uiPort, 
		NULL,CLIENT_COMPRESS) ? true : false);
}

bool SeMysqlLibraryInit()
{
	return mysql_library_init(0, NULL, NULL) == 0 ? true : false;
}

void SeMysqlLibraryEnd()
{
	mysql_library_end();
}

void SeMysqlInit(struct SEMYSQL *pkMysql, const char* pcHost, unsigned int uiPort, const char* pcDBName, const char* pcUser, const char* pcPasswd)
{
	pkMysql->uiPort = uiPort;
	SeStrNcpy(pkMysql->acHost, 256, pcHost);
	SeStrNcpy(pkMysql->acUser, 256, pcUser);
	SeStrNcpy(pkMysql->acPasswd, 256, pcPasswd);
	SeStrNcpy(pkMysql->acDBName, 256, pcDBName);
}

void SeMysqlFin(struct SEMYSQL *pkMysql)
{
}

bool SeMysqlTryConnect(struct SEMYSQL *pkMysql)
{
	return TryConnect(pkMysql);
}

bool SeMysqlIsConnect(struct SEMYSQL *pkMysql)
{
	return mysql_ping(&pkMysql->kMysql) == 0;
}

void SeMysqlDisConnect(struct SEMYSQL *pkMysql)
{
	mysql_close(&pkMysql->kMysql);
}

unsigned int SeMysqlGetErrorCode(struct SEMYSQL *pkMysql)
{
	return mysql_errno(&pkMysql->kMysql);
}

const char* SeMysqlGetErrorStr(struct SEMYSQL *pkMysql)
{
	return mysql_error(&pkMysql->kMysql);
}

bool SeMysqlSetAutoCommit(struct SEMYSQL *pkMysql, bool bAutoCommit)
{
	return mysql_autocommit(&pkMysql->kMysql, bAutoCommit) == 0;
}

bool SeMysqlCommit(struct SEMYSQL *pkMysql)
{
	return mysql_commit(&pkMysql->kMysql) == 0;
}

bool SeMysqlRollback(struct SEMYSQL *pkMysql)
{
	return mysql_rollback(&pkMysql->kMysql) == 0;
}

unsigned long SeMysqlEscape(struct SEMYSQL *pkMysql, char *pcDst, const char *pcSrc, unsigned long ulSrcLen)
{
	return mysql_real_escape_string(&pkMysql->kMysql, pcDst, pcSrc, ulSrcLen);
}

unsigned long SeMysqlMyEscape(char *pcDst, const char *pcSrc, unsigned long ulSrcLen)
{
	unsigned long ulPos;
	unsigned long ulIndex;

	for(ulIndex = 0, ulPos = 0; ulIndex < ulSrcLen; ulIndex++)
	{
		if((char)pcSrc[ulIndex] == '\0')
		{
			pcDst[ulPos] = '\\';
			ulPos++;
			pcDst[ulPos] = '0';
			ulPos++;
		}
		else if((char)pcSrc[ulIndex] == '\'')
		{
			pcDst[ulPos] = '\\';
			ulPos++;
			pcDst[ulPos] = '\'';
			ulPos++;
		}
		else if((char)pcSrc[ulIndex] == '"')
		{
			pcDst[ulPos] = '\\';
			ulPos++;
			pcDst[ulPos] = '"';
			ulPos++;
		}
		else if((char)pcSrc[ulIndex] == '\b')
		{
			pcDst[ulPos] = '\\';
			ulPos++;
			pcDst[ulPos] = 'b';
			ulPos++;
		}
		else if((char)pcSrc[ulIndex] == '\n')
		{
			pcDst[ulPos] = '\\';
			ulPos++;
			pcDst[ulPos] = 'n';
			ulPos++;
		}
		else if((char)pcSrc[ulIndex] == '\r')
		{
			pcDst[ulPos] = '\\';
			ulPos++;
			pcDst[ulPos] = 'r';
			ulPos++;
		}
		else if((char)pcSrc[ulIndex] == '\t')
		{
			pcDst[ulPos] = '\\';
			ulPos++;
			pcDst[ulPos] = 't';
			ulPos++;
		}
		else if((char)pcSrc[ulIndex] == '\\')
		{
			pcDst[ulPos] = '\\';
			ulPos++;
			pcDst[ulPos] = '\\';
			ulPos++;
		}
		else if((char)pcSrc[ulIndex] == 26)
		{
			pcDst[ulPos] = '\\';
			ulPos++;
			pcDst[ulPos] = 'Z';
			ulPos++;
		}
		else
		{
			pcDst[ulPos] = pcSrc[ulIndex];
			ulPos++;
		}
	}
	pcDst[ulPos] = '\0';
	return ulPos;
}

bool SeMysqlExecuteSql(struct SEMYSQL *pkMysql, const char *pcQuerySql, unsigned long ulLen)
{
	return mysql_real_query(&pkMysql->kMysql, pcQuerySql, ulLen) == 0 ? true : false;
}

bool SeMysqlNextResult(struct SEMYSQL *pkMysql)
{
	return mysql_next_result(&pkMysql->kMysql) == 0;
}

bool SeMysqlStoreResult(struct SEMYSQL *pkMysql, struct SEMYSQLRESULT *pkMysqlResult)
{
	pkMysqlResult->pkRes = mysql_store_result(&pkMysql->kMysql);
	return pkMysqlResult->pkRes ? true : false;
}

unsigned long long SeMysqlInsertId(struct SEMYSQL *pkMysql)
{
	return mysql_insert_id(&pkMysql->kMysql);
}

void SeMysqlResultInit(struct SEMYSQLRESULT *pkMysqlResult)
{
	pkMysqlResult->pkRes = 0;
}

void SeMysqlResultFin(struct SEMYSQLRESULT *pkMysqlResult)
{
	if(pkMysqlResult->pkRes) {
		mysql_free_result(pkMysqlResult->pkRes);
	}
	pkMysqlResult->pkRes = 0;
}

bool SeMysqlResultNextRecord(struct SEMYSQLRESULT *pkMysqlResult)
{
	pkMysqlResult->kRow = mysql_fetch_row(pkMysqlResult->pkRes);
	return pkMysqlResult->kRow ? true : false;
}

unsigned int SeMysqlResultGetFieldCount(struct SEMYSQLRESULT *pkMysqlResult)
{
	assert(pkMysqlResult->pkRes);
	return mysql_num_fields(pkMysqlResult->pkRes);
}

const char* SeMysqlResultGetFieldName(struct SEMYSQLRESULT *pkMysqlResult, unsigned int iIndex)
{
	assert(pkMysqlResult->pkRes);
	assert(iIndex < SeMysqlResultGetFieldCount(pkMysqlResult));
	MYSQL_FIELD* pkField = mysql_fetch_fields(pkMysqlResult->pkRes);
	return pkField[iIndex].name;
}

bool SeMysqlResultIsFieldValueNumType(struct SEMYSQLRESULT *pkMysqlResult, unsigned int iIndex)
{
	assert(pkMysqlResult->pkRes);
	assert(iIndex < SeMysqlResultGetFieldCount(pkMysqlResult));
	MYSQL_FIELD* pkField = mysql_fetch_fields(pkMysqlResult->pkRes);
	return IS_NUM(pkField[iIndex].type) ? true : false;
}

unsigned long SeMysqlResultGetFieldLen(struct SEMYSQLRESULT *pkMysqlResult, unsigned int iIndex)
{
	assert(pkMysqlResult->pkRes);
	assert(iIndex < SeMysqlResultGetFieldCount(pkMysqlResult));
	unsigned long *pkLengths = mysql_fetch_lengths(pkMysqlResult->pkRes);
	return pkLengths[iIndex];
}

const char* SeMysqlResultGetFieldValue(struct SEMYSQLRESULT *pkMysqlResult, unsigned int iIndex)
{
	assert(pkMysqlResult->pkRes);
	assert(iIndex < SeMysqlResultGetFieldCount(pkMysqlResult));
	return pkMysqlResult->kRow[iIndex];
}
