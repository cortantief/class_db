#include "query.h"
#include "btree.h"
#include "constants.h"
#include "table.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool test_condition_integer(union coldata *data, db_search_cond cond,
			    union coldata *to_cmp) {
    if (data == NULL || to_cmp == NULL)
	return false;
    if ((cond & EQUAL) == EQUAL && data->i32 == to_cmp->i32)
	return true;
    if ((cond & LESS) == LESS && data->i32 < to_cmp->i32)
	return true;
    if ((cond & MORE) == MORE && data->i32 > to_cmp->i32)
	return true;
    return false;
}

bool test_condition_str(union coldata *data, db_search_cond cond,
			union coldata *to_cmp) {
    if (data == NULL || to_cmp == NULL)
	return false;
    int cmp = strcmp(data->str, to_cmp->str);
    if ((cond & EQUAL) == EQUAL && cmp == 0)
	return true;
    if ((cond & LESS) == LESS && cmp < 0)
	return true;
    if ((cond & MORE) == MORE && cmp > 0)
	return true;
    return false;
}

query_col *new_qcol(db_search_query *query, enum coltype t, size_t index) {
    query_col *qcol = malloc(sizeof(query_col));
    if (qcol == NULL)
	return NULL;
    qcol->query = query;
    qcol->index = index;
    qcol->type = t;
    return qcol;
}

query_col **new_query_col(db_search_query **queries, db_table *table) {
    size_t queries_nbr = 0;
    while (queries[queries_nbr] != NULL)
	queries_nbr++;
    query_col **qcol = malloc(sizeof(query_col *) * (queries_nbr + 1));
    for (size_t i = 0; i < queries_nbr; i++)
	qcol[i] = NULL;
    size_t qcol_index = 0;
    for (size_t i = 0; (qcol_index < queries_nbr) && (i < table->col_size);
	 i++) {
	for (size_t a = 0; a < queries_nbr; a++) {
	    if (strcmp(queries[a]->column, table->cols[i].name) == 0) {
		qcol[qcol_index++] =
		    new_qcol(queries[a], table->cols[i].type, i);
		break;
	    }
	}
    }
    return qcol;
};

void search_by_cond(btree_node *node, query_col **qcol,
		    db_col_index **cols_index,
		    void (*func)(btree_node *node, db_col_index **cols_index)) {
    if (node == NULL || qcol == NULL || func == NULL)
	return;
    for (size_t i = 0; qcol[i] != NULL; i++) {
	union coldata data = node->data[qcol[i]->index];
	switch (qcol[i]->type) {
	case INT:
	    if (!test_condition_integer(&data, qcol[i]->query->cond,
					&qcol[i]->query->data))
		goto end;
	    break;
	case STRING:
	    if (!test_condition_str(&data, qcol[i]->query->cond,
				    &qcol[i]->query->data))
		goto end;
	    break;
	}
    }
    func(node, cols_index);
end:
    search_by_cond(node->left, qcol, cols_index, func);
    search_by_cond(node->right, qcol, cols_index, func);
}

