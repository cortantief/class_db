#include "btree.h"
#include "query.h"
#include "table.h"
#include <stdio.h>
#include <string.h>
#define MAX_COLUMNS 10
#define MAX_COLUMN_WIDTH 30
#define MAX_ROWS 100

// Prints a horizontal line
void print_horizontal_line(db_col_index **cols_index) {
    for (int i = 0; cols_index[i] != NULL; i++) {
	printf("+");
	for (int j = 0; j < MAX_COLUMN_WIDTH; j++) {
	    printf("-");
	}
    }
    printf("+\n");
}

// Prints the header row
void print_header(db_table *table, db_col_index **cols_index) {
    print_horizontal_line(cols_index);
    for (int i = 0; (cols_index[i] != NULL); i++) {
	printf("| %-*s", MAX_COLUMN_WIDTH - 1,
	       table->cols[cols_index[i]->index].name);
    }
    printf("|\n");
    print_horizontal_line(cols_index);
}

// Prints a row of data
void print_row(btree_node *node, db_col_index **cols_index) {
    if (node == NULL)
	return;
    for (int i = 0; cols_index[i] != NULL; i++) {
	if (cols_index[i]->type == STRING)
	    printf("| %-*s", MAX_COLUMN_WIDTH - 1,
		   node->data[cols_index[i]->index].str);
	else if (cols_index[i]->type == INT)
	    printf("| %-*d", MAX_COLUMN_WIDTH - 1,
		   node->data[cols_index[i]->index].i32);
    }
    printf("|\n");
    print_horizontal_line(cols_index);
}

// Finalizes the table

void print_table(db_table *table, query_col **qcol, db_col_index **cols_index) {
    if (cols_index == NULL || table == NULL || qcol == NULL)
	return;
    print_header(table, cols_index);
    // print_row(table->root, cols_index);
    search_by_cond(table->root, qcol, cols_index, print_row);
    // print_footer(table);
    // print_horizontal_line(cols_index);
}

