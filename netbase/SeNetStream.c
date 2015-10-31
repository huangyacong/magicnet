#include "SeNetStream.h"

void SeNetSreamInit(struct SENETSTREAM *pkNetStream)
{
	pkNetStream->iCount = 0;
	SeListInit(&pkNetStream->kList);
}

struct SENETSTREAMNODE *SeNetSreamNodeFormat(char *pcBuf, int iBufLen)
{
	struct SENETSTREAMNODE *pkNetStreamNode;

	assert(sizeof(struct SENETSTREAMNODE) >= iBufLen);
	pkNetStreamNode = (struct SENETSTREAMNODE *)pcBuf;
	pkNetStreamNode->iMaxLen = iBufLen - sizeof(struct SENETSTREAMNODE);
	pkNetStreamNode->pkBuf = pcBuf + sizeof(struct SENETSTREAMNODE);
	SeNetSreamNodeZero(pkNetStreamNode);
	return pkNetStreamNode;
}

void SeNetSreamNodeZero(struct SENETSTREAMNODE *pkNetStreamNode)
{
	SeListInitNode(&pkNetStreamNode->kNode);
	pkNetStreamNode->iFlag = 0;
	pkNetStreamNode->iReadPos = 0;
	pkNetStreamNode->iWritePos = 0;
}

int SeNetSreamCount(struct SENETSTREAM *pkNetStream)
{
	return (int)pkNetStream->iCount;
}

struct SENETSTREAMNODE *SeNetSreamHeadPop(struct SENETSTREAM *pkNetStream)
{
	struct SENODE *pkNode = 0;
	struct SENETSTREAMNODE *pkNetStreamNode = 0;
	
	pkNode = SeListHeadPop(&pkNetStream->kList);
	if(!pkNode) return 0;
	pkNetStreamNode = SE_CONTAINING_RECORD(pkNode, struct SENETSTREAMNODE, kNode);
	pkNetStream->iCount--;
	return pkNetStreamNode;
}

void SeNetSreamHeadAdd(struct SENETSTREAM *pkNetStream, struct SENETSTREAMNODE *pkNetStreamNode)
{
	SeListHeadAdd(&pkNetStream->kList, &pkNetStreamNode->kNode);
	pkNetStream->iCount++;
}

void SeNetSreamTailAdd(struct SENETSTREAM *pkNetStream, struct SENETSTREAMNODE *pkNetStreamNode)
{
	SeListTailAdd(&pkNetStream->kList, &pkNetStreamNode->kNode);
	pkNetStream->iCount++;
}

bool SeNetSreamRead(struct SENETSTREAM *pkNetStream, struct SENETSTREAM *pkNetStreamIdle, SEGETHEADERLENFUN pkGetHeaderLenFun, int iHeaderSize, char *pcBuf, int &riBufLen)
{
}

bool SeNetSreamWrite(struct SENETSTREAM *pkNetStream, struct SENETSTREAM *pkNetStreamIdle, SESETHEADERLENFUN pkSetHeaderLenFun, int iHeaderSize, char *pcBuf, int iBufLen)
{
}