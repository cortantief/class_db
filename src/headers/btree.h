#ifndef BTREE_H
#define BTREE_H

#include <stdint.h>

typedef struct {
	size_t* keys;
	btree_node** child;
	bool leaf;
	size_t size;
} btree_node;


#endif
