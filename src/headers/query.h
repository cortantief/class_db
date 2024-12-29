#ifndef QUERY_H
#define QUERY_H
#include "btree.h"
#include "table.h"

typedef struct {
    db_search_query *query;
    size_t index;
    enum coltype type;
} query_col;

query_col **new_query_col(db_search_query **queries, db_table *table);
void search_by_cond(btree_node *node, query_col **qcol,
		    db_col_index **cols_index,
		    void (*func)(btree_node *, db_col_index **cols_index));

#endif
