#include "btree.h"
#include "constants.h"
#include "parser.h"
#include "printer.h"
#include "query.h"
#include "repl.h"
#include "table.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

union coldata *new_coldata(char *name, int32_t age) {
    union coldata *c = malloc(sizeof(union coldata) * 2);
    c[0].i32 = age;
    c[1].str = name;
    return c;
}

db_col_index **get_cols_index(db_table *table, char *cols) {
    char *columns = strdup(cols);
    char *col = strtok(columns, DELIMITER);
    db_col_index **cols_index =
	malloc(sizeof(db_col_index *) * (table->col_size + 1));
    if (cols_index == NULL)
	return NULL;
    for (size_t i = 0; i <= table->col_size; i++)
	cols_index[i] = NULL;
    size_t coli = 0;
    do {
	bool already_present = false;
	for (size_t i = 0; i < table->col_size; i++) {
	    if (strcmp(col, table->cols[i].name) == 0) {
		for (size_t a = 0; a < coli; a++) {
		    if (cols_index[a]->index == i) {
			already_present = true;
			break;
		    }
		}
		if (already_present)
		    break;
		cols_index[coli] = malloc(sizeof(db_col_index));
		if (cols_index[coli] == NULL) {
		    for (size_t b = 0; b < coli; b++)
			free(cols_index[b]);
		    free(cols_index);
		    return NULL;
		}
		cols_index[coli]->index = i;
		cols_index[coli]->type = table->cols[i].type;
		coli++;
	    }
	}
    } while ((col = strtok(NULL, DELIMITER)) != NULL);
    free(columns);
    return cols_index;
}

int main(int argc, char *argv[], char *envp[]) {
    if (argc < 2)
	return 1;
    char *sql = argv[1];
    db_col *dbcols = malloc(sizeof(db_col) * 3);
    dbcols[0].name = strdup("age");
    dbcols[0].type = INT;
    dbcols[1].name = strdup("name");
    dbcols[1].type = STRING;
    char *str = trim_whitespace(sql);
    db_search_cond search_cond = NONE;
    db_table *dbtable = new_table("hello");
    dbtable->cols = dbcols;
    dbtable->col_size = 2;
    dbtable->root = new_node(new_coldata(strdup("rachid"), 23), 0);
    insert_data(dbtable->root, new_coldata(strdup("julien"), 26), 1);

    // char *str = sql;
    bool t = false;
    remove_spaces(str, true);
    char *cols = get_from_clause(str, SELECT_CLAUSE, FROM_CLAUSE);
    char *table = get_from_clause(str, FROM_CLAUSE, WHERE_CLAUSE);
    char *cond = get_from_clause(str, WHERE_CLAUSE, NULL);
    remove_spaces(cols, false);
    db_search_query **z = parse_query(cond);
    if (z == NULL)
	return 1;
    db_col_index **cols_index = get_cols_index(dbtable, cols);
    if (cols_index == NULL)
	return 1;
    query_col **c = new_query_col(z, dbtable);
    print_table(dbtable, c, cols_index);
    free(cols);
    free(table);

    free(cond);
    free(str);
    for (size_t i = 0; cols_index[i] != NULL; i++)
	free(cols_index[i]);
    free(cols_index);
    for (size_t i = 0; c[i] != NULL; i++)
	free(c[i]);
    free(c);

    for (size_t i = 0; z[i] != NULL; i++) {
	free(z[i]->column);
	free(z[i]);
    };
    free(z);
    free_table(dbtable);
    return 0;
}
