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
	malloc(sizeof(db_col_index *) * (table->col_size));
    size_t coli = 0;
    do {
	for (size_t i = 0; i < table->col_size; i++) {
	    if (strcmp(col, table->cols[i].name) == 0) {
		for (size_t a = 0; a < coli; a++) {
		    if (cols_index[a]->index == i)
			goto end_loop;
		}
		cols_index[coli] = malloc(sizeof(db_col_index));
		cols_index[coli]->index = i;
		cols_index[coli]->type = table->cols[i].type;
		coli++;
	    end_loop:
		break;
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
    db_col dbcols[] = {{.name = "age", .type = INT},
		       {.name = "name", .type = STRING}};
    char *str = trim_whitespace(sql);
    db_search_cond search_cond = NONE;
    db_table *dbtable = new_table("hello");
    dbtable->cols = dbcols;
    dbtable->col_size = 2;
    dbtable->root = new_node(new_coldata("rachid", 23), 0);
    insert_data(dbtable->root, new_coldata("julien", 26), 1);
    // char *str = sql;
    bool t = false;
    // remove_spaces(str, true);
    char *cols = get_from_clause(str, SELECT_CLAUSE, FROM_CLAUSE);
    char *table = get_from_clause(str, FROM_CLAUSE, WHERE_CLAUSE);
    char *cond = get_from_clause(str, WHERE_CLAUSE, NULL);
    remove_spaces(cols, false);
    db_search_query **z = parse_query(cond);
    printf("%p\n", z);
    if (z == NULL)
	return 1;
    db_col_index **cols_index = get_cols_index(dbtable, cols);
    if (cols_index == NULL)
	return 1;
    query_col **c = new_query_col(z, dbtable);
    // search_by_cond(dbtable->root, c, cols_index, print_node);
    print_table(dbtable, c, cols_index);
    free(cols);
    free(table);
    free(cond);
    for (size_t i = 0; cols_index[i] != NULL; i++)
	free(cols_index[i]);
    free(cols_index);
    for (size_t i = 0; c[i] != NULL; i++)
	free(c[i]);
    free(c);

    for (size_t i = 0; z[i] != NULL; i++)
	free(z[i]);
    free(z);
    return 0;
}
