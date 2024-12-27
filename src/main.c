#include "btree.h"
#include "constants.h"
#include "parser.h"
#include "repl.h"
#include "table.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_row(union coldata *data, db_col *cols, size_t col_size) {
    printf("| ");
    for (size_t i = 0; i < col_size; i++) {
	if (cols[i].type == STRING)
	    printf("%s", data[i].str);
	else if (cols[i].type == INT)
	    printf("%d", data[i].i32);
	printf(" | ");
    }
}

union coldata *new_coldata(char *name, int32_t age) {
    union coldata *c = malloc(sizeof(union coldata) * 2);
    c[0].i32 = age;
    c[1].str = name;
    return c;
}

int main(int argc, char *argv[], char *envp[]) {
    char *sql =
	"SELECT name,age from users where fname >= 'rober', lname = 'julien'";
    db_col dbcols[] = {{.name = "age", .type = INT},
		       {.name = "name", .type = STRING}};
    char *str = trim_whitespace(sql);
    db_search_cond search_cond = NONE;
    db_table *dbtable = new_table("hello");
    dbtable->cols = dbcols;
    dbtable->root = new_node(new_coldata("rachid", 23), 0);
    insert_data(dbtable->root, new_coldata("julien", 26), 1);
    // char *str = sql;
    bool t = false;
    // remove_spaces(str, true);
    char *cols = get_from_clause(str, "SELECT", "from");
    char *table = get_from_clause(str, "from", "WHERE");
    char *cond = get_from_clause(str, "where", NULL);
    db_search_query **z = parse_query(cond);
    for (size_t i = 0; z != NULL && z[i] != NULL; i++)
	printf("table=%s | cond=%b | requ=%s\n", z[i]->table, z[i]->cond,
	       z[i]->data.str);
    return 0;
}
