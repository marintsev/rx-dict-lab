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

struct node_t * root;

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

void btree_node_free(struct node_t * x) {
	assert(x->pointers != NULL);
	assert(x->keys != NULL);
	assert(x->is_leaf || x->n == 0);

	free(x->pointers);
	int i;
	for (i = 0; i < x->n; i++) {
		free(x->keys[i]);
	}
	x->n = 0;
	free(x->keys);
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
	x->keys[i] = y->keys[MIN_CHILDREN - 1]; // !!!
	// в родительском теперь на одну запись больше
	x->n++;
}

int btree_node_is_full(struct node_t * x) {
	return (x->n == 2 * MIN_CHILDREN - 1);
}

void btree_node_load_key(struct node_t * x, int i, char * k) {
	x->keys[i] = malloc(strlen(k) + 1);
	strcpy(x->keys[i], k);
}

char * btree_node_save_key(struct node_t * x, int i) {
	char * key = x->keys[i];
	char * result = malloc(strlen(key) + 1);
	strcpy(result, key);
	return result;
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
		//x->keys[i + 1] = k; // !
		btree_node_load_key(x, i + 1, k);
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

int btree_delete(struct node_t ** root, char * k) {
	return btree_delete_detail(root, *root, k);
}

int btree_delete_detail(struct node_t ** root, struct node_t * x, char * k) {
	// если ключ k находится в узле x и x является листом -- удаляем ключ k из x.
	if (x->is_leaf) {
		int i;
		for (i = 0; i < x->n; i++) {
			int cmp_res = strcmp(k, x->keys[i]);
			if (cmp_res < 0)
				break;
			if (cmp_res == 0) {
				// удаляем
				if (x->keys[i] == NULL) {
					fprintf( stderr, "[FATAL]: x->keys[i] == NULL.");
					exit(1);
				}
				free(x->keys[i]);

				int j;
				for (j = i + 1; j < x->n; j++)
					x->keys[j - 1] = x->keys[j];
				x->n--;
				return 0;
				//break; // будем надеяться, что он такой один
			}
		}
	} else {
		int i;
		int found_index = -1;
		for (i = 0; i < x->n; i++) {
			if (strcmp(k, x->keys[i]) == 0) {
				found_index = i;
				break;
			}
		}
		if (found_index >= 0) {
			fprintf(stderr, "[INFO]: btree_delete: ключ во внутреннем узле.\n");
			struct node_t * y = x->pointers[found_index];
			if (y->n >= MIN_CHILDREN) {
				fprintf(stderr,
						"[TODO]: btree_delete: узел слева достаточно большой.\n");
				exit(1);
			} else {
				struct node_t * z = x->pointers[found_index + 1];
				if (z->n >= MIN_CHILDREN) {
					fprintf(stderr,
							"[INFO]: btree_delete: узел справа достаточно большой. Удаляем в нём.\n");
					char * k1 = btree_node_save_key(z, 0);
					fprintf(stderr, "Удаление вида: %d %s\n",
							btree_delete_detail(root, z, k1), k1);
					btree_node_load_key(x, found_index, k1);
					return 1;
				} else {
					fprintf(stderr,
							"[INFO]: btree_delete: ключи слева и справа можно объединить после удаления.\n");
					// внести k в y
					btree_node_load_key(y, y->n, k);
					y->n++;

					// внести все ключи и указатели z в y
					int i;
					for (i = 0; i < z->n; i++) {
						// TODO: усложнить
						char * kx = btree_node_save_key(z, i);
						btree_node_load_key(y, y->n + i, kx);
					}
					for (i = 0; i <= z->n; i++) {
						y->pointers[y->n + i] = z->pointers[i];
					}
					y->n += z->n;

					// удалить k из x
					for (i = found_index; i < x->n; i++) {
						x->keys[i] = x->keys[i + 1];
					}
					// удалить указатель на z из x
					for (i = found_index; i <= x->n; i++) {
						x->pointers[i] = x->pointers[i + 1];
					}
					x->n--;
					// освободить z
					btree_node_free(z); //функция должна знать z.n, поэтому мы его не меняли

					// если в x ничего не осталось, то y -- новый корень
					if (x->n == 0) {
						*root = y;
						btree_node_free(x);
					}

					/*printf("До удаления");
					 btree_print( *root );*/
					// удалить k из y
					int delete_code = btree_delete_detail(root, y, k);
					/*printf("После удаления");
					 btree_print( *root );*/
					printf("Удаление вида: %d %s\n", delete_code, k);
					return 2;
				}
			}
		} else {
			// находим корень x.c_i поддерева, которое должно содержать k
			int delete_type = 4;

			assert(x->n != 0);
			int i = 0;
			int found_index = -1;
			while (1) {
				int cmp_res = strcmp(k, x->keys[i]);
				if (cmp_res == -1) {
					found_index = i;
					break;
				} else if (cmp_res == 0) {
					assert(0); // не должно быть равных ключей
				} else if (cmp_res == 1) {
					i++;
					if (i == x->n) {
						found_index = i;
						break;
					}
				}
			}
			fprintf(stderr, "%s: Наден указатель %d,\n", k, found_index);
			// теперь found_index содержит индекс x->pointers поддерева, содержащего k
			assert(found_index != -1);
			struct node_t * v = x->pointers[found_index];
			fprintf(stderr, "%s: Наден указатель %d, размер: %d.\n", k,
					found_index, v->n);
			fflush(stderr);
			// если x.c_i содержит только t-1 ключей, то
			if (v->n == MIN_CHILDREN - 1) {
				// а
				struct node_t * left = NULL;
				if (found_index != 0)
					left = x->pointers[found_index - 1];
				struct node_t * right = NULL;
				if (found_index != x->n)
					right = x->pointers[found_index + 1];

				// если левый сосед имеет не менее t ключей
				if (left != NULL && left->n >= MIN_CHILDREN) {
					fprintf(stderr, "[TODO]: btree_delete: (t) [t-1] ?.\n");
					exit(1);
				} else if (right != NULL && right->n >= MIN_CHILDREN) {
					// если правый сосед имеет не менее t ключей
					fprintf(stderr, "[TODO]: btree_delete: ? [t-1] (t).\n");
					exit(1);
				} else if (left == NULL && right == NULL) {
					fprintf(stderr, "[FATAL]: btree_delete: left==right==NULL\n");
					exit(1);
				} else if ((left == NULL || left->n == MIN_CHILDREN - 1)
						&& (right == NULL || right->n == MIN_CHILDREN - 1)) {
					// один или оба соседа имеют t-1
					fprintf(stderr,
							"[TODO]: btree_delete: (t-1)? [t-1] (t-1)?.\n");

					// левый
					if (left != NULL && right == NULL) {
						fprintf(stderr, "[INFO]: Объединяем (t-1) [t-1].\n");
						// левый узел содержит t-1 записей, текущий t-1, правого не существует
						// сдвинем всё на t вправо в текущем узле
						int i;
						for (i = 0; i < MIN_CHILDREN - 1; i++)
							v->keys[i + MIN_CHILDREN] = v->keys[i];
						for (i = 0; i < MIN_CHILDREN; i++)
							v->pointers[i + MIN_CHILDREN] = v->pointers[i];
						v->keys[MIN_CHILDREN - 1] = x->keys[found_index - 1];
						for (i = 0; i < MIN_CHILDREN - 1; i++)
							v->keys[i] = left->keys[i];
						for (i = 0; i < MIN_CHILDREN; i++)
							v->pointers[i] = left->pointers[i];
						v->n += 1 + left->n;
						left->n = 0;
						btree_node_free(left);
						x->n--;
						// если в x ничего не осталось, то y -- новый корень
						if (x->n == 0) {
							*root = v;
							btree_node_free(x);
						}
						delete_type = 5;
						fprintf( stderr,
								"[INFO]. Объединив левый (t-1) и текущий (t-1) удаляем как (%d).\n",
								btree_delete_detail(root, v, k));
						return delete_type;
					} else if (left == NULL && right != NULL) {
						fprintf(stderr, "[TODO]: btree_delete: правый.\n");
						exit(1);
					} else if (left != NULL && right != NULL) {
						fprintf(stderr, "[TODO]: btree_delete: оба.\n");
						exit(1);
					}
					assert(0);

					return -1;
				} else {
					fprintf(stderr, "[TODO]: btree_delete: ? [t-1] ?.\n");
					exit(1);
				}
			} else {
				// узел достаточно большой: ничего делать не надо
			}
			// рекурсивно удаляем k из соответствующего дочернего узла
			fprintf( stderr,
					"[INFO]. Удел достаточно большой. Просто удаляем (%d).\n",
					btree_delete_detail(root, v, k));
			return delete_type;
		}

		fprintf(stderr, "[TODO]: btree_delete: !is_leaf.\n");
		exit(1);
	}
}

int main(void) {
	root = btree_create();

	btree_print(root);

	if (root == NULL) {
		fprintf( stderr, "main: root=NULL");
		exit(1);
	}
	int i;
	/*char str[256];
	 for(i=0;i<100;i++)
	 {
	 sprintf( str, "%02d", i);
	 btree_insert(&root, str);
	 }*/
	btree_insert(&root, "01_first");
	btree_insert(&root, "02_second");
	btree_insert(&root, "03_third");
	btree_insert(&root, "04_fourth");
	btree_insert(&root, "05_fifth");
	btree_insert(&root, "06_sixth");
	btree_insert(&root, "07_seventh");
	btree_insert(&root, "08_eighth");

	btree_delete(&root, "04_fourth");
	btree_insert(&root, "04_fourth");
	btree_delete(&root, "04_fourth");
	btree_delete(&root, "03_third");

//btree_delete(root, "04_fourth");
	/*btree_insert(&root, "09_nineth");
	 btree_insert(&root, "10_tenth");
	 btree_insert(&root, "11_eleventh");
	 btree_insert(&root, "12_twelfth");*/

	btree_print(root);

	fflush( stdout);

	return 0;
}
