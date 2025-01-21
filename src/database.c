#include "database.h"
#include "btree.h"
#include "constants.h"
#include "parser.h"
#include "printer.h"
#include "query.h"
#include "table.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

database *new_database(char *name) {
    database *db = malloc(sizeof(database));
    if (db == NULL)
	return NULL;
    db->name = strdup(name);
    if (db->name == NULL) {
	free(db);
	return NULL;
    }
    db->table_size = 0;
    db->tables = NULL;
    return db;
}

db_table *get_table_by_name(database *db, char *name) {
    if (db == NULL || name == NULL)
	return NULL;
    for (size_t i = 0; i < db->table_size; i++) {
	if (strcmp(name, db->tables[i]->name) == 0)
	    return db->tables[i];
    }
    return NULL;
}

bool insert_table(database *db, db_table *table) {
    size_t new_size = db->table_size + 1;
    db_table **new_tables = malloc(sizeof(db_table *) * (new_size));
    if (new_tables == NULL)
	return false;
    for (size_t i = 0; i < db->table_size; i++)
	new_tables[i] = db->tables[i];
    new_tables[db->table_size] = table;
    db->table_size = new_size;
    free(db->tables);
    db->tables = new_tables;
    return true;
}

bool insert_table_col(db_table *table, char *name, enum coltype type) {
    size_t new_size = table->col_size + 1;
    db_col **new_cols = malloc(sizeof(db_col *) * new_size);
    if (new_cols == NULL)
	return NULL;
    for (size_t i = 0; i < table->col_size; i++)
	new_cols[i] = table->cols[i];
    new_cols[table->col_size] = new_col(name, type);
    if (new_cols[table->col_size] == NULL) {
	free(new_cols);
	return false;
    }
    free(table->cols);
    table->cols = new_cols;
    table->col_size = new_size;
    return true;
}

bool new_database_table(database *db, char *name) {
    db_table *table = new_table(name);
    if (table == NULL)
	return false;
    if (insert_table(db, table) == false)
	return false;
    return true;
}

bool free_database(database *db) {
    if (db == NULL)
	return true;
    free(db->name);
    for (size_t i = 0; i < db->table_size; i++)
	free_table(db->tables[i]);
    free(db->tables);
    free(db);
    return true;
}

bool insert_col_table(database *db, char *table_name, char *col_name,
		      enum coltype type) {
    if (db == NULL || col_name == NULL || table_name == NULL)
	return false;
    for (size_t i = 0; i < db->table_size; i++) {
	if (strcmp(db->tables[i]->name, table_name) == 0)
	    return insert_table_col(db->tables[i], col_name, type);
    }
    return false;
}

bool _insert_data(btree_node *tree, union coldata *data, size_t k) {
    btree_node *tmp = NULL;
    if (tree == NULL)
	return false;

    printf("zed\n");
    tmp = tree;
    while (tmp != NULL) {
	if (tmp->key == k)
	    return false;
	if (tmp->key < k) {
	    if (tmp->left == NULL) {
		tmp->left = new_node(data, k);
		return tmp->left != NULL;
	    }
	    tmp = tmp->left;
	} else if (tmp->key > k) {
	    if (tmp->right == NULL) {
		tmp->right = new_node(data, k);
		return tmp->right != NULL;
	    }
	    tmp = tmp->right;
	}
    }
    return false;
}
bool _update_data(btree_node *tree, union coldata *data, size_t k) {
    btree_node *tmp = NULL;
    if (tree == NULL)
	return false;

    tmp = tree;
    while (tmp != NULL) {
	if (tmp->key == k) {
	    tmp->data = data;
	    return true;
	}
	if (tmp->key < k)
	    tmp = tmp->left;
	else if (tmp->key > k)
	    tmp = tmp->right;
    }
    return false;
}

btree_node *find_min(btree_node *root) {
    while (root && root->left != NULL)
	root = root->left;
    return root;
}

