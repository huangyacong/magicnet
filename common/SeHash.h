#ifndef __SE_HASH_H__
#define __SE_HASH_H__

#include "SeList.h"

struct SEHASHNODE
{
	int id;
	struct SENODE main;
	struct SENODE list;
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


struct SEHASHNODE *SeHashGet(struct SEHASH *root, int id);

struct SEHASHNODE *SeHashRemove(struct SEHASH *root, struct SEHASHNODE *node);

struct SEHASHNODE *SeHashPop(struct SEHASH *root);

void SeHashAdd(struct SEHASH *root, int id, struct SEHASHNODE *node);

#endif


