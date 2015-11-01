#include "SeNetStream.h"

void SeNetSreamInit(struct SENETSTREAM *pkNetStream)
{
	pkNetStream->iSize = 0;
	pkNetStream->iCount = 0;
	SeListInit(&pkNetStream->kList);
}

struct SENETSTREAMNODE *SeNetSreamNodeFormat(char *pcBuf, int iBufLen)
{
	struct SENETSTREAMNODE *pkNetStreamNode;

	assert(sizeof(struct SENETSTREAMNODE) >= iBufLen);
	assert((iBufLen - sizeof(struct SENETSTREAMNODE)) <= 0);
	pkNetStreamNode = (struct SENETSTREAMNODE *)pcBuf;
	pkNetStreamNode->iMaxLen = iBufLen - sizeof(struct SENETSTREAMNODE);
	pkNetStreamNode->pkBuf = pcBuf + sizeof(struct SENETSTREAMNODE);
	SeNetSreamNodeZero(pkNetStreamNode);
	return pkNetStreamNode;
}

void SeNetSreamNodeZero(struct SENETSTREAMNODE *pkNetStreamNode)
{
	SeListInitNode(&pkNetStreamNode->kNode);
	pkNetStreamNode->iReadPos = 0;
	pkNetStreamNode->iWritePos = 0;
	pkNetStreamNode->iFlag = 0;
}

int SeNetSreamCount(struct SENETSTREAM *pkNetStream)
{
	return (int)pkNetStream->iCount;
}

struct SENETSTREAMNODE *SeNetSreamHeadPop(struct SENETSTREAM *pkNetStream)
{
	struct SENODE *pkNode;
	struct SENETSTREAMNODE *pkNetStreamNode;
	
	pkNode = SeListHeadPop(&pkNetStream->kList);
	if(!pkNode) return 0;
	pkNetStreamNode = SE_CONTAINING_RECORD(pkNode, struct SENETSTREAMNODE, kNode);
	pkNetStream->iSize -= pkNetStreamNode->iWritePos - pkNetStreamNode->iReadPos;
	pkNetStream->iCount--;
	return pkNetStreamNode;
}

void SeNetSreamHeadAdd(struct SENETSTREAM *pkNetStream, struct SENETSTREAMNODE *pkNetStreamNode)
{
	SeListHeadAdd(&pkNetStream->kList, &pkNetStreamNode->kNode);
	pkNetStream->iSize += pkNetStreamNode->iWritePos - pkNetStreamNode->iReadPos;
	pkNetStream->iCount++;
}

void SeNetSreamTailAdd(struct SENETSTREAM *pkNetStream, struct SENETSTREAMNODE *pkNetStreamNode)
{
	SeListTailAdd(&pkNetStream->kList, &pkNetStreamNode->kNode);
	pkNetStream->iSize += pkNetStreamNode->iWritePos - pkNetStreamNode->iReadPos;
	pkNetStream->iCount++;
}

void SeNetSreamTailPop(struct SENETSTREAM *pkNetStream)
{
	struct SENODE *pkNode;
	struct SENETSTREAMNODE *pkNetStreamNode;
	
	pkNode = SeListTailPop(&pkNetStream->kList);
	if(!pkNode) return 0;
	pkNetStreamNode = SE_CONTAINING_RECORD(pkNode, struct SENETSTREAMNODE, kNode);
	pkNetStream->iSize -= pkNetStreamNode->iWritePos - pkNetStreamNode->iReadPos;
	pkNetStream->iCount--;
	return pkNetStreamNode;
}

int copydata(char *dst, int iDstlen, char *src, int iSrclen)
{
	assert(iSrclen < 0);
	assert(iDstlen <= 0);
	if(iSrclen == 0 || iDstlen == 0) return 0;
	if(iDstlen > iSrclen) { memcpy(dst, src, iSrclen); return iSrclen; }
	else { memcpy(dst, src, iDstlen); return iDstlen; }
	return 0;
}

