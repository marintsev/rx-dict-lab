#include "btree.h"
#include "btree_node.h"

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
	free(x);
}

// x -- ссылка на родительский узел
// i -- номер потомка, который заполнен и который надо разделить
void btree_split_child(struct node_t * x, int i) {
	struct node_t *z, *y;
	z = btree_node_create();
	y = x->pointers[i];
	z->is_leaf = y->is_leaf;

	// x.keys = (k0 k1 k2 ...) (k_i) (k_i+1...)

	int j;
	// копируем в новый узел ключи после k_i:
	// z.keys = (k_{i+1} k_{i+2} ... k_{n-1})
	int central = btree_node_who_at_middle(y);
	//fprintf(stderr, "central: %d.\n", central);
	assert(central != -1);

	btree_node_allocate(z, y->n - central - 1);

	for (j = 0; j < y->n - central - 1; j++)
		z->keys[j] = y->keys[j + central + 1]; // !!!
	// если наш заполненный узел не лист, то копируем и его указатели
	if (!y->is_leaf) {
		for (j = 0; j < y->n - central; j++)
			z->pointers[j] = y->pointers[j + central + 1];
	}
	// мы отделили от y размером 2*(t-1)+1 узел размером t-1.
	y->n = central;

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
	x->keys[i] = y->keys[central]; // !!!
	// в родительском теперь на одну запись больше
	x->n++;
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

//  0 -- ключ был и мы его удалили
// -1 -- ключа не было
int btree_delete_from_leaf(struct node_t * x, char * k) {
	int i;
	for (i = 0; i < x->n; i++) {
		int cmp_res = strcmp(k, x->keys[i]);
		if (cmp_res < 0) // мы либо пропустили ключ, либо его здесь никогда не было
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
			//break; // будем считать, пока, что за раз удаляем только один такой ключ
		}
	}
	return -1;
}

int btree_find_key(struct node_t *x, char *k) {
	int i;
	int found_index = -1;
	for (i = 0; i < x->n; i++) {
		if (strcmp(k, x->keys[i]) == 0) {
			found_index = i;
			break;
		}
	}
	return found_index;
}

int btree_delete_detail(struct node_t ** root, struct node_t * x, char * k);

int btree_delete_from_this_node(struct node_t ** root, struct node_t * x,
		int found_index, char * k) {
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
}

int btree_find_where_is_key(struct node_t * x, char * k) {
	assert(x->n != 0);
	int i = 0;
	int found_index = -1;
	fprintf( stderr, "x->n=%d", x->n);
	while (1) {
		int cmp_res = strcmp(k, x->keys[i]);
		if (cmp_res < 0) {
			found_index = i;
			break;
		} else if (cmp_res == 0) {
			assert(0); // не должно быть равных ключей
		} else if (cmp_res > 0) {
			i++;
			if (i == x->n) {
				found_index = i;
				break;
			}
		}
	}
	fprintf( stderr, "found_index=%d", found_index);
	return found_index;
}

// левый удаляется
struct node_t * btree_delete_collect_with_left(struct node_t **root,
		struct node_t * left, struct node_t * parent, struct node_t * right,
		int found_index) {
	struct node_t * v = right;
	struct node_t * x = parent;

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
	return v;
}

// правый удаляется
struct node_t * btree_delete_collect_with_right(struct node_t **root,
		struct node_t * current, struct node_t * parent, struct node_t * right,
		int found_index) {
	struct node_t * v = current;
	struct node_t * x = parent;

	int i;
	v->keys[MIN_CHILDREN - 1] = x->keys[found_index];
	for (i = 0; i < MIN_CHILDREN - 1; i++) {
		v->keys[i + MIN_CHILDREN] = right->keys[i];
	}
	for (i = 0; i < MIN_CHILDREN; i++)
		v->pointers[i + MIN_CHILDREN] = right->pointers[i];
	v->n += 1 + right->n;
	right->n = 0;
	btree_node_free(right);
	x->n--;
	// если в x ничего не осталось, то y -- новый корень
	if (x->n == 0) {
		*root = v;
		btree_node_free(x);
	}
	return v;
}

