#include "SeNetStream.h"

void SeNetSreamInit(struct SENETSTREAM *pkNetStream)
{
	pkNetStream->iSize = 0;
	pkNetStream->iCount = 0;
	pkNetStream->llFlag = 0;
	SeListInit(&pkNetStream->kList);
}

struct SENETSTREAMNODE *SeNetSreamNodeFormat(char *pcBuf, int iBufLen)
{
	struct SENETSTREAMNODE *pkNetStreamNode;

	assert((int)sizeof(struct SENETSTREAMNODE) < iBufLen);
	assert((iBufLen - sizeof(struct SENETSTREAMNODE)) < 0xFFFF);
	pkNetStreamNode = (struct SENETSTREAMNODE *)pcBuf;
	pkNetStreamNode->pkBuf = pcBuf + sizeof(struct SENETSTREAMNODE);
	pkNetStreamNode->usMaxLen = (unsigned short)(iBufLen - sizeof(struct SENETSTREAMNODE));
	SeNetSreamNodeZero(pkNetStreamNode);
	return pkNetStreamNode;
}

void SeNetSreamNodeZero(struct SENETSTREAMNODE *pkNetStreamNode)
{
	SeListInitNode(&pkNetStreamNode->kNode);
	pkNetStreamNode->usReadPos = 0;
	pkNetStreamNode->usWritePos = 0;
	pkNetStreamNode->usStreamFlag = 0;
}

int SeNetSreamCount(struct SENETSTREAM *pkNetStream)
{
	return (int)pkNetStream->iCount;
}

int SeGetNetSreamLen(struct SENETSTREAM *pkNetStream)
{
	assert(pkNetStream->iSize >= 0);
	return pkNetStream->iSize;
}

struct SENETSTREAMNODE *SeNetSreamHeadPop(struct SENETSTREAM *pkNetStream)
{
	int iLeaveLen;
	struct SENODE *pkNode;
	struct SENETSTREAMNODE *pkNetStreamNode;
	
	pkNode = SeListHeadPop(&pkNetStream->kList);
	if(!pkNode) return 0;
	pkNetStreamNode = SE_CONTAINING_RECORD(pkNode, struct SENETSTREAMNODE, kNode);
	iLeaveLen = pkNetStreamNode->usWritePos - pkNetStreamNode->usReadPos;
	assert(iLeaveLen >= 0);
	pkNetStream->iSize -= iLeaveLen;
	pkNetStream->iCount--;
	return pkNetStreamNode;
}

void SeNetSreamHeadAdd(struct SENETSTREAM *pkNetStream, struct SENETSTREAMNODE *pkNetStreamNode)
{
	int iLeaveLen;

	SeListHeadAdd(&pkNetStream->kList, &pkNetStreamNode->kNode);
	iLeaveLen = pkNetStreamNode->usWritePos - pkNetStreamNode->usReadPos;
	assert(iLeaveLen >= 0);
	pkNetStream->iSize += iLeaveLen;
	pkNetStream->iCount++;
}

void SeNetSreamTailAdd(struct SENETSTREAM *pkNetStream, struct SENETSTREAMNODE *pkNetStreamNode)
{
	int iLeaveLen;
	SeListTailAdd(&pkNetStream->kList, &pkNetStreamNode->kNode);
	iLeaveLen = pkNetStreamNode->usWritePos - pkNetStreamNode->usReadPos;
	assert(iLeaveLen >= 0);
	pkNetStream->iSize += iLeaveLen;
	pkNetStream->iCount++;
}

struct SENETSTREAMNODE *SeNetSreamTailPop(struct SENETSTREAM *pkNetStream)
{
	int iLeaveLen;
	struct SENODE *pkNode;
	struct SENETSTREAMNODE *pkNetStreamNode;
	
	pkNode = SeListTailPop(&pkNetStream->kList);
	if(!pkNode) return 0;
	pkNetStreamNode = SE_CONTAINING_RECORD(pkNode, struct SENETSTREAMNODE, kNode);
	iLeaveLen = pkNetStreamNode->usWritePos - pkNetStreamNode->usReadPos;
	assert(iLeaveLen >= 0);
	pkNetStream->iSize -= iLeaveLen;
	pkNetStream->iCount--;
	return pkNetStreamNode;
}

