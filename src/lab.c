/*
 ============================================================================
 Name        : lab.c
 Author      : Myron Marintsev
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>

struct node_t {
	int n; // количество ключей
	char ** keys; // ключи (n штук)
	int is_leaf; // является ли листом?
	struct node_t ** pointers; // указатели на детей (n+1 штук)
};

struct node_t * root;

/*void leaf_set_key( struct leaf_t * node, char * key )
 {
 node->key = malloc(strlen(key) + 1);
 strcpy(node->key, key);
 }*/

struct node_t * btree_node_create() {
	struct node_t * x = malloc(sizeof(struct node_t));
	x->n = 0;
	x->pointers = NULL;
	x->keys = NULL;
	return x;
}

#define MIN_CHILDREN	4

// выделяет n keys и n+1 pointers
void btree_node_allocate(struct node_t * x, int n) {
	assert(x->pointers == NULL);
	assert(x->keys == NULL);
	x->n = n;
	int max_n = 2 * MIN_CHILDREN - 1;
	x->keys = malloc(sizeof(char*) * max_n);
	x->pointers = malloc(sizeof(char*) * (max_n + 1));
}

struct node_t * btree_create() {
	struct node_t * new = btree_node_create();
	new->is_leaf = 1;
	btree_node_allocate(new, 0);
	return new;
}

// x -- ссылка на родительский узел
// i -- номер потомка, который заполнен и который надо разделить
void btree_split_child(struct node_t * x, int i) {
	struct node_t *z, *y;
	z = btree_node_create();
	y = x->pointers[i];
	z->is_leaf = y->is_leaf;
	btree_node_allocate(z, MIN_CHILDREN - 1);

	// x.keys = (k0 k1 k2 ...) (k_i) (k_i+1...)

	int j;
	// копируем в новый узел ключи после k_i:
	// z.keys = (k_{i+1} k_{i+2} ... k_{n-1})
	for (j = 0; j < MIN_CHILDREN - 1; j++)
		z->keys[j] = y->keys[j + MIN_CHILDREN]; // !!!
	// если наш заполненный узел не лист, то копируем и его указатели
	if (!y->is_leaf) {
		for (j = 0; j < MIN_CHILDREN; j++)
			z->pointers[j] = y->pointers[j + MIN_CHILDREN];
	}
	// мы отделили от y размером 2*(t-1)+1 узел размером t-1.
	y->n = MIN_CHILDREN - 1;

	// сдвигаем в родительском узле указатели правее i на один
	for (j = x->n; j >= i; j--)
		x->pointers[j + 1] = x->pointers[j];

	// бывший указатель по-прежнему указывает на y
	// правее от него будет указывать на z
	x->pointers[i + 1] = z;
	// сдвигаем ключи справа от заданного на 1
	for (j = x->n - 1; j >= i - 1; j--)
		x->keys[j + 1] = x->keys[j]; // !!!
	// устанавливаем между k_i и k_{i+1} бывший центральный ключ в y
	x->keys[i] = y->keys[MIN_CHILDREN-1]; // !!!
	// в родительском теперь на одну запись больше
	x->n++;
}

int btree_node_is_full(struct node_t * x) {
	return (x->n == 2 * MIN_CHILDREN - 1);
}

void btree_insert_nonfull(struct node_t * x, char * k) {
	if (x == NULL) {
		fprintf( stderr, "[FATAL]: x == NULL.\n");
		exit(1);
	}

	int i = x->n - 1;
	if (x->is_leaf) {
		//fprintf( stderr, "[INFO]: x is leaf.\n" );
		for (; i >= 0; i--) {
			if (strcmp(k, x->keys[i]) < 0)
				x->keys[i + 1] = x->keys[i]; // !
			else
				break;
		}
		x->keys[i + 1] = k; // !
		x->n++;
		//
	} else {
		for (; i >= 0; i--) {
			if (strcmp(k, x->keys[i]) < 0)
				x->keys[i + 1] = x->keys[i]; // !
			else
				break;
		}
		i++;
		if (btree_node_is_full(x->pointers[i])) {
			btree_split_child(x, i);
			if (strcmp(k, x->keys[i]) > 0)
				i++;
		}
		btree_insert_nonfull(x->pointers[i], k);
	}
}

void btree_insert(struct node_t ** t, char * k) {
	struct node_t *r, *s;

	if (t == NULL) {
		fprintf( stderr, "btree_insert: t == NULL.\n");
		exit(1);
	}

	r = *t;

	if (r == NULL) {
		fprintf( stderr, "btree_insert: r == NULL.\n");
		exit(1);
	}

	// корень заполнен?
	if (btree_node_is_full(r)) {
		//fprintf( stderr, "[INFO] btree_node_is_full(r).\n" );
		s = btree_node_create();
		*t = s;
		s->is_leaf = 0;
		btree_node_allocate(s, 0);
		s->pointers[0] = r;
		btree_split_child(s, 0);
		btree_insert_nonfull(s, k);
	} else {
		//fprintf( stderr, "[INFO] !btree_node_is_full(r).\n" );
		btree_insert_nonfull(r, k);
	}
}

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

int main(void) {
	root = btree_create();

	btree_print(root);

	if (root == NULL) {
		fprintf( stderr, "main: root=NULL");
		exit(1);
	}
	btree_insert(&root, "01_first");
	btree_insert(&root, "02_second");
	btree_insert(&root, "03_third");
	btree_insert(&root, "04_fourth");
	btree_insert(&root, "05_fifth");
	btree_insert(&root, "06_sixth");
	btree_insert(&root, "07_seventh");
	btree_insert(&root, "08_eighth");
	btree_insert(&root, "09_nineth");
	btree_insert(&root, "10_tenth");
	btree_insert(&root, "11_eleventh");
	btree_insert(&root, "12_twelfth");

	btree_print(root);

	fflush( stdout);

	return 0;
}
