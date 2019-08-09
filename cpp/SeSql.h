#ifndef __SE_SQL_H__
#define __SE_SQL_H__

#include <string>
#include "SeMysql.h"

using namespace std;

class SeSql;
class SeSqlResult
{
public:
	SeSqlResult();
	virtual ~SeSqlResult();
public:
	// 取得下一条记录
	bool NextRecord();
	// 根据序号获取某条记录某列的值
	const char* GetFieldValue(unsigned int iIndex);
public:
	int GetFieldValueToInt(unsigned int iIndex);
	long long GetFieldValueToLongLong(unsigned int iIndex);
public:
	// 根据序号获取某条记录某列的值是否是数子类型
	bool IsFieldValueNumType(unsigned int iIndex);
	// 根据序号获取字段名字
	const char* GetFieldName(unsigned int iIndex);
	// 根据序号获取某条记录某列的值的长度(通常使用在二进制类型，如果是char类型，必然以\0结尾，不必使用这个函数)
	unsigned long GetFieldLen(unsigned int iIndex);
private:
	// 每条记录有几列
	unsigned int GetFieldCount();
private:
	friend class SeSql;
	SEMYSQLRESULT	m_kMySqlResult;
};

class SeSql
{
public:
	SeSql(const char* pcHost, unsigned int uiPort, const char* pcDBName, const char* pcUser, const char* pcPasswd, const char* pcCharacterSet);
	virtual ~SeSql();
public:
	bool TryConnect();
public:
	// 请使用 SeTool.h 中的 SeSnprintf 来构造SQL语句
	bool ExecuteSql(const char *pcQuerySql, unsigned long ulLen);
	bool StoreResult(SeSqlResult *pkMysqlResult);
	unsigned long long GetLastInsertId();
	unsigned long long GetAffectedRows();
public:
	const string Escape(const char *pcSrc, unsigned long ulSrcLen);
public:
	bool SetAutoCommit(bool bAutoCommit);
	bool Commit();
	bool Rollback();
public:
	unsigned int GetErrorCode();
	const char* GetErrorStr();
public:
	bool IsConnect();
	void DisConnect();
private:
	bool NextResult();
private:
	SEMYSQL m_kMySql;
	char m_acCharacterSet[128];
	char *m_pcEscape;
};

bool SeInitMysqlLibrary();

void SeEndMysqlLibrary();

#endif