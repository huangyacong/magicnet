#ifndef __SE_NETSTREAM_H__
#define __SE_NETSTREAM_H__

#include "SeList.h"
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
	long long			llListCount;
	struct SELIST		kList;
};

void SeNetSreamInit(struct SENETSTREAM *pkNetStream);

void SeNetSreamNodeInit(struct SENETSTREAMNODE *pkNetStreamNode);

int SeNetSreamCount(struct SENETSTREAM *pkNetStream);

void SeNetSreamHeadAdd(struct SENETSTREAM *pkNetStream, struct SENETSTREAMNODE *pkNetStreamNode);

void SeNetSreamTailAdd(struct SENETSTREAM *pkNetStream, struct SENETSTREAMNODE *pkNetStreamNode);

struct SENETSTREAMNODE *SeNetSreamHeadPop(struct SENETSTREAM *pkNetStream);

struct SENETSTREAMNODE *SeNetSreamTailPop(struct SENETSTREAM *pkNetStream);

#endif
