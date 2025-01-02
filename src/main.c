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
    if (!insert_data_table(db, "users", 1, new_coldata(strdup("julien"), 25))) {
	free_database(db);
	return 1;
    }
    exec_insert_query(db, sql);
    exec_select_query(db, "SELECT name,age FROM users");
    return 0;
}
