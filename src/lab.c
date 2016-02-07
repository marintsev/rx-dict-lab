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
	struct node_t * pointers; // указатели на детей (n+1 штук)
};

/*void btree_print( struct leaf_t * head )
{
	struct leaf_t * p = head->next;
	avl_print1( p );
}*/

struct node_t * root;

/*void leaf_set_key( struct leaf_t * node, char * key )
{
	node->key = malloc(strlen(key) + 1);
	strcpy(node->key, key);
}*/

struct node_t * btree_create()
{
	struct node_t * new = malloc( sizeof( struct node_t ) );
	new->is_leaf = 1;
	new->n = 0;
	new->keys = NULL;
	new->pointers = NULL;
	return new;
}

#define MIN_CHILDREN	4

struct node_t * btree_node_create()
{
	struct node_t * x = malloc( sizeof( struct node_t ));
	x->n = 0;
	x->pointers = NULL;
	x->keys = NULL;
	return x;
}

// выделяет n keys и n+1 pointers
void btree_node_allocate( struct node_t * x, int n )
{
	assert( x->pointers == NULL );
	assert( x->keys == NULL );
	x->n = n;
	x->keys = malloc( sizeof( char* ) * n );
	x->pointers = malloc( sizeof( char* ) * (n+1) );
}

// x -- ссылка на родительский узел
// i -- номер потомка, который заполнен и который надо разделить
void btree_split_child( struct node_t * x, int i )
{
	struct node_t *z,
	              *y;
	z = btree_node_create();
	y = x->pointers[i];
	z.is_leaf = y.is_leaf;
	btree_node_allocate( z, MIN_CHILDREN-1 );

	// x.keys = (k0 k1 k2 ...) (k_i) (k_i+1...)

	int j;
	// копируем в новый узел ключи после k_i:
	// z.keys = (k_{i+1} k_{i+2} ... k_{n-1})
	for( j=0; j<MIN_CHILDREN-1; j++ )
		z->keys[j] = y->keys[j+MIN_CHILDREN];
	// если наш заполненный узел не лист, то копируем и его указатели
	if( !y.is_leaf )
	{
		for( j=0; j<MIN_CHILDREN; j++ )
			z->pointers[j] = y->pointers[j+MIN_CHILDREN];
	}
	// мы отделили от y размером 2*(t-1)+1 узел размером t-1.
	y.n = MIN_CHILDREN-1;

	// сдвигаем в родительском узле указатели правее i на один
	for( j=x->n; j>=i; j-- )
		x->pointers[j+1]=x->pointers[j];

	// бывший указатель по-прежнему указывает на y
	// правее от него будет указывать на z
	x->pointers[i+1] = z;
	// сдвигаем ключи справа от заданного на 1
	for( j=x.n-1; j>=i-1; j--)
		x->keys[j+1]=x->keys[j];
	// устанавливаем между k_i и k_{i+1} бывший центральный ключ в y
	x->keys[i]=y->keys[MIN_CHILDREN];
	// в родительском теперь на одну запись больше
	x->n++;
}

int main(void) {
	root = btree_create();


	//avl_print( &head );

	return 0;
}