btree_node *_delete_data(btree_node *root, size_t key, db_col **cols,
			 size_t col_size) {
    if (root == NULL)
	return NULL;

    if (key < root->key) {
	root->left = _delete_data(root->left, key, cols, col_size);
    } else if (key > root->key) {
	root->right = _delete_data(root->right, key, cols, col_size);
    } else {
	if (root->left == NULL && root->right == NULL) {
	    free_node(root, cols, col_size);
	    return NULL;
	}

	if (root->left == NULL) {
	    btree_node *temp = root->right;
	    free_node(root, cols, col_size);
	    return temp;
	} else if (root->right == NULL) {
	    btree_node *temp = root->left;
	    free_node(root, cols, col_size);
	    return temp;
	}

	btree_node *temp = find_min(root->right); // Find in-order successor
	root->data = temp->data; // Replace data with in-order successor's data
	root->right = _delete_data(root->right, temp->key, cols,
				   col_size); // Delete the in-order successor
    }

    return root;
}

bool insert_data_table(db_table *table, size_t key, union coldata *data) {
    if (data == NULL)
	return false;
    if (table == NULL)
	return false;
    if (table->root != NULL) {
	if (_insert_data(table->root, data, key)) {
	    table->row_size++;
	    return true;
	}
	return false;
    }
    table->root = new_node(data, key);
    table->row_size = 1;
    return table->root != NULL;
}

bool update_data_table(database *db, char *table_name, size_t key,
		       union coldata *data) {
    db_table *table = NULL;
    if (data == NULL)
	return false;
    for (size_t i = 0; i < db->table_size; i++) {
	if (strcmp(table_name, db->tables[i]->name) == 0) {
	    table = db->tables[i];
	    break;
	}
    }
    if (table == NULL)
	return false;
    return _update_data(table->root, data, key);
}

bool delete_data_table(database *db, char *table_name, size_t key,
		       union coldata *data) {
    db_table *table = NULL;
    if (data == NULL)
	return false;
    for (size_t i = 0; i < db->table_size; i++) {
	if (strcmp(table_name, db->tables[i]->name) == 0) {
	    table = db->tables[i];
	    break;
	}
    }
    if (table == NULL)
	return false;
    _delete_data(table->root, key, table->cols, table->col_size);
    return true;
}

db_query *_parse_select_query_cond(db_table *dbtable, char *cols, char *cond) {
    db_query *query = malloc(sizeof(db_query));
    if (query == NULL)
	return NULL;
    db_search_query **queries = parse_query(dbtable, cond);
    if (queries == NULL) {
	free(query);
	return NULL;
    }
    db_col_index **cols_index = get_cols_index(dbtable, cols);
    if (cols_index == NULL) {
	free(query);
	free(queries);
	return NULL;
    }
    query->has_condition = true;
    query->with_cond = queries;
    return query;
}

db_query *parse_select_query(database *db, char *sql_query) {
    char *str = trim_whitespace(sql_query);
    remove_spaces(str, true);
    char *cols_name = get_from_clause(str, SELECT_CLAUSE, FROM_CLAUSE);
    if (cols_name == NULL)
	goto free_query;
    remove_spaces(cols_name, false);
    char *table_name = get_from_clause(str, FROM_CLAUSE, WHERE_CLAUSE);
    if (table_name == NULL)
	goto free_cols;
    remove_spaces(table_name, false);
    db_table *dbtable = get_table_by_name(db, table_name);
    if (dbtable == NULL)
	goto free_table;

    char *cond = get_from_clause(str, WHERE_CLAUSE, NULL);
    if (cond != NULL) {
	db_search_query **z = parse_query(dbtable, cond);
	if (z == NULL)
	    goto free_cond;
	db_col_index **cols_index = get_cols_index(dbtable, cols_name);
	if (cols_index == NULL) {
	    free(z);
	    goto free_cond;
	}
    }
free_cond:
    free(cond);
free_table:
    free(table_name);
free_cols:
    free(cols_name);
free_query:
    free(str);

    return NULL;
}

int exec_select_query(database *db, char *sql) {
    char *str = trim_whitespace(sql);
    remove_spaces(str, true);
    char *cols = get_from_clause(str, SELECT_CLAUSE, FROM_CLAUSE);
    char *table = get_from_clause(str, FROM_CLAUSE, WHERE_CLAUSE);
    remove_spaces(table, false);
    db_table *dbtable = get_table_by_name(db, table);
    if (dbtable == NULL) {
	free(cols);
	free(str);
	free(table);
	return 1;
    }
    char *cond = get_from_clause(str, WHERE_CLAUSE, NULL);
    remove_spaces(cols, false);
    db_search_query **z = parse_query(dbtable, cond);
    db_col_index **cols_index = get_cols_index(dbtable, cols);
    db_query c;
    c.has_condition = z != NULL;
    if (cond != NULL && z == NULL)
	printf("ERRROROR!\n");
    if (c.has_condition) {
	c.with_cond = z;
    }
    c.without_cond = cols_index;
    if (cols_index == NULL)
	return 1;
    print_table(dbtable, &c);
    free(cols);
    free(table);
    free(cond);
    free(str);
    for (size_t i = 0; cols_index[i] != NULL; i++)
	free(cols_index[i]);
    free(cols_index);

    for (size_t i = 0; z != NULL && z[i] != NULL; i++)
	free(z[i]);
    free(z);
    return 0;
}

