#include "SeBuffWR.h"
#include "SeNetBase.h"
#include "SeTool.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdarg.h>

void SeBuffReadChar(const unsigned char* pcBuff, unsigned char* pcReadBuff, int iReadLen, int* riIndex)
{
	const unsigned char* pcResult;

	assert(iReadLen > 0);
	pcResult = &pcBuff[*riIndex];
	memcpy(pcReadBuff, pcResult, iReadLen);
	*riIndex = *riIndex + iReadLen;
}

unsigned short SeBuffReadShort(const unsigned char* pcBuff, int* riIndex)
{
	unsigned short* pResult;
	unsigned short usResult;
	const unsigned char* pcResult;

	pcResult = &pcBuff[*riIndex];
	pResult = (unsigned short*)pcResult;
	usResult = *pResult;
	*riIndex = *riIndex + (int)sizeof(unsigned short);

	if(SeLocalIsLittleEndian())
		usResult = SeBigToLittleEndianS(usResult);

	return usResult;
}

unsigned int SeBuffReadInt(const unsigned char* pcBuff, int* riIndex)
{
	unsigned int* pResult;
	unsigned int uiResult;
	const unsigned char* pcResult;

	pcResult = &pcBuff[*riIndex];
	pResult = (unsigned int*)pcResult;
	uiResult = *pResult;
	*riIndex = *riIndex + (int)sizeof(unsigned int);

	if(SeLocalIsLittleEndian())
		uiResult = SeBigToLittleEndianL(uiResult);

	return uiResult;
}

unsigned long long SeBuffReadLongLong(const unsigned char* pcBuff, int* riIndex)
{
	unsigned long long* pResult;
	unsigned long long ullResult;
	const unsigned char* pcResult;

	pcResult = &pcBuff[*riIndex];
	pResult = (unsigned long long*)pcResult;
	ullResult = *pResult;
	*riIndex = *riIndex + (int)sizeof(unsigned long long);

	if(SeLocalIsLittleEndian())
		ullResult = SeBigToLittleEndianLL(ullResult);

	return ullResult;
}

float SeBuffReadFloat(const unsigned char* pcBuff, int* riIndex)
{
	unsigned int* pResult;
	float fResult;
	float* pfTmp;
	const unsigned char* pcResult;

	pcResult = &pcBuff[*riIndex];
	pResult = (unsigned int*)pcResult;
	pfTmp = (float*)pcResult;
	fResult = *pfTmp;
	*riIndex = *riIndex + (int)sizeof(float);

	if(SeLocalIsLittleEndian())
		fResult = SeBigToLittleEndianF(*pResult);

	return fResult;
}

double SeBuffReadDouble(const unsigned char* pcBuff, int* riIndex)
{
	unsigned long long* pResult;
	double dResult;
	double* pdTmp;
	const unsigned char* pcResult;

	pcResult = &pcBuff[*riIndex];
	pResult = (unsigned long long*)pcResult;
	pdTmp = (double*)pcResult;
	dResult = *pdTmp;
	*riIndex = *riIndex + (int)sizeof(double);

	if(SeLocalIsLittleEndian())
		dResult = SeBigToLittleEndianDF(*pResult);

	return dResult;
}

void SeBuffWriteChar(unsigned char* pcBuff, const unsigned char* pcWriteBuf, int iReadLen, int* riIndex)
{
	assert(iReadLen > 0);
	assert(*riIndex >= 0);
	memcpy(pcBuff + *riIndex, pcWriteBuf, iReadLen);
	*riIndex = *riIndex + iReadLen;
}

void SeBuffWriteShort(unsigned char* pcBuff, unsigned short usValue, int* riIndex)
{
	unsigned short usTmp;
	assert(*riIndex >= 0);

	usTmp = usValue;
	if(SeLocalIsLittleEndian())
		usTmp = SeLittleToBigEndianS(usValue);

	memcpy(pcBuff + *riIndex, &usTmp, sizeof(usTmp));
	*riIndex = *riIndex + (int)sizeof(usTmp);
}

void SeBuffWriteInt(unsigned char* pcBuff, unsigned int uiValue, int* riIndex)
{
	unsigned int uiTmp;
	assert(*riIndex >= 0);

	uiTmp = uiValue;
	if(SeLocalIsLittleEndian())
		uiTmp = SeLittleToBigEndianL(uiValue);

	memcpy(pcBuff + *riIndex, &uiTmp, sizeof(uiTmp));
	*riIndex = *riIndex + (int)sizeof(uiTmp);
}

void SeBuffWriteLongLong(unsigned char* pcBuff, unsigned long long ullValue, int* riIndex)
{
	unsigned long long ullTmp;
	assert(*riIndex >= 0);

	ullTmp = ullValue;
	if(SeLocalIsLittleEndian())
		ullTmp = SeLittleToBigEndianLL(ullValue);

	memcpy(pcBuff + *riIndex, &ullTmp, sizeof(ullTmp));
	*riIndex = *riIndex + (int)sizeof(ullTmp);
}

void SeBuffWriteFloat(unsigned char* pcBuff, float fValue, int* riIndex)
{
	unsigned int* p;
	unsigned int uiTmp;
	assert(*riIndex >= 0);

	p = (unsigned int*)(&fValue);
	if(SeLocalIsLittleEndian())
		uiTmp = SeLittleToBigEndianF(fValue);
	else
		uiTmp = *p;

	memcpy(pcBuff + *riIndex, &uiTmp, sizeof(uiTmp));
	*riIndex = *riIndex + (int)sizeof(uiTmp);
}

void SeBuffWriteDouble(unsigned char* pcBuff, double dValue, int* riIndex)
{
	unsigned long long* p;
	unsigned long long ullTmp;
	assert(*riIndex >= 0);

	p = (unsigned long long*)(&dValue);
	if(SeLocalIsLittleEndian())
		ullTmp = SeLittleToBigEndianDF(dValue);
	else
		ullTmp = *p;

	memcpy(pcBuff + *riIndex, &ullTmp, sizeof(ullTmp));
	*riIndex = *riIndex + (int)sizeof(ullTmp);
}