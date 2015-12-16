#ifndef __SE_HASH_H__
#define __SE_HASH_H__

#include "SeList.h"
#include <stdbool.h>

struct SEHASHNODE
{
	struct SENODE main;
	struct SENODE list;
	int id;
};

struct SEHASH
{
	int max;
	struct SELIST *pkMain;
	struct SELIST list;
};

void SeHashInit(struct SEHASH *root, int max);

void SeHashFin(struct SEHASH *root);

void SeHashNodeInit(struct SEHASHNODE *node);


void SeHashAdd(struct SEHASH *root, int id, struct SEHASHNODE *node);

struct SEHASHNODE *SeHashGet(struct SEHASH *root, int id);

struct SEHASHNODE *SeHashGetHead(struct SEHASH *root);

struct SEHASHNODE *SeHashRemove(struct SEHASH *root, struct SEHASHNODE *node);

struct SEHASHNODE *SeHashPop(struct SEHASH *root);

void SeHashMoveToEnd(struct SEHASH *root, struct SEHASHNODE *node);

#endif