size_t count_part(char *str) {
    size_t counter = 0;
    bool in_quote = false;
    for (size_t i = 0; str[i] != '\0'; i++) {
	if (str[i] == '"' || str[i] == '\'')
	    in_quote = !in_quote;
	else if (str[i] == ',' && !in_quote)
	    counter++;
    }
    return counter + 1;
}

db_col_index *get_col_index(db_table *table, char *table_part) {
    char **values = extract_all_values(table_part);
    size_t part_nbr = count_part(*values);
    if (part_nbr != table->col_size) {
	free(*values);
	free(values);
	return false;
    }
    db_col_index *cindex = malloc(sizeof(db_col_index) * table->col_size);
    size_t len = 0;
    char *col = strtok(*values, ",");
    do {
	bool found = false;
	for (size_t i = 0; !found && i < table->col_size; i++) {
	    if (strcmp(table->cols[i]->name, col) == 0) {
		found = true;
		cindex[len].index = i;
		cindex[len].type = table->cols[i]->type;
		len++;
	    }
	}
	if (found == false) {
	    free(cindex);
	    return NULL;
	}
    } while ((col = strtok(NULL, ",")) != NULL);
    return cindex;
}
char *get_table_name_from_part(char *table_part) {
    size_t end = 0;
    for (size_t i = 0; table_part[i] != '\0'; i++) {
	if (table_part[i] == ' ') {
	    end = i;
	    break;
	}
    }
    if (end == 0)
	return NULL;
    char *table_name = malloc(sizeof(char) * (end + 1));
    if (table_name == NULL)
	return NULL;
    for (size_t i = 0; i < end; i++)
	table_name[i] = table_part[i];
    table_name[end] = '\0';
    remove_spaces(table_name, false);
    return table_name;
}

enum coltype parse_coltype(char *str) {
    if (str == NULL)
	return Nil;
    char fc = '\0';
    if (str[0] == '\'' || str[0] == '"')
	fc = str[0];
    size_t len = fc != '\0' ? 1 : 0;
    for (; fc != '\0' && str[len] != '\0'; len++) {
	if (str[len] == fc)
	    break;
    }
    for (size_t i = 0; fc == '\0' && str[i] != '\0'; i++) {
	if (!isdigit(str[i]))
	    return Nil;
	len++;
    }
    if (fc != '\0' && len == strlen(str) - 1)
	return STRING;
    if (fc == '\0' && len == strlen(str))
	return INT;
    return Nil;
}

char **split_values(char *values) {
    size_t len = count_part(values);
    char **parts = malloc(sizeof(char *) * len);
    if (parts == NULL)
	return NULL;
    len = 0;
    size_t starti = 0;
    bool in_quote = false;
    for (size_t i = 0; values[i] != '\0'; i++) {
	if (values[i] == '\'' || values[i] == '"')
	    in_quote = !in_quote;
	if (in_quote || values[i] != ',')
	    continue;
	if ((parts[len] = strndup(&values[starti], i - starti)) == NULL) {
	    for (size_t a = 0; a < len; a++)
		free(parts[a]);
	    free(parts);
	    return NULL;
	};

	len++;
	starti = i + 1;
    }
    if ((parts[len] = strndup(&values[starti], strlen(values) - starti)) ==
	NULL) {
	for (size_t a = 0; a < len; a++)
	    free(parts[a]);
	free(parts);
	return NULL;
    };

    return parts;
}

