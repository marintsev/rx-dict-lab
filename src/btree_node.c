#include "btree_node.h"

// пересекает ли отрезок размером length с началом в here границу border?
int is_cross(int here, int length, int border) {
	if (here < border) {
		if (here < border - length) {
			return CROSS_NOT; // не пересекает
		} else if (here == border - length) {
			return CROSS_END; // концом
		} else if (here > border - length) {
			return CROSS_MID; // середина
		}
	} else if (here == border) {
		return CROSS_BEGIN; // начало
	} else if (here > border) {
		return CROSS_SKIP; // заведомо после
	}
}

int btree_node_leaf_who_at_middle(struct node_t * x) {
	assert(x->is_leaf);
	int offset = 2;
	int i = 0;
	again: if (i == x->n) {
		return -1;
	} else {
		int res = is_cross(offset, 8, 2048);
		if (res != CROSS_NOT)
			return i;
		offset += 8;
		int length = strlen(x->keys[i])+1;
		res = is_cross(offset, length, 2048);
		if (res != CROSS_NOT && res != CROSS_END)
			return i;
		offset += length;
		i++;
		goto again;
	}
}

int btree_node_node_who_at_middle(struct node_t * x) {
	assert(!(x->is_leaf));
	int offset = 2;
	int i = 0;
	again: if (i == x->n) {
		int res = is_cross(offset, 8, 2048);
		if (res != CROSS_NOT && res != CROSS_END)
			return i;
		return -1;
	} else {
		int res = is_cross(offset, 8, 2048);
		if (res != CROSS_NOT)
			return i;
		offset += 8;
		int length = strlen(x->keys[i])+1;
		res = is_cross(offset, length, 2048);
		if (res != CROSS_NOT && res != CROSS_END)
			return i;
		offset += length;
		i++;
		goto again;
	}
}

int btree_node_who_at_middle(struct node_t * x) {
	if (x->is_leaf)
		return btree_node_leaf_who_at_middle(x);
	else
		return btree_node_node_who_at_middle(x);
}

// TODO: уделить больше времени
int btree_node_length(struct node_t * node) {
	static unsigned char page[8192];

	int length = btree_node_encode(page, node);
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
			int length = strlen(node->keys[i]);
			assert(length <= 128);
			p1[0] = length;
			offset++;
			p1++;
			strncpy(p1, node->keys[i], length);
			offset += length;
			p1 += length;
		}
	} else {
		int i;
		for (i = 0; i < node->n; i++) {
			// 8 байт указатель на данные
			((u_int64_t*) p1)[0] = 0xDEADBEEFDEADBEEF;
			offset += 8;
			p1 += 8;
			int length = strlen(node->keys[i]);
			assert(length <= 128);
			p1[0] = length;
			offset++;
			p1++;
			strncpy(p1, node->keys[i], length);
			offset += length;
			p1 += length;
		}
		((u_int64_t*) p1)[0] = 0xBEEFDEADBEEFDEAD;
		offset += 8;
		p1 += 8;
	}
	return offset;
}

int btree_node_is_full(struct node_t * x) {
	int len = btree_node_length(x);
	//fprintf( stderr, "len: %d %d.\n", len, 4096 - len < 8 + 128);
	if (x->is_leaf) {
		return 4096 - len < 8 + 128;
	} else {
		return 4096 - len < 8 + 128;
	}
	//return (x->n == 2 * MIN_CHILDREN - 1);
}
