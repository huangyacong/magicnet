#include "SeNetStream.h"

void SeNetSreamInit(struct SENETSTREAM *pkNetStream)
{
	pkNetStream->iListCount = 0;
	SeListInit(&pkNetStream->kList);
}

void SeNetSreamNodeInit(struct SENETSTREAMNODE *pkNetStreamNode)
{
	SeListInitNode(&pkNetStreamNode->kNode);
	pkNetStreamNode->kStream.len = 0;
	pkNetStreamNode->kStream.buf = pkNetStreamNode->acStream;
#if defined(_DEBUG)
	memset(pkNetStreamNode->acStream, 0, sizeof(pkNetStreamNode->acStream));
#endif
}

void SeNetSreamHeadAdd(struct SENETSTREAM *pkNetStream, struct SENETSTREAMNODE *pkNetStreamNode)
{
	SeListHeadAdd(&pkNetStream->kList, &pkNetStreamNode->kNode);
	pkNetStream->iListCount++;
}

void SeNetSreamTailAdd(struct SENETSTREAM *pkNetStream, struct SENETSTREAMNODE *pkNetStreamNode)
{
	SeListTailAdd(&pkNetStream->kList, &pkNetStreamNode->kNode);
	pkNetStream->iListCount++;
}

struct SENETSTREAMNODE *SeNetSreamHeadPop(struct SENETSTREAM *pkNetStream)
{
	struct SENODE *pkNode = 0;
	struct SENETSTREAMNODE *pkNetStreamNode = 0;
	
	pkNode = SeListHeadPop(&pkNetStream->kList);
	if(!pkNode) return 0;
	pkNetStreamNode = SE_CONTAINING_RECORD(pkNode, struct SENETSTREAMNODE, kNode);
	pkNetStream->iListCount--;
	return pkNetStreamNode;
}

struct SENETSTREAMNODE *SeNetSreamTailPop(struct SENETSTREAM *pkNetStream)
{
	struct SENODE *pkNode = 0;
	struct SENETSTREAMNODE *pkNetStreamNode = 0;

	pkNode = SeListTailPop(&pkNetStream->kList);
	if(!pkNode) return 0;
	pkNetStreamNode = SE_CONTAINING_RECORD(pkNode, struct SENETSTREAMNODE, kNode);
	pkNetStream->iListCount--;
	return pkNetStreamNode;
}
