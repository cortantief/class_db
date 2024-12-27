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
    table->name = strdup(name);
    if (table->name == NULL) {
	free(table);
	return NULL;
    }
    table->row_size = 0;
    table->col_size = 0;
    table->root = NULL;
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
