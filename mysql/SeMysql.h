#ifndef __SE_MYSQL_H__
#define __SE_MYSQL_H__

#ifdef	__cplusplus
extern "C" {
#endif

#include "SeTool.h"
#include <stdbool.h>

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
	char			acHost[256];
	char			acUser[256];
	char			acPasswd[256];
	char			acDBName[256];
};

/*
[client]
default-character-set=utf8mb4

[mysql]
default-character-set=utf8mb4

[mysqld]
max_allowed_packet = 4M
character-set-server=utf8mb4
default-storage-engine=INNODB
collation-server = utf8mb4_unicode_ci
init_connect='SET NAMES utf8mb4'

SHOW VARIABLES WHERE Variable_name LIKE 'character\_set\_%' OR Variable_name LIKE 'collation%';
*/

bool SeMysqlLibraryInit();

void SeMysqlLibraryEnd();

void SeMysqlInit(struct SEMYSQL *pkMysql, const char* pcHost, unsigned int uiPort, const char* pcDBName, const char* pcUser, const char* pcPasswd);

void SeMysqlFin(struct SEMYSQL *pkMysql);


bool SeMysqlTryConnect(struct SEMYSQL *pkMysql);

bool SeMysqlIsConnect(struct SEMYSQL *pkMysql);

void SeMysqlDisConnect(struct SEMYSQL *pkMysql);


void SeSetCharacterSet(struct SEMYSQL *pkMysql, const char *pcCharacterSet);


unsigned int SeMysqlGetErrorCode(struct SEMYSQL *pkMysql);

const char* SeMysqlGetErrorStr(struct SEMYSQL *pkMysql);


bool SeMysqlSetAutoCommit(struct SEMYSQL *pkMysql, bool bAutoCommit);

bool SeMysqlCommit(struct SEMYSQL *pkMysql);

bool SeMysqlRollback(struct SEMYSQL *pkMysql);


// pcDst len must at least (ulSrcLen*2 + 1) and return value = copy len not include '\0'
unsigned long SeMysqlEscape(struct SEMYSQL *pkMysql, char *pcDst, const char *pcSrc, unsigned long ulSrcLen);

// pcDst len must at least (ulSrcLen*2 + 1) and return value = copy len not include '\0'
unsigned long SeMysqlMyEscape(char *pcDst, const char *pcSrc, unsigned long ulSrcLen);

bool SeMysqlExecuteSql(struct SEMYSQL *pkMysql, const char *pcQuerySql, unsigned long ulLen);

bool SeMysqlNextResult(struct SEMYSQL *pkMysql);

bool SeMysqlStoreResult(struct SEMYSQL *pkMysql, struct SEMYSQLRESULT *pkMysqlResult);

unsigned long long SeMysqlInsertId(struct SEMYSQL *pkMysql);

unsigned long long SeMysqlAffectedRows(struct SEMYSQL *pkMysql);



void SeMysqlResultInit(struct SEMYSQLRESULT *pkMysqlResult);

void SeMysqlResultFin(struct SEMYSQLRESULT *pkMysqlResult);

bool SeMysqlResultNextRecord(struct SEMYSQLRESULT *pkMysqlResult);

unsigned int SeMysqlResultGetFieldCount(struct SEMYSQLRESULT *pkMysqlResult);

const char* SeMysqlResultGetFieldName(struct SEMYSQLRESULT *pkMysqlResult, unsigned int iIndex);

bool SeMysqlResultIsFieldValueNumType(struct SEMYSQLRESULT *pkMysqlResult, unsigned int iIndex);

unsigned long SeMysqlResultGetFieldLen(struct SEMYSQLRESULT *pkMysqlResult, unsigned int iIndex);//for binary data

const char* SeMysqlResultGetFieldValue(struct SEMYSQLRESULT *pkMysqlResult, unsigned int iIndex);

#ifdef	__cplusplus
}
#endif

#endif
