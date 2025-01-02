#include "btree.h"
#include "query.h"
#include "table.h"
#include <stdio.h>
#include <string.h>
#define MAX_COLUMNS 10
#define MAX_COLUMN_WIDTH 30
#define MAX_ROWS 100

void _print_horizontal_line_with(db_search_query **qcol) {
    for (int i = 0; qcol[i] != NULL; i++) {
	printf("+");
	for (int j = 0; j < MAX_COLUMN_WIDTH; j++) {
	    printf("-");
	}
    }
    printf("+\n");
}

void _print_horizontal_line_without(db_col_index **qcol) {
    for (int i = 0; qcol[i] != NULL; i++) {
	printf("+");
	for (int j = 0; j < MAX_COLUMN_WIDTH; j++) {
	    printf("-");
	}
    }
    printf("+\n");
}

// Prints a horizontal linevoid print_horizontal_line(db_search_query **qcol) {
void print_horizontal_line(db_query *qcol) {
    if (qcol->has_condition)
	_print_horizontal_line_with(qcol->with_cond);
    else
	_print_horizontal_line_without(qcol->without_cond);
}

// Prints the header row
void _print_header_with(db_table *table, db_search_query **qcol) {
    _print_horizontal_line_with(qcol);
    for (int i = 0; qcol[i] != NULL; i++) {
	printf("| %-*s", MAX_COLUMN_WIDTH - 1,
	       table->cols[qcol[i]->index]->name);
    }
    printf("|\n");
    _print_horizontal_line_with(qcol);
}
void _print_header_without(db_table *table, db_col_index **qcol) {
    _print_horizontal_line_without(qcol);
    for (int i = 0; qcol[i] != NULL; i++) {
	printf("| %-*s", MAX_COLUMN_WIDTH - 1,
	       table->cols[qcol[i]->index]->name);
    }
    printf("|\n");
    _print_horizontal_line_without(qcol);
}

void print_header(db_table *table, db_query *qcol) {
    //   print_horizontal_line();
    _print_header_without(table, qcol->without_cond);
    //    printf("|\n");
    //    print_horizontal_line(qcol);
}
void _print_row_with(btree_node *node, db_search_query **qcol) {
    if (node == NULL)
	return;
    for (int i = 0; qcol[i] != NULL; i++) {
	if (qcol[i]->type == STRING)
	    printf("| %-*s", MAX_COLUMN_WIDTH - 1,
		   node->data[qcol[i]->index].str);
	else if (qcol[i]->type == INT)
	    printf("| %-*d", MAX_COLUMN_WIDTH - 1,
		   node->data[qcol[i]->index].i32);
    }
    printf("|\n");
    _print_horizontal_line_with(qcol);
}
void _print_row_without(btree_node *node, db_col_index **qcol) {
    if (node == NULL)
	return;
    for (int i = 0; qcol[i] != NULL; i++) {
	if (qcol[i]->type == STRING)
	    printf("| %-*s", MAX_COLUMN_WIDTH - 1,
		   node->data[qcol[i]->index].str);
	else if (qcol[i]->type == INT)
	    printf("| %-*d", MAX_COLUMN_WIDTH - 1,
		   node->data[qcol[i]->index].i32);
    }
    printf("|\n");
    _print_horizontal_line_without(qcol);
}

// Prints a row of data
void print_row(btree_node *node, db_query *qcol) {
    if (qcol == NULL)
	return;
    _print_row_without(node, qcol->without_cond);
}

// Finalizes the table

void print_table(db_table *table, db_query *qcol) {
    if (table == NULL || qcol == NULL)
	return;
    print_header(table, qcol);
    search_by_cond(table->root, qcol, print_row);
}

