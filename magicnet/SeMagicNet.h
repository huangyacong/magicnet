#ifndef __SE_MAGICNET_H__
#define __SE_MAGICNET_H__

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "SeNetCore.h"

struct REGSVRNODE
{
	int						iSvrNo;
	struct SEHASHNODE		kHashNode;
};

struct SEMAGICNETS
{
	struct SENETCORE		kNetCore;
	struct SEHASH			kRegSvrList;
};

struct SEMAGICNETC
{
	int						iSvrNo;
	struct SENETCORE		kNetCore;
};

bool SeSetHeader(char* pcHeader, const int iheaderlen, const int ilen);

bool SeGetHeader(const char* pcHeader, const int iheaderlen, int *ilen);

#endif
