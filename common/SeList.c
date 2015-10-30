/**********************************************************************
 *
 * selist.c
 *
 * NOTE: This is a C version queue operation wrapper
 * 
 *
 **********************************************************************/

#include "SeList.h"
#include <stdio.h>
#include <assert.h>

void SeListInit(struct SELIST *root)
{
	root->head = 0;
	root->tail = 0;
}

void SeListInitNode(struct SENODE *node)
{
	node->prev = 0;
	node->next = 0;
}

int SeListNodeInRoot(struct SELIST *root, struct SENODE *node)
{
	struct SENODE *tmp = root->head;
	while (tmp) { if (tmp == node) return 1; tmp = tmp->next; }
	return 0;
}

void SeListHeadAdd(struct SELIST *root, struct SENODE *node)
{
	assert(root && node);
	assert(!node->prev);
	assert(!node->next);
#if defined(_DEBUG)
	if (SeListNodeInRoot(root, node) == 1) assert( 0 != 0);
#endif
	node->prev = 0; node->next = root->head;
	if (root->head) root->head->prev = node;
	root->head = node;
	if (root->tail == 0) root->tail = node;
}

void SeListTailAdd(struct SELIST *root, struct SENODE *node)
{
	assert(root && node);
	assert(!node->prev);
	assert(!node->next);
#if defined(_DEBUG)
	if (SeListNodeInRoot(root, node) == 1) assert( 0 != 0);
#endif
	node->next = 0; node->prev = root->tail;
	if (root->tail) root->tail->next = node;
	root->tail = node;
	if (root->head == 0) root->head = node;
}

void SeListHeadAddList(struct SELIST *root, struct SENODE *node_list)
{
	struct SENODE *node;
	struct SENODE *nodetail;

	nodetail = node_list;
	while (nodetail) { if (nodetail->next) { nodetail = nodetail->next; } else { break; } }
	while (nodetail) { node = nodetail; nodetail = nodetail->prev; SeListInitNode(node); SeListHeadAdd(root, node); }
}

void SeListTailAddList(struct SELIST *root, struct SENODE *node_list)
{
	struct SENODE *node;
	while (node_list) { node = node_list; node_list = node_list->next; SeListInitNode(node); SeListTailAdd(root, node); }
}

struct SENODE *SeListRemove(struct SELIST *root, struct SENODE *node)
{
	assert(root && node);
#if defined(_DEBUG)
	if (SeListNodeInRoot(root, node) == 0) assert( 0 != 0);
#endif
	if (node->prev) node->prev->next = node->next;
	else root->head = node->next;
	if (node->next) node->next->prev = node->prev;
	else root->tail = node->prev;
	node->prev = 0; node->next = 0;
	return node;
}

struct SENODE *SeListRemoveEnd(struct SELIST *root, struct SENODE *node_end)
{
	struct SENODE *result = 0;
#if defined(_DEBUG)
	if (SeListNodeInRoot(root, node_end) == 0) assert( 0 != 0);
#endif
	assert(root && node_end);
	result = root->head;
	if (node_end->next) { root->head = node_end->next; node_end->next->prev = 0; }
	else {root->head = 0; root->tail = 0; }
	node_end->next = 0;
	return result;
}

struct SENODE *SeListHeadPop(struct SELIST *root)
{
	struct SENODE *node = root->head;
	if (node == 0) return 0;
	SeListRemove(root, node);
	return node;
}

struct SENODE *SeListTailPop(struct SELIST *root)
{
	struct SENODE *node = root->tail;
	if (node == 0) return 0;
	SeListRemove(root, node);
	return node;
}