db_insert **parse_inserts(db_table *dbtable, db_col_index *cindex,
			  char **values) {
    size_t insert_size = 0;
    size_t insert_capacity = 5;
    db_insert **inserts = calloc(insert_capacity, sizeof(db_insert *));
    if (inserts == NULL)
	goto error_handling;
    for (size_t i = 0; values != NULL && values[i] != NULL; i++) {
	if (dbtable->col_size != count_part(values[i]))
	    goto error_handling;
	if (insert_size >= insert_capacity) {
	    db_insert **tmp = realloc(inserts, (insert_capacity *= 2));
	    if (tmp == NULL)
		goto error_handling;
	    inserts = tmp;
	}
	inserts[insert_size] = calloc(dbtable->col_size, sizeof(db_insert));
	if (inserts[insert_size] == NULL)
	    goto error_handling;

	remove_spaces(values[i], false);
	char **parts = split_values(values[i]);
	if (parts == NULL)
	    goto error_handling;
	for (size_t a = 0; a < dbtable->col_size; a++) {
	    enum coltype type = parse_coltype(parts[a]);
	    db_insert insert;
	    if (type == Nil || cindex[a].type != type) {
		for (size_t b = 0; b < dbtable->col_size; b++)
		    free(parts[b]);
		free(parts);
		goto error_handling;
	    }
	    insert.index = cindex[a].index;
	    switch (type) {
	    case INT:
		insert.data.i32 = strtol(parts[a], NULL, 10);
		break;
	    case STRING:
		insert.data.str = strndup(&parts[a][1], strlen(parts[a]) - 2);
		break;
	    case Nil:
		break;
	    }
	    inserts[insert_size][a] = insert;
	}
	insert_size++;
    }
    return inserts;
error_handling:
    if (inserts != NULL) {
	for (size_t i = 0; i < insert_size; i++)
	    free(inserts[i]);
	free(inserts);
    }
    return NULL;
}

union coldata *create_col_data_from_insert(db_table *dbtable,
					   db_insert *inserts) {
    union coldata *data = malloc(sizeof(union coldata) * dbtable->col_size);
    for (size_t a = 0; a < dbtable->col_size; a++) {
	for (size_t b = 0; b < dbtable->col_size; b++) {
	    if (inserts[b].index == a) {
		data[a] = inserts[b].data;
		break;
	    }
	}
    }
    return data;
}

int exec_insert_query(database *db, char *sql) {
    char *str = trim_whitespace(sql);
    int error = 0;
    char *table_part = NULL;
    char *table_name = NULL;
    db_table *dbtable = NULL;
    char *values_part = NULL;
    char **values = NULL;
    db_col_index *cindex = NULL;
    db_insert **inserts = NULL;

    remove_spaces(str, true);
    table_part = get_from_clause(str, INSERT_CLAUSE, VALUES_CLAUSE);
    if (table_part == NULL)
	goto error_handling;
    table_name = get_table_name_from_part(table_part);
    if (table_name == NULL)
	goto error_handling;
    dbtable = get_table_by_name(db, table_name);
    values_part = get_from_clause(str, VALUES_CLAUSE, NULL);
    if (values_part == NULL)
	goto error_handling;
    values = extract_all_values(values_part);
    cindex = get_col_index(dbtable, table_part);
    if (cindex == NULL)
	goto error_handling;
    inserts = parse_inserts(dbtable, cindex, values);
    if (inserts == NULL)
	goto error_handling;
    size_t old_row_size = dbtable->row_size;
    for (size_t i = 0; inserts[i] != NULL; i++) {
	union coldata *data = create_col_data_from_insert(dbtable, inserts[i]);
	if (!insert_data_table(dbtable, dbtable->row_size + 1, data)) {
	    for (size_t a = old_row_size; a < dbtable->row_size; a++)
		_delete_data(dbtable->root, a, dbtable->cols,
			     dbtable->col_size);
	    dbtable->row_size = old_row_size;
	    goto error_handling;
	}
    }
    return 0;
    goto free_all;

error_handling:
    error = 1;

free_all:
    if (table_part != NULL)
	free(table_part);
    if (table_name != NULL)
	free(table_name);
    if (values_part != NULL)
	free(values_part);
    if (values != NULL) {
	for (size_t i = 0; values[i] != NULL; i++)
	    free(values[i]);
	free(values);
    }
    if (cindex != NULL)
	free(cindex);
    if (inserts != NULL) {
	for (size_t i = 0; inserts[i] != NULL; i++)
	    free(inserts[i]);
	free(inserts);
    }
    return error;
}
