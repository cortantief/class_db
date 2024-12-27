#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool is_valid_column(char *str) {
    if (str == NULL)
	return false;

    for (int i = 0; str[i] != '\0'; i++) {
	if (!(isalnum(*str) || str[i] == '_' || str[i] == '-'))
	    return false;
    }
    return true;
}

void remove_spaces(char *str, bool duplicate_only) {
    char *src = str;
    char *dst = str;
    bool in_quote = false;
    bool space_found = false;
    for (size_t i = 0; str[i] != '\0'; i++) {
	if (str[i] == '"')
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
    if (end == NULL)
	return strdup(start);
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
    end = str + len - 1;
    while (end > str && isspace((unsigned char)*end))
	end--;
    end++;
    out_size = (end - str) < len - 1 ? (end - str) : len - 1;
    out = malloc(sizeof(char) * out_size);

    if (out == NULL)
	return NULL;

    memcpy(out, str, out_size);
    out[out_size] = '\0';
    return out;
}