bool SeNetSreamWriteLen(struct SENETSTREAM *pkNetStream, struct SENETSTREAM *pkNetStreamIdle, const char *pcBuf, int iBufLen)
{
	int iCopyLen;
	int iWritePos;
	struct SENETSTREAMNODE *pkNetStreamNode;

	if(!pcBuf || iBufLen < 0)
	{
		assert(pcBuf);
		assert(iBufLen >= 0);
		return false;
	}

	if(iBufLen == 0)
	{
		return true;
	}
	
	pkNetStreamNode = SeNetSreamTailPop(pkNetStream);
	if(pkNetStreamNode)
	{
		if(pkNetStreamNode->usMaxLen <= pkNetStreamNode->usWritePos)
		{
			SeNetSreamTailAdd(pkNetStream, pkNetStreamNode);
			pkNetStreamNode = 0;
		}
	}

	if(!pkNetStreamNode)
	{
		pkNetStreamNode = SeNetSreamHeadPop(pkNetStreamIdle);
		if(pkNetStreamNode)
		{
			SeNetSreamNodeZero(pkNetStreamNode);
		}
	}

	if(!pkNetStreamNode)
	{
		return false;
	}

	iWritePos = 0;

	assert(iBufLen >= iWritePos);
	assert(pkNetStreamNode->usMaxLen >= pkNetStreamNode->usWritePos);
	iCopyLen = SeCopyData(pkNetStreamNode->pkBuf + pkNetStreamNode->usWritePos, pkNetStreamNode->usMaxLen - pkNetStreamNode->usWritePos, pcBuf + iWritePos, iBufLen - iWritePos);
	pkNetStreamNode->usWritePos += iCopyLen;
	iWritePos += iCopyLen;
	SeNetSreamTailAdd(pkNetStream, pkNetStreamNode);

	while(iWritePos < iBufLen)
	{
		pkNetStreamNode = SeNetSreamHeadPop(pkNetStreamIdle);
		if(!pkNetStreamNode) { assert(pkNetStreamNode); return false; }
		SeNetSreamNodeZero(pkNetStreamNode);

		assert(iBufLen >= iWritePos);
		assert(pkNetStreamNode->usMaxLen >= pkNetStreamNode->usWritePos);
		iCopyLen = SeCopyData(pkNetStreamNode->pkBuf + pkNetStreamNode->usWritePos, pkNetStreamNode->usMaxLen - pkNetStreamNode->usWritePos, pcBuf + iWritePos, iBufLen - iWritePos);
		pkNetStreamNode->usWritePos += iCopyLen;
		iWritePos += iCopyLen;

		if(pkNetStreamNode->usWritePos > pkNetStreamNode->usReadPos) { assert(pkNetStreamNode->usMaxLen >= pkNetStreamNode->usWritePos); SeNetSreamTailAdd(pkNetStream, pkNetStreamNode); }
		else { assert(pkNetStreamNode->usWritePos <= pkNetStreamNode->usReadPos); SeNetSreamTailAdd(pkNetStreamIdle, pkNetStreamNode); return false; }
	}

	return (iWritePos == iBufLen);
}

bool SeNetSreamReadLen(struct SENETSTREAM *pkNetStream, struct SENETSTREAM *pkNetStreamIdle, char *pcBuf, int iBufLen)
{
	int iCopyLen;
	int iCopyPos;
	struct SENETSTREAMNODE *pkNetStreamNode;

	if(!pcBuf || iBufLen < 0 || pkNetStream->iSize < iBufLen)
	{
		assert(pcBuf);
		assert(iBufLen >= 0);
		return false;
	}

	if(iBufLen == 0)
	{
		return true;
	}
	
	iCopyPos = 0;

	while(iCopyPos < iBufLen)
	{
		pkNetStreamNode = SeNetSreamHeadPop(pkNetStream);
		if(!pkNetStreamNode) { assert(pkNetStreamNode); return false; }

		assert(iBufLen >= iCopyPos);
		assert(pkNetStreamNode->usReadPos <= pkNetStreamNode->usMaxLen);
		assert(pkNetStreamNode->usWritePos <= pkNetStreamNode->usMaxLen);
		assert(pkNetStreamNode->usWritePos >= pkNetStreamNode->usReadPos);
		iCopyLen = SeCopyData(pcBuf + iCopyPos, iBufLen - iCopyPos, pkNetStreamNode->pkBuf + pkNetStreamNode->usReadPos, pkNetStreamNode->usWritePos - pkNetStreamNode->usReadPos);
		pkNetStreamNode->usReadPos += iCopyLen;
		iCopyPos += iCopyLen;

		if(pkNetStreamNode->usWritePos <= pkNetStreamNode->usReadPos) { assert(pkNetStreamNode->usWritePos == pkNetStreamNode->usReadPos); SeNetSreamTailAdd(pkNetStreamIdle, pkNetStreamNode); }
		else { SeNetSreamHeadAdd(pkNetStream, pkNetStreamNode); break; }
	}

	return (iBufLen == iCopyPos);
}

