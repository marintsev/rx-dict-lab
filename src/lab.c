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

struct leaf_t {
	char * key;
	char * content;
	struct leaf_t * prev;
	struct leaf_t * next;
	unsigned char balanced;
};

struct leaf_t * avl_insert(struct leaf_t * head, char * k) {
	struct leaf_t *t, *s, *p, *q, *r;
	int a;

	t = head;
	s = p = head->next;
	int cmp_res = strcmp(k, p->key);
	a2: if (cmp_res == -1)
		goto a3;
	else if (cmp_res == 1)
		goto a4;
	else
		return p; // успешное завершение
	a3: q = p->prev;
	if (q == NULL) {
		q = malloc(sizeof(struct leaf_t));
		p->prev = q;
		goto a5;
	} else if (q->balanced != 0) {
		t = p;
		s = q;
		p = q;
		goto a2;
	}
	a4: q = p->next;
	if (q == NULL) {
		q = malloc(sizeof(struct leaf_t));
		p->next = q;
		goto a5;
	} else if (q->balanced != 0) {
		t = p;
		s = q;
		p = q;
		goto a2;
	}
	a5: q->key = malloc(strlen(k) + 1);
	strcpy(q->key, k);
	q->prev = q->next = NULL;
	q->balanced = 0;
	a6: cmp_res = strcmp(k, s->key);
	if (cmp_res == -1) {
		a = -1;
		r = p = s->prev;
	} else {
		a = 1;
		r = p = s->next;
	}

	while (p != q) {
		cmp_res = strcmp(k, p->key);
		if (cmp_res == -1) {
			p->balanced = -1;
			p = p->prev;
		} else if (cmp_res == 1) {
			p->balanced = 1;
			p = p->next;
		}
	}
	a7: if (s->balanced == 0) {
		s->balanced = a;
		head->prev++;
		goto stop;
	} else if (s->balanced == -a) {
		s->balanced = 0;
		goto stop;
	} else if (s->balanced == a) {
		if (r->balanced == a)
			goto a8;
		if (r->balanced == -a)
			goto a9;
	}
	a8: p = r;
	if (a == -1) {
		s->prev = r->next;
		r->next = s;
	} else if (a == 1) {
		s->next = r->prev;
		r->prev = s;
	}
	s->balanced = r->balanced = 0;
	goto a10;
	a9: if (a == -1) {
		p = r->next;
		r->next = p->prev;
		p->prev = r;
		s->prev = p->next;
		p->next = s;
	} else if (a == 1) {
		p = r->prev;
		r->prev = p->next;
		p->next = r;
		s->next = p->prev;
		p->prev = s;
	}
	if (p->balanced == a) {
		s->balanced = -a;
		r->balanced = 0;
	} else if (p->balanced == 0) {
		s->balanced = r->balanced = 0;
	} else if (p->balanced == -a) {
		s->balanced = 0;
		r->balanced = a;
	}
	p->balanced = 0;
	a10: if (s == t->next)
		t->next = p;
	else
		t->prev = p;
stop:
	return p;
}

void avl_print2( struct leaf_t * root )
{
	printf( "key: %s", root->key );
}

void avl_print1( struct leaf_t * root )
{
	if( root->prev )
		avl_print1( root->prev );
	avl_print2( root );
	if( root->next )
		avl_print1( root->next );
}

void avl_print( struct leaf_t * head )
{
	struct leaf_t * p = head->next;
	avl_print1( p );
}

struct leaf_t head;
struct leaf_t root;

void leaf_set_key( struct leaf_t * node, char * key )
{
	node->key = malloc(strlen(key) + 1);
	strcpy(node->key, key);
}

int main(void) {
	root.prev = NULL;
	root.next = NULL;
	//leaf_set_key( &root, "first" );
	root.content = "";
	root.balanced = 0;

	head.prev = 0;
	head.next = NULL;root;

	struct leaf_t * new = avl_insert( &head, "second");
	new = avl_insert( &head, "third");
	//leaf_set_key( new, "second");


	avl_print( &head );

	return 0;
}
