#include "btree.h"
#include "constants.h"
#include "database.h"
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

int main(int argc, char *argv[]) {
    if (argc < 2)
	return 1;
    char *sql = argv[1];
    database *db = new_database("MY DATABASE");
    if (db == NULL)
	return 1;
    if (!new_database_table(db, "users")) {
	free_database(db);
	return 1;
    }
    if (!insert_col_table(db, "users", "age", INT)) {
	free_database(db);
	return 1;
    }
    if (!insert_col_table(db, "users", "name", STRING)) {
	free_database(db);
	return 1;
    }

    if (!insert_data_table(db, "users", 0, new_coldata(strdup("rachid"), 23))) {
	free_database(db);
	return 1;
    }
    char *str = trim_whitespace(sql);
    db_table *dbtable = get_table_by_name(db, "users");
    remove_spaces(str, true);
    char *cols = get_from_clause(str, SELECT_CLAUSE, FROM_CLAUSE);
    char *table = get_from_clause(str, FROM_CLAUSE, WHERE_CLAUSE);
    char *cond = get_from_clause(str, WHERE_CLAUSE, NULL);
    printf("%p\n", cond);
    remove_spaces(cols, false);
    db_search_query **z = parse_query(cond);
    if (z == NULL) {
	free(cols);
	if (cond != NULL)
		free(cond);
	free(str);
	free_database(db);
	return 1;
    }
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
    free_database(db);
    return 0;
}
