/**********************************************************************
 *
 * selist.c
 *
 * NOTE: This is a C version queue operation wrapper
 * 
 *
 **********************************************************************/

#include "SeHash.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

void SeHashInit(struct SEHASH *root, int max)
{
	int i;

	assert(max > 0);
	root->max = max;
	root->pkMain = (struct SEHASH *)malloc(sizeof(struct SELIST)*root->max);
	SeListInit(&root->list);
	
	for(i = 0; i < root->max; i++)
	{
		SeListInit(&root->pkMain[i]);
	}
}

void SeHashFin(struct SEHASH *root)
{
	free(root->pkMain);
	root->max = 0;
	root->pkMain = 0;
	SeListInit(&root->list);
}

void SeHashNodeInit(struct SEHASHNODE *node)
{
	node->id = 0;
	SeListInitNode(&node->main);
	SeListInitNode(&node->list);
}
