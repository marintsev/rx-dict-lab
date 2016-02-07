#include "btree_print.h"

void indent(int n) {
	int i;
	for (i = 0; i < n; i++)
		putchar(' ');
}

void btree_subprint(struct node_t * root, int level) {
	if (root == NULL) {
		fprintf(stderr, "[FATAL] Корень дерева == NULL.\n");
		exit(1);
	}

	int size = root->n;
	if (size == 0) {
		printf("Пустое дерево.\n");
	} else {
		// Лист, следовательно указателей нет
		if (root->is_leaf) {
			indent(level);
			printf("Лист размера %d:\n", size);
			int i;
			for (i = 0; i < size; i++) {
				indent(level + 1);
				printf("Ключ: %s\n", root->keys[i]);
			}
		} else {
			indent(level);
			printf("Узел размера %d\n", size);
			if (size == 0) {
				fprintf(stderr, "[FATAL]: size=0\n");
				exit(1);
			}
			btree_subprint(root->pointers[0], level + 2);
			int i;
			for (i = 0; i < size; i++) {
				indent(level + 1);
				printf("Ключ: %s\n", root->keys[i]);
				btree_subprint(root->pointers[i + 1], level + 2);
			}
		}
	}
}

void btree_print(struct node_t * root) {
	btree_subprint(root, 0);
}
