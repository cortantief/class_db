#include "constants.h"
#include "table.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char **extract_all_values(const char *query) {
    if (!query)
	return NULL;
    size_t capacity = 5;
    size_t len = 0;
    char **values = malloc(sizeof(char *) * capacity);
    if (values == NULL)
	return NULL;

    char *values_start = strchr(query, '(');
    if (!values_start)
	goto error_handling;

    while (values_start) {
	char *values_end = strchr(values_start, ')');
	if (!values_end)
	    goto error_handling;

	size_t length = values_end - values_start - 1;
	values[len] = malloc(sizeof(char) * (length + 1));
	if (values[len] == NULL)
	    goto error_handling;
	strncpy(values[len], values_start + 1, length);
	values[len][length] = '\0';
	len++;
	if (len >= capacity) {
	    char **new_ptr = realloc(values, (capacity = capacity * 2));
	    if (new_ptr == NULL) {
		for (size_t i = 0; i < len; i++)
		    free(values[i]);
		free(values);
		return NULL;
	    }
	    values = new_ptr;
	}
	values_start = strchr(values_end, '(');
    }
    return values;

error_handling:
    for (size_t i = 0; i < len; i++)
	free(values[i]);
    free(values);
    return NULL;
}

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
    char *start = strcasestr(str, start_clause);
    if (start == NULL)
	return NULL;
    start += strlen(start_clause);
    char *end = end_clause == NULL ? NULL : strcasestr(str, end_clause);
    if (start == NULL)
	return NULL;
    if (end == NULL) {
	return strdup(start);
    }
    char *cstr = malloc(sizeof(char) * (end - start + 1));
    for (size_t i = 0; i < (size_t)(end - start + 1); i++)
	cstr[i] = '\0';
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

    while (isspace(*str))
	str++;
    if (*str == 0) {
	if ((out = malloc(sizeof(char))) == NULL)
	    return NULL;
	*out = '\0';
	return out;
    }

    len = strlen(str);
    end = str + len;
    while (end > str && isspace((unsigned char)*end))
	end--;
    end++;
    out_size = (size_t)(end - str) < len ? (size_t)(end - str) : len;
    out = malloc(sizeof(char) * (out_size + 1));
    if (out == NULL)
	return NULL;

    memcpy(out, str, out_size);
    out[out_size] = '\0';
    return out;
}

bool is_valid_cond(char *cond, size_t l) {
    char valid_cond[][5] = {">", "<", "=", ">=", "<="};
    for (size_t i = 0; i < 5; i++) {
	if (strncmp(cond, valid_cond[i], l) == 0)
	    return true;
    }
    return false;
}

