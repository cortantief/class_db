#include "btree.h"
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

int main(int argc, char *argv[], char *envp[]) {
    char *sql = "SELECT";
    char *str = trim_whitespace(sql);
    bool t = false;
    remove_spaces(str, true);
    char *cols = get_from_clause(str, "SELECT", "from");
    char *table = get_from_clause(str, "from", "WHERE");
    char *cond = get_from_clause(str, "where", NULL);
    remove_spaces(cols, false);
    printf("'%s'\n", str);
    printf("%s\n%s\n%s\n", cols, table, cond);
    return 0;
}
