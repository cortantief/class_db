#ifndef TABLE_H
#define TABLE_H

#include "btree.h"

typedef struct {
	char* name;
	db_col* cols;
	size_t col_size;
	size_t row_size;
	btree_node* root;
} db_table;

#endif
