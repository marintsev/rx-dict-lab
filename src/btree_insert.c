#include "btree_insert.h"
#include "btree_node.h"

void btree_insert_nonfull(struct node_t * x, char * k) {
	if (x == NULL) {
		fprintf( stderr, "[FATAL]: x == NULL.\n");
		exit(1);
	}

	int i = x->n - 1;
	if (x->is_leaf) {
		//fprintf( stderr, "[INFO]: x is leaf.\n" );
		for (; i >= 0; ) {
			if (strcmp(k, x->keys[i]) < 0)
			{
				x->keys[i + 1] = x->keys[i]; // !
				i--;
			}
			else
				break;
		}
		//x->keys[i + 1] = k; // !
		/*char str[256];
		sprintf(str,"%s*",k);*/
		btree_node_load_key(x, i + 1, k);
		x->n++;
		//
	} else {
		for (; i >= 0; ) {
			if (strcmp(k, x->keys[i]) < 0)
				i--;
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
