#ifndef __SE_NETSTREAM_H__
#define __SE_NETSTREAM_H__

#include "SeList.h"
#include "SeNetBase.h"

#define NETSTREAM_SIZE 1024

#if defined(__linux)

	struct WSABUF 
	{
		unsigned long	len;
		char			*buf;
	};

#endif

struct SENETSTREAMNODE
{
	struct SENODE		kNode;
	struct WSABUF		kStream;
	char				acStream[NETSTREAM_SIZE];
};

struct SENETSTREAM
{
	int					iListCount;
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
