#ifndef QUERY_H
#define QUERY_H
#include "btree.h"
#include "table.h"

/*typedef struct {
    db_search_query *query;
    size_t index;
    enum coltype type;
} query_col;
*/
typedef struct {
    bool has_condition;
    db_search_query **with_cond;
    db_col_index **without_cond;
} db_query;

// query_col **new_query_col(db_search_query **queries, db_table *table);
void search_by_cond(btree_node *node, db_query *qcol,
		    void (*func)(btree_node *, db_query *qcol));

#endif
