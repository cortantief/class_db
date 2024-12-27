#include "constants.h"
#include "table.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool is_valid_char(char c) { return isalnum(c) || c == '_' || c == '-'; }
bool is_valid_column(char *str) {
    if (str == NULL)
	return false;

    for (int i = 0; str[i] != '\0'; i++) {
	if (!is_valid_char(str[i]))
	    return false;
    }

    return true;
}

void trim_whitespace_inplace(char *str) {
    if (str == NULL || *str == '\0') {
	return;
    }

    // Trim leading whitespace
    char *start = str;
    while (isspace((unsigned char)*start)) {
	start++;
    }

    if (start != str) {
	memmove(str, start, strlen(start) + 1); // Shift the string to the left
    }

    // Trim trailing whitespace
    char *end = str + strlen(str) - 1;
    while (end >= str && isspace((unsigned char)*end)) {
	*end = '\0';
	end--;
    }
}

void remove_spaces(char *str, bool duplicate_only) {
    char *src = str;
    char *dst = str;
    bool in_quote = false;
    bool space_found = false;
    for (size_t i = 0; str[i] != '\0'; i++) {
	if (str[i] == '"' || str[i] == '\'')
	    in_quote = !in_quote;

	if (str[i] != ' ' || in_quote) {
	    *dst = src[i];
	    dst++;
	    space_found = false;
	} else if (!space_found && duplicate_only) {
	    space_found = true;
	    *dst = src[i];
	    dst++;
	}
    }
    *dst = '\0';
}

char **parse_columns(char *d) {
    char *copyd = strdup(d);
    if (copyd == NULL)
	return NULL;

    size_t t = 0;
    for (int i = 0; d[i] != '\0'; i++) {
	if (d[i] == ',')
	    t++;
    }

    char **cols = malloc(sizeof(char *) * (t + 2));
    if (cols == NULL) {
	free(copyd);
	return NULL;
    }
    char *col = strtok(copyd, ",");
    t = 0;
    while (col != NULL) {
	cols[t++] = strdup(col);
	col = strtok(NULL, ",");
    };
    cols[t] = NULL;
    free(copyd);
    return cols;
}

char *get_from_clause(char *str, char *start_clause, char *end_clause) {
    if (str == NULL || start_clause == NULL)
	return NULL;
    char *start = strcasestr(str, start_clause) + strlen(start_clause);
    char *end = end_clause == NULL ? NULL : strcasestr(str, end_clause);
    if (start == NULL)
	return NULL;
    if (end == NULL) {
	return strdup(start);
    }
    char *cstr = malloc(sizeof(char) * (end - start + 1));
    for (size_t i = 0; (start + i) != end; i++)
	cstr[i] = start[i];
    return cstr;
}

char *trim_whitespace(const char *str) {
    if (str == NULL)
	return NULL;
    const char *end;
    size_t out_size = 0;
    char *out;
    size_t len = 0;

    while (isspace((unsigned char)*str))
	str++;
    if (*str == 0) {
	*out = '\0';
	return out;
    }

    len = strlen(str);
    end = str + len;
    while (end > str && isspace((unsigned char)*end))
	end--;
    end++;
    out_size = (end - str) < len ? (end - str) : len;
    out = malloc(sizeof(char) * out_size);

    if (out == NULL)
	return NULL;

    memcpy(out, str, out_size);
    out[out_size] = '\0';
    return out;
}

bool is_valid_cond(db_search_cond cond) {
    if (cond == NONE)
	return false;
    else if (((cond & LESS) == LESS) && ((cond & MORE) == MORE)) {
	return false;
    }
    return true;
}

db_search_cond parse_cond(char *query, size_t *starti, size_t *endi) {
    db_search_cond cond = NONE;
    size_t start_condi = *starti;
    size_t end_condi = *endi;
    while (query[start_condi] != '\0') {
	if (query[start_condi] == LESS || query[start_condi] == MORE_OP ||
	    query[start_condi] == EQUAL_OP)
	    break;
	start_condi++;
    }
    end_condi = start_condi + 1;
    while (query[end_condi] != '\0' &&
	   (query[end_condi] == LESS_OP || query[end_condi] == MORE_OP ||
	    query[end_condi] == EQUAL_OP))
	end_condi++;
    for (size_t i = start_condi; i < end_condi; i++) {
	if (query[i] == LESS_OP)
	    cond |= LESS;
	else if (query[i] == MORE_OP)
	    cond |= MORE;
	else if (query[i] == EQUAL_OP)
	    cond |= EQUAL;
    }
    if (!is_valid_cond(cond))
	return NONE;
    *starti = start_condi;
    *endi = end_condi;
    return cond;
}

db_search_query *_parse_query(char *query) {
    db_search_query *q = malloc(sizeof(db_search_query));
    if (q == NULL)
	return NULL;
    size_t start_condi = 0;
    size_t end_condi = 0;
    char cond_char[] = {LESS_OP, MORE_OP, EQUAL_OP, '\0'};
    q->cond = parse_cond(query, &start_condi, &end_condi);
    char *table = malloc(sizeof(char) * (start_condi + 1));
    if (table == NULL) {
	free(q);
	return NULL;
    }

    char *req = malloc(sizeof(char) * (strlen(query) - end_condi + 1));
    if (req == NULL) {
	free(q);
	free(table);
	return NULL;
    }

    for (size_t i = 0; i < start_condi; i++)
	table[i] = query[i];
    table[start_condi] = '\0';
    trim_whitespace_inplace(table);
    for (size_t i = 0; query[end_condi + i] != '\0'; i++)
	req[i] = query[end_condi + i];
    req[end_condi] = '\0';
    trim_whitespace_inplace(req);

    if (!is_valid_column(table)) {
	free(table);
	free(req);
	free(q);
	return NULL;
    }

    q->data.str = req;
    q->table = table;

    return q;
}
db_search_query **parse_query(char *query) {
    db_search_query **queries = NULL;
    if (query == NULL)
	return NULL;
    size_t counter = 0;
    for (size_t i = 0; query[i] != '\0'; i++)
	query[i] == ',' ? counter++ : counter;
    queries = malloc(sizeof(db_search_query *) * (counter + 2));
    queries[counter + 1] = NULL;
    char *q = strtok(query, ",");
    counter = 0;
    do {
	if ((queries[counter++] = _parse_query(q)) == NULL) {
	    for (size_t i = 0; i < counter; i++)
		free(queries[i]);
	    free(queries);
	    return NULL;
	}
    } while ((q = strtok(NULL, ",")) != NULL);
    return queries;
}

