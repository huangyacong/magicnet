#ifndef __SE_NETSTREAM_H__
#define __SE_NETSTREAM_H__

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include "SeList.h"
#include "SeTool.h"

// 32 size
struct SENETSTREAMNODE
{
	struct SENODE		kNode;
	unsigned short		usMaxLen;
	unsigned short		usReadPos;
	unsigned short		usWritePos;
	unsigned short		usStreamFlag;
	char				*pkBuf;
};

// 32 size
struct SENETSTREAM
{
	struct SELIST		kList;
	int					iCount;
	int					iFlag;
	long long			llSize;
};

struct SENETSTREAMBUF
{
	const char			*pcBuf;
	int					iBufLen;
};



typedef bool (*SEGETHEADERLENFUN)(const unsigned char*, const int, int*);

typedef bool (*SESETHEADERLENFUN)(unsigned char*, const int, const int);



void SeNetSreamInit(struct SENETSTREAM *pkNetStream);

struct SENETSTREAMNODE *SeNetSreamNodeFormat(char *pcBuf, int iBufLen);

void SeNetSreamNodeZero(struct SENETSTREAMNODE *pkNetStreamNode);

int SeNetSreamCount(struct SENETSTREAM *pkNetStream);

long long SeGetNetSreamLen(struct SENETSTREAM *pkNetStream);



bool SeNetSreamSetHeader(unsigned char* pcHeader, const int iheaderlen, const int ilen);

bool SeNetSreamGetHeader(const unsigned char* pcHeader, const int iheaderlen, int *ilen);


struct SENETSTREAMNODE *SeNetSreamGetHead(struct SENETSTREAM *pkNetStream);


struct SENETSTREAMNODE *SeNetSreamHeadPop(struct SENETSTREAM *pkNetStream);

void SeNetSreamHeadAdd(struct SENETSTREAM *pkNetStream, struct SENETSTREAMNODE *pkNetStreamNode);

struct SENETSTREAMNODE *SeNetSreamTailPop(struct SENETSTREAM *pkNetStream);

void SeNetSreamTailAdd(struct SENETSTREAM *pkNetStream, struct SENETSTREAMNODE *pkNetStreamNode);


bool SeNetSreamCanRead(struct SENETSTREAM *pkNetStream, SEGETHEADERLENFUN pkGetHeaderLenFun, int iHeaderSize);

bool SeNetSreamCanWrite(struct SENETSTREAM *pkNetStream, SESETHEADERLENFUN pkSetHeaderLenFun, int iHeaderSize, int iWriteLen);

bool SeNetSreamRead(struct SENETSTREAM *pkNetStream, struct SENETSTREAM *pkNetStreamIdle, SEGETHEADERLENFUN pkGetHeaderLenFun, int iHeaderSize, char *pcBuf, int *riBufLen);

bool SeNetSreamWrite(struct SENETSTREAM *pkNetStream, struct SENETSTREAM *pkNetStreamIdle, SESETHEADERLENFUN pkSetHeaderLenFun, int iHeaderSize, const char *pcBuf, int iBufLen);

bool SeNetSreamWriteExtend(struct SENETSTREAM *pkNetStream, struct SENETSTREAM *pkNetStreamIdle, SESETHEADERLENFUN pkSetHeaderLenFun, int iHeaderSize, const struct SENETSTREAMBUF *pkBufList, int iNum);

#ifdef	__cplusplus
}
#endif

#endif
