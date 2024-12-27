#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

bool is_valid_column(char *str);
char **parse_columns(char *d);
char *get_from_clause(char *str, char *start_clause, char *end_clause);
void remove_spaces(char *str, bool duplicate_only);
char *trim_whitespace(const char *str);
#endif
