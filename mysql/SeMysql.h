#ifndef __SE_MYSQL_H__
#define __SE_MYSQL_H__

#include "SeTool.h"
#include "SeBool.h"

#if defined(__linux)

#include <sys/socket.h>
#include <mysql/mysql.h>

#elif (defined(_WIN32) || defined(WIN32))

#include <winsock2.h>
#include "mysql.h"

#endif

struct SEMYSQLRESULT
{
	MYSQL_RES*		pkRes;
	MYSQL_ROW		kRow;
};

struct SEMYSQL
{
	MYSQL			kMysql;
	
	unsigned int	uiPort;
	int				iAutoReconnect;
	char			acHost[256];
	char			acUser[256];
	char			acPasswd[256];
	char			acDBName[256];
};

void SeMysqlInit(struct SEMYSQL *pkMysql, const char* pcHost, unsigned int uiPort, const char* pcDBName, const char* pcUser, const char* pcPasswd, int iAutoReconnect);

void SeMysqlFin(struct SEMYSQL *pkMysql);


bool SeMysqlTryConnect(struct SEMYSQL *pkMysql);

bool SeMysqlIsConnect(struct SEMYSQL *pkMysql);

void SeMysqlDisConnect(struct SEMYSQL *pkMysql);


unsigned int SeMysqlGetErrorCode(struct SEMYSQL *pkMysql);

const char* SeMysqlGetErrorStr(struct SEMYSQL *pkMysql);


bool SeMysqlSetAutoCommit(struct SEMYSQL *pkMysql, bool bAutoCommit);

bool SeMysqlCommit(struct SEMYSQL *pkMysql);

bool SeMysqlRollback(struct SEMYSQL *pkMysql);


// pcDst len must at least (ulSrcLen*2 + 1) and return value = copy len not include '\0'
unsigned long SeMysqlEscape(struct SEMYSQL *pkMysql, char *pcDst, const char *pcSrc, unsigned long ulSrcLen);

// pcDst len must at least (ulSrcLen*2 + 1) and return value = copy len not include '\0'
unsigned long SeMysqlMyEscape(char *pcDst, const char *pcSrc, unsigned long ulSrcLen);

long SeMysqlExecuteSql(struct SEMYSQL *pkMysql, const char *pcQuerySql, unsigned long ulLen);

bool SeMysqlNextResult(struct SEMYSQL *pkMysql);

bool SeMysqlStoreResult(struct SEMYSQL *pkMysql, struct SEMYSQLRESULT *pkMysqlResult);




void SeMysqlResultInit(struct SEMYSQLRESULT *pkMysqlResult);

void SeMysqlResultFin(struct SEMYSQLRESULT *pkMysqlResult);

bool SeMysqlResultNextRecord(struct SEMYSQLRESULT *pkMysqlResult);

unsigned int SeMysqlResultGetFieldCount(struct SEMYSQLRESULT *pkMysqlResult);

const char* SeMysqlResultGetFieldName(struct SEMYSQLRESULT *pkMysqlResult, unsigned int iIndex);

bool SeMysqlResultIsFieldValueNumType(struct SEMYSQLRESULT *pkMysqlResult, unsigned int iIndex);

unsigned long SeMysqlResultGetFieldLen(struct SEMYSQLRESULT *pkMysqlResult, unsigned int iIndex);//for binary data

const char* SeMysqlResultGetFieldValue(struct SEMYSQLRESULT *pkMysqlResult, unsigned int iIndex);

#endif



