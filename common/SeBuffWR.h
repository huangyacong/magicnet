#ifndef __SE_BUFF_WR_H__
#define __SE_BUFF_WR_H__

#ifdef	__cplusplus
extern "C" {
#endif

/**********************************************************************
 *
 * 
 * use BigEndian
 * 
 *
 **********************************************************************/

#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdarg.h>
#include <stdio.h>

void SeBuffReadChar(const unsigned char* pcBuff, unsigned char* pcReadBuff, int iReadLen, int* riIndex);

unsigned short SeBuffReadShort(const unsigned char* pcBuff, int* riIndex);

unsigned int SeBuffReadInt(const unsigned char* pcBuff, int* riIndex);

unsigned long long SeBuffReadLongLong(const unsigned char* pcBuff, int* riIndex);

float SeBuffReadFloat(const unsigned char* pcBuff, int* riIndex);

double SeBuffReadDouble(const unsigned char* pcBuff, int* riIndex);


void SeBuffWriteChar(unsigned char* pcBuff, const unsigned char* pcWriteBuf, int iReadLen, int* riIndex);

void SeBuffWriteShort(unsigned char* pcBuff, unsigned short usValue, int* riIndex);

void SeBuffWriteInt(unsigned char* pcBuff, unsigned int uiValue, int* riIndex);

void SeBuffWriteLongLong(unsigned char* pcBuff, unsigned long long ullValue, int* riIndex);

void SeBuffWriteFloat(unsigned char* pcBuff, float fValue, int* riIndex);

void SeBuffWriteDouble(unsigned char* pcBuff, double dValue, int* riIndex);

#ifdef	__cplusplus
}
#endif

#endif