int btree_delete_from_subnode(struct node_t ** root, struct node_t * x,
		char * k) {
	// находим корень x.c_i поддерева, которое должно содержать k
	int delete_type = 4;
	int found_index = btree_find_where_is_key(x, k);
	if (found_index == -1) {
		fprintf(stderr, "[FATAL]. Указатель не найден.");
		exit(2);
	}
	fprintf(stderr, "%s: Наден указатель %d,\n", k, found_index);
	fflush(stderr);
	// теперь found_index содержит индекс x->pointers поддерева, содержащего k
	assert(found_index != -1);
	struct node_t * v = x->pointers[found_index];
	fprintf(stderr, "%s: Наден указатель %d, размер: %d.\n", k, found_index,
			v->n);
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
			fprintf(stderr, "[TODO]: btree_delete: (t-1)? [t-1] (t-1)?.\n");

			// левый
			if (left != NULL && right == NULL) {
				fprintf(stderr, "[INFO]: Объединяем (t-1) [t-1].\n");
				// левый узел содержит t-1 записей, текущий t-1, правого не существует
				// сдвинем всё на t вправо в текущем узле
				v = btree_delete_collect_with_left(root, left, x, v,
						found_index);
				delete_type = 5;
				fprintf( stderr,
						"[INFO]. Объединив левый (t-1) и текущий (t-1) удаляем как (%d).\n",
						btree_delete_detail(root, v, k));
			} else if (left == NULL && right != NULL) {
				fprintf(stderr, "[INFO]: btree_delete: правый.\n");
				v = btree_delete_collect_with_right(root, v, x, right,
						found_index);
				delete_type = 6;
				//btree_print(*root);
				fprintf( stderr,
						"[INFO]. Объединив правый (t-1) и текущий (t-1) удаляем как (%d).\n",
						btree_delete_detail(root, v, k));
			} else if (left != NULL && right != NULL) {
				fprintf(stderr, "[INFO]: btree_delete: оба.\n");
				v = btree_delete_collect_with_left(root, left, x, v,
						found_index);
				delete_type = 7;
				fprintf( stderr,
						"[INFO]. Объединив левый из двух (t-1) и текущий (t-1) удаляем как (%d).\n",
						btree_delete_detail(root, v, k));
			} else {
				assert(0);
			}
			fprintf(stderr, "[INFO]: (t-1) (t-1) завершен.");
			return delete_type;
		} else {
			fprintf(stderr, "[TODO]: btree_delete: ? [t-1] ?.\n");
			exit(1);
		}
	} else {
		// узел достаточно большой: ничего делать не надо
	}
	// рекурсивно удаляем k из соответствующего дочернего узла
	fprintf( stderr, "[INFO]. Удел достаточно большой. Просто удаляем (%d).\n",
			btree_delete_detail(root, v, k));
	return delete_type;
}

int btree_delete_detail(struct node_t ** root, struct node_t * x, char * k) {
	// если ключ k находится в узле x и x является листом -- удаляем ключ k из x.
	if (x->is_leaf) {
		return btree_delete_from_leaf(x, k);
	} else {
		fprintf(stderr, "[INFO]: не узел.\n");
		int found_index = btree_find_key(x, k);
		if (found_index >= 0) {
			fprintf(stderr, "[INFO]: ключ найден в узле (%d).\n", found_index);
			return btree_delete_from_this_node(root, x, found_index, k);
		} else {
			fprintf(stderr, "[INFO]: ключ не найден в узле.\n");
			return btree_delete_from_subnode(root, x, k);
		}
	}
}

int btree_delete(struct node_t ** root, char * k) {
	return btree_delete_detail(root, *root, k);
}
