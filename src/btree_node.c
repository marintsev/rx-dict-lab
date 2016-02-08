#include "btree_node.h"

// TODO: уделить больше времени
int btree_node_length( struct node_t * node )
{
	static unsigned char page[8192];

	int length = btree_node_encode( page, node );
	return length;
}

// TODO: length
int btree_node_encode(void * page, struct node_t * node) {
	int is_leaf = (node->is_leaf) ? 1 : 0;
	((unsigned char*) page)[0] = is_leaf;

	int count = node->n;
	assert(count < 256);
	((unsigned char*) page)[1] = count;

	int offset = 2;
	unsigned char * p1 = page + offset;
	if (is_leaf) {
		int i;
		for (i = 0; i < node->n; i++) {
			// 8 байт указатель на данные
			((u_int64_t*) p1)[0] = 0x5555555555555555;
			offset += 8;
			p1 += 8;
			assert(strlen(node->keys[i]) <= 128);
			strcpy(p1, node->keys[i]);
			offset += 128;
			p1 += 128;
		}
	} else {
		int i;
		for (i = 0; i < node->n; i++) {
			// 8 байт указатель на данные
			((u_int64_t*) p1)[0] = 0xDEADBEEFDEADBEEF;
			offset += 8;
			p1 += 8;
			assert(strlen(node->keys[i]) <= 128);
			strcpy(p1, node->keys[i]);
			offset += 128;
			p1 += 128;
		}
		((u_int64_t*) p1)[0] = 0xBEEFDEADBEEFDEAD;
		offset += 8;
		p1 += 8;
	}
	return offset;
}

int btree_node_is_full(struct node_t * x) {
	int len = btree_node_length(x);
	fprintf( stderr, "len: %d %d.\n", len, 4096 - len < 8+128 );
	if( x->is_leaf )
	{
		return 4096 - len < 8+128;
	}
	else
	{
		return 4096 - len < 8+128;
	}
	//return (x->n == 2 * MIN_CHILDREN - 1);
}
