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
	// ȡ����һ����¼
	bool NextRecord();
	// ������Ż�ȡĳ����¼ĳ�е�ֵ
	const char* GetFieldValue(unsigned int iIndex);
public:
	int GetFieldValueToInt(unsigned int iIndex);
	long long GetFieldValueToLongLong(unsigned int iIndex);
public:
	// ������Ż�ȡĳ����¼ĳ�е�ֵ�Ƿ�����������
	bool IsFieldValueNumType(unsigned int iIndex);
	// ������Ż�ȡ�ֶ�����
	const char* GetFieldName(unsigned int iIndex);
	// ������Ż�ȡĳ����¼ĳ�е�ֵ�ĳ���(ͨ��ʹ���ڶ��������ͣ������char���ͣ���Ȼ��\0��β������ʹ���������)
	unsigned long GetFieldLen(unsigned int iIndex);
private:
	// ÿ����¼�м���
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
	// ��ʹ�� SeTool.h �е� SeSnprintf ������SQL���
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