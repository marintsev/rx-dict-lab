#ifndef BTREE_NODE_H
#define BTREE_NODE_H

#include "main.h"

#define CROSS_NOT		0
#define CROSS_BEGIN		1
#define CROSS_MID		2
#define CROSS_END		3
#define CROSS_SKIP		4

extern int btree_node_encode(void * page, struct node_t * node);
extern int btree_node_is_full(struct node_t * x);
extern struct node_t * btree_node_create();
extern void btree_node_allocate(struct node_t * x, int n);

#endif
