#ifndef __SE_NETSTREAM_H__
#define __SE_NETSTREAM_H__

#ifdef	__cplusplus
extern "C" {
#endif

#include "SeList.h"
#include <stdbool.h>
#include "SeTool.h"

// 32 size
struct SENETSTREAMNODE
{
	struct SENODE		kNode;
	char				*pkBuf;
	unsigned short		usMaxLen;
	unsigned short		usReadPos;
	unsigned short		usWritePos;
	unsigned short		usStreamFlag;
};

// 32 size
struct SENETSTREAM
{
	struct SELIST		kList;
	int					iCount;
	int					iSize;
	long long			llFlag;
};



typedef bool (*SEGETHEADERLENFUN)(const unsigned char*, const int, int*);

typedef bool (*SESETHEADERLENFUN)(unsigned char*, const int, const int);



void SeNetSreamInit(struct SENETSTREAM *pkNetStream);

struct SENETSTREAMNODE *SeNetSreamNodeFormat(char *pcBuf, int iBufLen);

void SeNetSreamNodeZero(struct SENETSTREAMNODE *pkNetStreamNode);

int SeNetSreamCount(struct SENETSTREAM *pkNetStream);



struct SENETSTREAMNODE *SeNetSreamHeadPop(struct SENETSTREAM *pkNetStream);

void SeNetSreamHeadAdd(struct SENETSTREAM *pkNetStream, struct SENETSTREAMNODE *pkNetStreamNode);



bool SeNetSreamRead(struct SENETSTREAM *pkNetStream, struct SENETSTREAM *pkNetStreamIdle, SEGETHEADERLENFUN pkGetHeaderLenFun, int iHeaderSize, char *pcBuf, int *riBufLen);

bool SeNetSreamWrite(struct SENETSTREAM *pkNetStream, struct SENETSTREAM *pkNetStreamIdle, SESETHEADERLENFUN pkSetHeaderLenFun, int iHeaderSize, char *pcBuf, int iBufLen);

#ifdef	__cplusplus
}
#endif

#endif
