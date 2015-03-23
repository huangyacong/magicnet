#include "SeNetStream.h"

void SeNetSreamInit(struct SENETSTREAM *pkNetStream)
{
	pkNetStream->llListCount = 0;
	SeListInit(&pkNetStream->kList);
}

void SeNetSreamNodeInit(struct SENETSTREAMNODE *pkNetStreamNode)
{
	SeListInitNode(&pkNetStreamNode->kNode);
	pkNetStreamNode->iFlag = 0;
	pkNetStreamNode->iMaxLen = 0;
	pkNetStreamNode->iReadPos = 0;
	pkNetStreamNode->iWritePos = 0;
	pkNetStreamNode->pkBuf = 0;
}

int SeNetSreamCount(struct SENETSTREAM *pkNetStream)
{
	return (int)pkNetStream->llListCount;
}

void SeNetSreamHeadAdd(struct SENETSTREAM *pkNetStream, struct SENETSTREAMNODE *pkNetStreamNode)
{
	SeListHeadAdd(&pkNetStream->kList, &pkNetStreamNode->kNode);
	pkNetStream->llListCount++;
}

void SeNetSreamTailAdd(struct SENETSTREAM *pkNetStream, struct SENETSTREAMNODE *pkNetStreamNode)
{
	SeListTailAdd(&pkNetStream->kList, &pkNetStreamNode->kNode);
	pkNetStream->llListCount++;
}

struct SENETSTREAMNODE *SeNetSreamHeadPop(struct SENETSTREAM *pkNetStream)
{
	struct SENODE *pkNode = 0;
	struct SENETSTREAMNODE *pkNetStreamNode = 0;
	
	pkNode = SeListHeadPop(&pkNetStream->kList);
	if(!pkNode) return 0;
	pkNetStreamNode = SE_CONTAINING_RECORD(pkNode, struct SENETSTREAMNODE, kNode);
	pkNetStream->llListCount--;
	return pkNetStreamNode;
}

struct SENETSTREAMNODE *SeNetSreamTailPop(struct SENETSTREAM *pkNetStream)
{
	struct SENODE *pkNode = 0;
	struct SENETSTREAMNODE *pkNetStreamNode = 0;

	pkNode = SeListTailPop(&pkNetStream->kList);
	if(!pkNode) return 0;
	pkNetStreamNode = SE_CONTAINING_RECORD(pkNode, struct SENETSTREAMNODE, kNode);
	pkNetStream->llListCount--;
	return pkNetStreamNode;
}