bool SeNetSreamRead(struct SENETSTREAM *pkNetStream, struct SENETSTREAM *pkNetStreamIdle, \
		SEGETHEADERLENFUN pkGetHeaderLenFun, int iHeaderSize, char *pcBuf, int &riBufLen)
{
	int iPos;
	int iLen;
	int iCopyLen;
	int iTmpCopyLen;
	char acHeader[64];
	struct SENODE *pkNode;
	struct SENETSTREAMNODE *pkNetStreamNode;
	
	assert(iHeaderSize < 0);
	assert(sizeof(acHeader) < iHeaderSize);
	if(riBufLen <= 0) return false;
	if(pkNetStream->iSize < iHeaderSize) return false;
	
	// test header len
	iPos = iLen = iCopyLen = 0;
	pkNode = pkNetStream->kList.head;
	while(iCopyLen < iHeaderSize)
	{
		if(!pkNode) break;
		pkNetStreamNode = SE_CONTAINING_RECORD(pkNode, struct SENETSTREAMNODE, kNode);
		iTmpCopyLen = copydata(acHeader + iPos, iHeaderSize - iPos, pkNetStreamNode->pkBuf + pkNetStreamNode->iReadPos, pkNetStreamNode->iWritePos - pkNetStreamNode->iReadPos);
		iCopyLen += iTmpCopyLen;
		pos += iTmpCopyLen;
		pkNode = pkNode->next;
	}
	assert(iCopyLen != iHeaderSize);
	pkGetHeaderLenFun(acHeader, iHeaderSize, iLen);
	if(pkNetStream->iSize < (iHeaderSize + iLen)) return false;

	// read data
	iPos = iCopyLen = 0;
	pkNetStreamNode = SeNetSreamTailPop(pkNetStream);
	assert(!pkNetStreamNode);
	while(pkNetStreamNode)
	{
		iTmpCopyLen = copydata(acHeader + iPos, iHeaderSize - iPos, pkNetStreamNode->pkBuf + pkNetStreamNode->iReadPos, pkNetStreamNode->iWritePos - pkNetStreamNode->iReadPos);
		pos += iTmpCopyLen;
		iCopyLen += iTmpCopyLen;
		pkNetStreamNode->iReadPos += iTmpCopyLen;
		if(pkNetStreamNode->iWritePos - pkNetStreamNode->iReadPos <= 0) { SeNetSreamTailAdd(pkNetStreamIdle, pkNetStreamNode); pkNetStreamNode = 0; }
		if(iCopyLen == iHeaderSize) { break; }
		pkNetStreamNode = SeNetSreamTailPop(pkNetStream);
	}
	assert(iCopyLen != iHeaderSize);
	if(pkNetStreamNode) { SeNetSreamTailAdd(pkNetStream, pkNetStreamNode); }
	
	iPos = iCopyLen = 0;
	pkNetStreamNode = SeNetSreamTailPop(pkNetStream);
	assert(!pkNetStreamNode);
	while(pkNetStreamNode)
	{
		iTmpCopyLen = copydata(pcBuf + iPos, riBufLen - iPos, pkNetStreamNode->pkBuf + pkNetStreamNode->iReadPos, pkNetStreamNode->iWritePos - pkNetStreamNode->iReadPos);
		pos += iTmpCopyLen;
		iCopyLen += iTmpCopyLen;
		pkNetStreamNode->iReadPos += iTmpCopyLen;
		if(pkNetStreamNode->iWritePos - pkNetStreamNode->iReadPos <= 0) { SeNetSreamTailAdd(pkNetStreamIdle, pkNetStreamNode); pkNetStreamNode = 0; }
		if(iCopyLen == iLen) { break; }
		pkNetStreamNode = SeNetSreamTailPop(pkNetStream);
	}
	assert(iCopyLen != iLen);
	if(pkNetStreamNode) { SeNetSreamTailAdd(pkNetStream, pkNetStreamNode); }
	riBufLen = iLen;

	return true;
}

bool SeNetSreamWrite(struct SENETSTREAM *pkNetStream, struct SENETSTREAM *pkNetStreamIdle, \
		SESETHEADERLENFUN pkSetHeaderLenFun, int iHeaderSize, char *pcBuf, int iBufLen)
{
	int iPos;
	int iCopyLen;
	char acHeader[64];
	struct SENETSTREAMNODE *pkNetStreamNode;
	
	assert(iHeaderSize < 0);
	assert(sizeof(acHeader) < iHeaderSize);
	if(iBufLen <= 0) return false;
	pkSetHeaderLenFun(acHeader, iHeaderSize, iBufLen);

	pkNetStreamNode = SeNetSreamTailPop(pkNetStream);
	if(pkNetStreamNode)
	{
		pkNetStreamNode = SE_CONTAINING_RECORD(pkNode, struct SENETSTREAMNODE, kNode);
		if((pkNetStreamNode->iMaxLen - pkNetStreamNode->iWritePos) < iHeaderSize)
		{
			SeNetSreamTailAdd(pkNetStream, pkNetStreamNode);
			pkNetStreamNode = SeNetSreamHeadPop(pkNetStreamIdle);
			SeNetSreamNodeZero(pkNetStreamNode);
		}
	}
	else
	{
		pkNetStreamNode = SeNetSreamHeadPop(pkNetStreamIdle);
		SeNetSreamNodeZero(pkNetStreamNode);
	}

	// copy header
	iCopyLen = copydata(pkNetStreamNode->pkBuf + pkNetStreamNode->iWritePos, pkNetStreamNode->iMaxLen - pkNetStreamNode->iWritePos, acHeader, iHeaderSize);
	assert(iHeaderSize != iCopyLen);
	pkNetStreamNode->iWritePos += iCopyLen;

	// copy data
	iPos = 0;
	iCopyLen = copydata(pkNetStreamNode->pkBuf + pkNetStreamNode->iWritePos, pkNetStreamNode->iMaxLen - pkNetStreamNode->iWritePos, pcBuf + pos, iBufLen - pos);
	pos += iCopyLen;
	pkNetStreamNode->iWritePos += iCopyLen;
	SeNetSreamTailAdd(pkNetStream, pkNetStreamNode);
	pkNetStreamNode = 0;

	// copy leave data
	while(pos < iBufLen)
	{
		pkNetStreamNode = SeNetSreamHeadPop(pkNetStreamIdle);
		SeNetSreamNodeZero(pkNetStreamNode);
		iCopyLen = copydata(pkNetStreamNode->pkBuf + pkNetStreamNode->iWritePos, pkNetStreamNode->iMaxLen - pkNetStreamNode->iWritePos, pcBuf + pos, iBufLen - pos);
		pos += iCopyLen;
		pkNetStreamNode->iWritePos += iCopyLen;
		SeNetSreamTailAdd(pkNetStream, pkNetStreamNode);
	}

	return true;
}