db_search_cond parse_cond(char *query, size_t *starti, size_t *endi) {
    db_search_cond cond = NONE;
    size_t start_condi = *starti;
    size_t end_condi = *endi;
    while (query[start_condi] != '\0') {
	if (query[start_condi] == LESS_OP || query[start_condi] == MORE_OP ||
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
    if (!is_valid_cond(&query[start_condi], end_condi - start_condi))
	return NONE;
    *starti = start_condi;
    *endi = end_condi;
    return cond;
}

void remove_quotes(char *str) {
    if (!str || strlen(str) < 2) {
	return;
    }

    size_t len = strlen(str);
    if ((str[0] == '\'' || str[0] == '\"') && (str[0] == str[len - 1])) {
	memmove(str, str + 1, len - 1);
	str[len - 2] = '\0';
    }
}

db_search_query *_parse_query(db_table *table, char *query) {
    db_search_query *q = malloc(sizeof(db_search_query));
    if (q == NULL)
	return NULL;
    size_t start_condi = 0;
    size_t end_condi = 0;
    q->cond = parse_cond(query, &start_condi, &end_condi);
    char *col = malloc(sizeof(char) * (start_condi + 1));
    if (col == NULL || q->cond == NONE) {
	if (col != NULL)
	    free(col);
	free(q);
	return NULL;
    }
    size_t req_size = strlen(query) - end_condi;
    char *req = malloc(sizeof(char) * (req_size + 1));
    if (req == NULL) {
	free(q);
	free(col);
	return NULL;
    }

    for (size_t i = 0; i < start_condi; i++)
	col[i] = query[i];
    col[start_condi] = '\0';
    trim_whitespace_inplace(col);
    for (size_t i = 0; query[end_condi + i] != '\0'; i++)
	req[i] = query[end_condi + i];
    req[req_size] = '\0';
    trim_whitespace_inplace(req);

    if (!is_valid_column(col)) {
	free(col);
	free(req);
	free(q);
	return NULL;
    }
    db_col *dbcol = NULL;
    for (size_t i = 0; i < table->col_size; i++) {
	if (strcmp(col, table->cols[i]->name) == 0) {
	    dbcol = table->cols[i];
	    break;
	}
    }
    free(col);
    if (dbcol == NULL) {
	free(req);
	free(q);
	return NULL;
    }
    bool is_digit = dbcol->type == INT;
    for (size_t i = 0; is_digit && (req[i] != '\0'); i++) {
	if (!isdigit(req[i])) {
	    free(req);
	    free(q);
	    return NULL;
	}
    }
    if (is_digit) {
	q->data.i32 = strtol(req, NULL, 10);
	q->type = INT;
	free(req);
    } else {
	q->type = STRING;
	remove_quotes(req);
	q->data.str = req;
    }
    return q;
}

db_search_query **parse_query(db_table *table, char *query) {
    db_search_query **queries = NULL;
    if (query == NULL)
	return NULL;
    size_t counter = 0;
    for (size_t i = 0; query[i] != '\0'; i++)
	query[i] == ',' ? counter++ : counter;
    queries = malloc(sizeof(db_search_query *) * (counter + 2));
    queries[counter + 1] = NULL;
    char *q = strtok(query, DELIMITER);
    counter = 0;
    do {
	if ((queries[counter++] = _parse_query(table, q)) == NULL) {
	    for (size_t i = 0; i < counter; i++)
		free(queries[i]);
	    free(queries);
	    return NULL;
	}
    } while ((q = strtok(NULL, DELIMITER)) != NULL);
    return queries;
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

char *_parse_table_definition_db_cols(char *str) {
    size_t starti = 0;
    for (size_t i = 0; str[i] != '\0'; i++) {
	starti++;
	if (str[i] == ' ')
	    break;
    }
    size_t endi = starti + 1;
    for (size_t i = starti; str[i] != '\0'; i++) {
	endi++;
	if (str[i] == ' ')
	    break;
    }
    char *out = malloc(sizeof(char) * (endi - starti + 1));
    for (size_t i = starti; i < endi; i++)
	out[i] = str[i];
    out[endi] = '\0';
    return out;
}

char *_parse_table_definition_db_table(char *str) {
    size_t endi = 0;
    for (size_t i = 0; str[i] != '\0'; i++) {
	endi = i;
	if (str[i] == ' ')
	    break;
    }
    if (endi == 0)
	return NULL;
    char *table_name = malloc(sizeof(char) * (endi + 1));

    for (size_t i = 0; i < endi; i++)
	table_name[i] = str[i];

    table_name[endi] = '\0';
    if (!is_valid_column(table_name)) {
	free(table_name);
	return NULL;
    }
    return table_name;
}

db_col **parse_col(char **cols) { return NULL; }
db_table *parse_table_definition(char *str) {
    char *stmt = trim_whitespace(str);
    remove_spaces(stmt, true);
    char *dbdef = _parse_table_definition_db_table(stmt);
    printf("%s\n", dbdef);
    char **cols_value = extract_all_values(stmt);
    if (cols_value == NULL)
	return NULL;
    printf("str=%s\n", *cols_value);
    char **cols = parse_columns(*cols_value);
    if (cols == NULL)
	return NULL;

    for (size_t i = 0; cols_value[i] == NULL; i++)
	free(cols_value[i]);
    for (size_t i = 0; cols[i] != NULL; i++)
	free(cols[i]);
    free(cols);
}
