/*
 ============================================================================
 Name        : lab.c
 Author      : Myron Marintsev
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "main.h"

#include "btree_print.h"
#include "btree.h"

struct node_t * root;

struct raw_page_t {
	void * data;
	int size;
};

struct raw_page_t * raw_page_allocate(int size) {
	struct raw_page_t * page = malloc(sizeof(struct raw_page_t));
	page->data = malloc(size);
	page->size = size;
	return page;
}

void raw_page_free(struct raw_page_t * page) {
	free(page->data);
	free(page);
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
			offset += 8;
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
			offset += 8;
			p1 += 128;
		}
		((u_int64_t*) p1)[0] = 0xBEEFDEADBEEFDEAD;
		offset += 8;
		p1 += 8;
	}
	return offset;
}

void btree_write_helper(struct node_t * root, void * page, int page_index,
		FILE * f) {
	if (root == NULL)
		return;
	btree_node_encode(page, root);
	int length = btree_node_encode(page, root);
	fprintf( stderr, "Длина узла: %d.\n", length);
	fwrite(page, 4096, 1, f);
	if (!root->is_leaf) {
		int i;
		for (i = 0; i <= root->n; i++) {
			btree_write_helper(root->pointers[i], page, page_index + 1, f);
		}
	}
}

void btree_write(struct node_t * root, FILE * f) {
	void * page = malloc(4096);
	btree_write_helper(root, page, 0, f);
}

int main(void) {
	root = btree_create();

	btree_print(root);

	if (root == NULL) {
		fprintf( stderr, "main: root=NULL");
		exit(1);
	}
	int i;

	btree_insert(&root, "01_first");
	btree_insert(&root, "02_second");
	btree_insert(&root, "03_third");
	btree_insert(&root, "04_fourth");
	btree_insert(&root, "05_fifth");
	btree_insert(&root, "06_sixth");
	btree_insert(&root, "07_seventh");
	btree_insert(&root, "08_eighth");

	/*	btree_delete(&root, "04_fourth");
	 btree_insert(&root, "04_fourth");
	 btree_delete(&root, "04_fourth");
	 fprintf( stderr, "[MAIN]: dlete_type: %d\n",
	 btree_delete(&root, "03_third"));*/

//btree_delete(root, "04_fourth");
	/*btree_insert(&root, "09_nineth");
	 btree_insert(&root, "10_tenth");
	 btree_insert(&root, "11_eleventh");
	 btree_insert(&root, "12_twelfth");*/

	btree_print(root);

	FILE * f = fopen("btree.db", "w+");
	btree_write( root, f );
	fclose(f);

	fflush( stdout);

	return 0;
}
