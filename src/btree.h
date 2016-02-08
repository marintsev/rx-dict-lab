#include "main.h"

#define MIN_CHILDREN	4

extern struct node_t * btree_create();
extern void btree_insert(struct node_t ** t, char * k);
extern int btree_delete(struct node_t ** root, char * k);

