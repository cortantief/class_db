#include <stdlib.h>
#include <string.h>

#include "btree.h"
#include "table.h"

db_table *new_table(char *name) {
    db_table *table;

    if (name == NULL)
	return NULL;

    table = malloc(sizeof(db_table));
    if (table == NULL)
	return NULL;
    table->row_size = 0;
    table->col_size = 0;
    table->root = NULL;
    table->name = strdup(name);
    table->cols = NULL;
    if (table->name == NULL) {
	free(table);
	return NULL;
    }
    return table;
}

db_col *new_col(char *name, enum coltype type) {
    db_col *c;

    c = malloc(sizeof(db_col));
    if (c == NULL)
	return NULL;
    c->name = strdup(name);
    if (c->name == NULL) {
	free(c);
	return NULL;
    }
    c->type = type;
    return c;
}

void free_node(btree_node *node, db_col **cols, size_t col_size) {
    if (node == NULL)
	return;
    for (size_t i = 0; i < col_size; i++) {
	if (cols[i]->type == STRING)
	    free(node->data[i].str);
    }
    free(node->data);
    free_node(node->left, cols, col_size);
    free_node(node->right, cols, col_size);
    free(node);
}

void free_table(db_table *table) {
    free(table->name);
    free_node(table->root, table->cols, table->col_size);
    for (size_t i = 0; i < table->col_size; i++) {
	free(table->cols[i]->name);
	free(table->cols[i]);
    }
    free(table->cols);
    free(table);
}

