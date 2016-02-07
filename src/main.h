#ifndef MAIN_H
#define MAIN_H

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

#endif
