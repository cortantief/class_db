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
	    if (strcmp(col, table->cols[i]->name) == 0) {
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
		cols_index[coli]->type = table->cols[i]->type;
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
    remove_spaces(cols, false);
    db_search_query **z = parse_query(cond);
    if (z == NULL) {
	free(cols);
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