bool SeNetSreamCanRead(struct SENETSTREAM *pkNetStream, SEGETHEADERLENFUN pkGetHeaderLenFun, int iHeaderSize)
{
	int iLen;
	int iCopyLen;
	char acHeader[64];
	struct SENODE *pkNode;
	struct SENETSTREAMNODE *pkNetStreamNode;
	
	assert(iHeaderSize >= 0);
	assert((int)sizeof(acHeader) >= iHeaderSize);
	if((int)sizeof(acHeader) < iHeaderSize || iHeaderSize < 0) return false;
	if(pkNetStream->iSize < iHeaderSize || pkNetStream->iSize <= 0) return false;
	
	// test header len
	iCopyLen = 0;
	pkNode = pkNetStream->kList.head;
	if(!pkNode) return false;
	while(iCopyLen < iHeaderSize)
	{
		if(!pkNode) break;
		pkNetStreamNode = SE_CONTAINING_RECORD(pkNode, struct SENETSTREAMNODE, kNode);
		iCopyLen += SeCopyData(acHeader + iCopyLen, iHeaderSize - iCopyLen, pkNetStreamNode->pkBuf + pkNetStreamNode->usReadPos, pkNetStreamNode->usWritePos - pkNetStreamNode->usReadPos);
		pkNode = pkNode->next;
	}
	if(iCopyLen != iHeaderSize) { return false; }

	iLen = -1;
	if(iHeaderSize > 0)
	{
		if(!pkGetHeaderLenFun((const unsigned char *)acHeader, iHeaderSize, &iLen))
		{
			return false;
		}
	}
	else
	{
		iLen = 0;
	}
	assert(iLen >= 0);
	if(iLen < 0) return false;
	if(pkNetStream->iSize < (iHeaderSize + iLen)) return false;

	return true;
}

bool SeNetSreamCanWrite(struct SENETSTREAM *pkNetStream, SESETHEADERLENFUN pkSetHeaderLenFun, int iHeaderSize, int iWriteLen)
{
	char acHeader[64];

	assert(iHeaderSize >= 0);
	assert((int)sizeof(acHeader) >= iHeaderSize);
	if(iWriteLen < 0 || (int)sizeof(acHeader) < iHeaderSize || iHeaderSize < 0) return false;

	return iHeaderSize <= 0 ? true : pkSetHeaderLenFun((unsigned char *)acHeader, iHeaderSize, iWriteLen);
}

bool SeNetSreamRead(struct SENETSTREAM *pkNetStream, struct SENETSTREAM *pkNetStreamIdle, \
		SEGETHEADERLENFUN pkGetHeaderLenFun, int iHeaderSize, char *pcBuf, int *riBufLen)
{
	char acHeader[64];
	
	assert(iHeaderSize >= 0);
	assert((int)sizeof(acHeader) >= iHeaderSize);
	if(*riBufLen <= 0 || iHeaderSize < 0 || (int)sizeof(acHeader) < iHeaderSize) return false;

	if(iHeaderSize <= 0)
	{
		*riBufLen = pkNetStream->iSize >= *riBufLen ? *riBufLen : pkNetStream->iSize;
		if(!SeNetSreamReadLen(pkNetStream, pkNetStreamIdle, pcBuf, *riBufLen)) return false;
	}
	else
	{
		*riBufLen = -1;
		if(!SeNetSreamReadLen(pkNetStream, pkNetStreamIdle, acHeader, iHeaderSize)) return false;
		if(!pkGetHeaderLenFun((const unsigned char *)acHeader, iHeaderSize, &(*riBufLen))) return false;
		if(!SeNetSreamReadLen(pkNetStream, pkNetStreamIdle, pcBuf, *riBufLen)) return false;
		assert(*riBufLen >= 0);
	}

	return true;
}

bool SeNetSreamWrite(struct SENETSTREAM *pkNetStream, struct SENETSTREAM *pkNetStreamIdle, \
		SESETHEADERLENFUN pkSetHeaderLenFun, int iHeaderSize, const char *pcBuf, int iBufLen)
{
	bool bRet;
	char acHeader[64];
	
	assert(iHeaderSize >= 0);
	assert((int)sizeof(acHeader) >= iHeaderSize);
	if(iBufLen < 0 || iHeaderSize < 0 || (int)sizeof(acHeader) < iHeaderSize) return false;

	bRet = iHeaderSize <= 0 ? true : pkSetHeaderLenFun((unsigned char *)acHeader, iHeaderSize, iBufLen);
	if(!bRet) return false;

	if(!SeNetSreamWriteLen(pkNetStream, pkNetStreamIdle, acHeader, iHeaderSize)) return false;
	if(!SeNetSreamWriteLen(pkNetStream, pkNetStreamIdle, pcBuf, iBufLen)) return false;

	return true;
}

