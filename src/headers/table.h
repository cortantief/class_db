#ifndef TABLE_H
#define TABLE_H

#include "btree.h"

typedef struct {
    char *name;
    db_col **cols;
    size_t col_size;
    size_t row_size;
    btree_node *root;
} db_table;

typedef enum {
    NONE = 0,	    // No flags set
    EQUAL = 1 << 0, // 1 (bit 0)
    LESS = 1 << 1,  // 2 (bit 1)
    MORE = 1 << 2,  // 4 (bit 2)
    NOT = 1 << 3    // 8 (bit 3)
} db_search_cond;

typedef struct {
    db_search_cond cond;
    union coldata data;
    enum coltype type;
    size_t index;
} db_search_query;

typedef struct {
    size_t index;
    enum coltype type;
} db_col_index;

typedef struct {
    size_t index;
    union coldata data;
} db_insert;

db_table *new_table(char *name);
db_col *new_col(char *name, enum coltype type);
void free_table(db_table *table);
void free_node(btree_node *node, db_col **cols, size_t col_size);
#endif
