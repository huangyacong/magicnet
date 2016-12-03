/**********************************************************************
 *
 * selist.h
 *
 * NOTE: This is a C version list operation wrapper
 * 
 *
 **********************************************************************/

#ifndef __SE_LIST_H__
#define __SE_LIST_H__

#ifdef	__cplusplus
extern "C" {
#endif

/**********************************************************************
 * Define a node in list                                              *
 **********************************************************************/
struct SENODE
{
	struct SENODE *prev;
	struct SENODE *next;
};

/**********************************************************************
 * Define the hole list                                               *
 **********************************************************************/
struct SELIST
{
	struct SENODE *head;
	struct SENODE *tail;
};

/**********************************************************************
 * list  function                                                     *
 **********************************************************************/

void SeListInit(struct SELIST *root);

void SeListInitNode(struct SENODE *node);

void SeListHeadAdd(struct SELIST *root, struct SENODE *node);

void SeListTailAdd(struct SELIST *root, struct SENODE *node);

void SeListHeadAddList(struct SELIST *root, struct SENODE *node_list);

void SeListTailAddList(struct SELIST *root, struct SENODE *node_list);

struct SENODE *SeListRemove(struct SELIST *root, struct SENODE *node);

struct SENODE *SeListRemoveEnd(struct SELIST *root, struct SENODE *node_end);

struct SENODE *SeListHeadPop(struct SELIST *root);

struct SENODE *SeListTailPop(struct SELIST *root);

#ifdef	__cplusplus
}
#endif

#endif


