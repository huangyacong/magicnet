#ifndef __SE_NETSTREAM_H__
#define __SE_NETSTREAM_H__

#include "SeList.h"
#include "SeBool.h"
#include "SeNetBase.h"

struct SENETSTREAMNODE
{
	struct SENODE		kNode;
	int					iFlag;
	int					iMaxLen;
	int					iReadPos;
	int					iWritePos;
	char				*pkBuf;
};

struct SENETSTREAM
{
	int					iCount;
	struct SELIST		kList;
};



typedef void (*SEGETHEADERLENFUN)(const char*, const int, int&);

typedef void (*SESETHEADERLENFUN)(char*, const int, const int);



void SeNetSreamInit(struct SENETSTREAM *pkNetStream);

struct SENETSTREAMNODE *SeNetSreamNodeFormat(char *pcBuf, int iBufLen);

void SeNetSreamNodeZero(struct SENETSTREAMNODE *pkNetStreamNode);

int SeNetSreamCount(struct SENETSTREAM *pkNetStream);



struct SENETSTREAMNODE *SeNetSreamHeadPop(struct SENETSTREAM *pkNetStream);

void SeNetSreamHeadAdd(struct SENETSTREAM *pkNetStream, struct SENETSTREAMNODE *pkNetStreamNode);

void SeNetSreamTailAdd(struct SENETSTREAM *pkNetStream, struct SENETSTREAMNODE *pkNetStreamNode);



bool SeNetSreamRead(struct SENETSTREAM *pkNetStream, struct SENETSTREAM *pkNetStreamIdle, SEGETHEADERLENFUN pkGetHeaderLenFun, int iHeaderSize, char *pcBuf, int &riBufLen);

bool SeNetSreamWrite(struct SENETSTREAM *pkNetStream, struct SENETSTREAM *pkNetStreamIdle, SESETHEADERLENFUN pkSetHeaderLenFun, int iHeaderSize, char *pcBuf, int iBufLen);

#endif
