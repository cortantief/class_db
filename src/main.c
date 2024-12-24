#include <stdio.h>
#include <stdlib.h>
#include "repl.h"
#include "btree.h"
#include "table.h"
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

void print_row(union coldata* data, db_col* cols, size_t col_size) {
	printf("| ");
	for (size_t i = 0; i < col_size; i++) {
		if (cols[i].type == STRING)
			printf("%s", data[i].str);
		else if (cols[i].type == INT)
			printf("%d", data[i].i32);
		printf(" | ");
	}
}

int main(int argc, char* argv[], char* envp[]){
	db_table table;
	db_col cols[3] = {{.type = INT, .name = "age"},{.type = STRING, .name = "fname"},{.type = STRING, .name = "lname"}};
	union coldata data[3] = {{.i32 = 26}, {.str="soufiane"}, {.str="harbouli"}};
	table.name = "MAIN";
	table.cols = cols;
	table.col_size = 3;
	table.row_size = 0;
	table.root = new_node(data, ++table.row_size);
	for(size_t i = 0; i < table.row_size; i++)
		print_row(table.root->data, cols, table.col_size);
//  repl();
	return 0;
}
