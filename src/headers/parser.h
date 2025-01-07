#ifndef PARSER_H
#define PARSER_H

#include "table.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

bool is_valid_column(char *str);
char **parse_columns(char *d);
char *get_from_clause(char *str, char *start_clause, char *end_clause);
void remove_spaces(char *str, bool duplicate_only);
char *trim_whitespace(const char *str);
void trim_whitespace_inplace(char *str);
db_search_query **parse_query(db_table *dbtable, char *query);
db_col_index **get_cols_index(db_table *table, char *cols);
db_table *parse_table_definition(char *str);
char **extract_all_values(const char *query);
#endif
