#include "btree.h"
#include "database.h"
#include "repl.h"
#include "table.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PREFIX_SEPARATOR ":"

#define PREFIX_DATABASE_SIZE "DATABASE_SIZE"
#define PREFIX_DATABASE "DATABASE_NAME"
#define PREFIX_DATABASE_TABLE_SIZE "DATABASE_TABLE_SIZE"
#define PREFIX_DATABASE_TABLE "DATABASE_TABLE"
#define PREFIX_DATABASE_TABLE_COL_SIZE "DATABASE_TABLE_COL_SIZE"
#define PREFIX_DATABASE_TABLE_COL "DATABASE_TABLE_COL"
#define PREFIX_DATABASE_TABLE_VALUE_SIZE "DATABASE_TABLE_VALUE_SIZE"
#define PREFIX_DATABASE_TABLE_VALUE "DATABASE_TABLE_VALUE"

#define DATABASE_FILE "class_db.cdb"
#define TEST_DATABASE_FILE "test.cdb"
#define DATABASE_SIZE PREFIX_DATABASE_SIZE PREFIX_SEPARATOR "%zu"
#define DATABASE PREFIX_DATABASE PREFIX_SEPARATOR "%zu" PREFIX_SEPARATOR "%s"
#define DATABASE_TABLE_SIZE PREFIX_DATABASE_TABLE_SIZE PREFIX_SEPARATOR "%zu"
#define DATABASE_TABLE                                                         \
    PREFIX_DATABASE_TABLE PREFIX_SEPARATOR "%zu" PREFIX_SEPARATOR "%s"
#define DATABASE_TABLE_COL_SIZE                                                \
    PREFIX_DATABASE_TABLE_COL_SIZE PREFIX_SEPARATOR "%zu"
#define DATABASE_TABLE_COL                                                     \
    PREFIX_DATABASE_TABLE_COL PREFIX_SEPARATOR "%zu" PREFIX_SEPARATOR          \
					       "%s" PREFIX_SEPARATOR "%d"
#define DATABASE_TABLE_VALUE                                                   \
    PREFIX_DATABASE_TABLE_VALUE PREFIX_SEPARATOR "%zu" PREFIX_SEPARATOR "%s"
#define DATABASE_TABLE_VALUE_SIZE                                              \
    PREFIX_DATABASE_TABLE_VALUE_SIZE PREFIX_SEPARATOR "%zu"

bool write_value(db_table *table, FILE *file, btree_node *node) {
    if (node == NULL)
	return true;
    for (size_t ci = 0; ci < table->col_size; ci++) {
	char *value;
	db_col *col = table->cols[ci];
	if (col->type == STRING)
	    value = node->data[ci].str;
	if (col->type == INT) {
	    size_t n =
		(size_t)((ceil(log10(node->data[ci].i32)) + 1) * sizeof(char));
	    value = calloc(n + 1, sizeof(char));
	    sprintf(value, "%d", node->data[ci].i32);
	}
	fprintf(file, DATABASE_TABLE_VALUE, (size_t)strlen(value), value);
	if (col->type == INT)
	    free(value);
    }
    if (!write_value(table, file, node->left))
	return false;
    return write_value(table, file, node->right);
}

bool write_table(db_table *table, FILE *file) {
    fprintf(file, DATABASE_TABLE "\n", (size_t)strlen(table->name),
	    table->name);
    fprintf(file, DATABASE_TABLE_COL_SIZE "\n", table->col_size);
    for (size_t ci = 0; ci < table->col_size; ci++) {
	db_col *col = table->cols[ci];
	fprintf(file, DATABASE_TABLE_COL "\n", (size_t)strlen(col->name),
		col->name, col->type);
    }
    fprintf(file, DATABASE_TABLE_VALUE_SIZE "\n", (size_t)table->row_size);
    return write_value(table, file, table->root);
}
bool write_header(app_state *state, FILE *file) {
    fprintf(file, DATABASE_SIZE "\n", state->database_size);
    for (size_t dbi = 0; dbi < state->database_size; dbi++) {
	database *db = state->databases[dbi];
	fprintf(file, DATABASE "\n", (size_t)strlen(db->name), db->name);
	fprintf(file, DATABASE_TABLE_SIZE "\n", db->table_size);
	printf("DATABAS\n");
	for (size_t ti = 0; ti < db->table_size; ti++)
	    write_table(db->tables[ti], file);
    }
    return true;
}

bool save(app_state *state) {
    FILE *fptr = fopen(DATABASE_FILE, "w");
    bool v = write_header(state, fptr);
    fclose(fptr);
    return v;
}
bool load_value(FILE *fptr, db_table *table) {
    union coldata *data = malloc(sizeof(union coldata) * (table->col_size));
    for (size_t i = 0; i < table->col_size; i++) {
	char *value;
	size_t len = 0;
	fscanf(fptr, PREFIX_DATABASE_TABLE_VALUE PREFIX_SEPARATOR "%zu", &len);
	db_col *col = table->cols[i];
	value = calloc(len + 1, sizeof(char));
	fscanf(fptr, PREFIX_SEPARATOR "%s\n", value);
	if (col->type == STRING)
	    data[i].str = value;
	if (col->type == INT) {
	    data[i].i32 = atoi(value);
	    free(value);
	}
    }

    if (table->row_size == 0) {
	if ((table->root = new_node(data, ++table->row_size)) == NULL) {
	    table->row_size = 0;
	    return false;
	}
	return true;
    } else if (_insert_data(table->root, data, table->row_size + 1)) {
	table->row_size++;
	return true;
    }
    return false;
}
bool load_col(FILE *fptr, db_table *table) {
    size_t len = 0;
    char *col_name = NULL;
    enum coltype col_type = Nil;
    fscanf(fptr,
	   PREFIX_DATABASE_TABLE_COL PREFIX_SEPARATOR "%zu" PREFIX_SEPARATOR,
	   &len);
    if (len == 0)
	return false;
    col_name = calloc(++len, sizeof(char));
    if (col_name == NULL)
	return false;
    // fscanf(fptr, PREFIX_SEPARATOR "%s" PREFIX_SEPARATOR "\n", col_name);
    fgets(col_name, len, fptr);
    fscanf(fptr, PREFIX_SEPARATOR "%d\n", (int *)(&col_type));
    bool r = insert_table_col(table, col_name, col_type);
    free(col_name);
    return r;
}

db_table *load_table(FILE *fptr) {
    size_t len = 0;
    size_t col_size = 0;
    fscanf(fptr, PREFIX_DATABASE_TABLE PREFIX_SEPARATOR "%zu" PREFIX_SEPARATOR,
	   &len);
    char *table_name = calloc(len + 1, sizeof(char));
    if (table_name == NULL)
	return NULL;
    fscanf(fptr, "%s\n", table_name);
    db_table *table = new_table(table_name);
    free(table_name);
    if (table == NULL)
	return NULL;
    fscanf(fptr, DATABASE_TABLE_COL_SIZE "\n", &col_size);
    for (size_t i = 0; i < col_size; i++)
	load_col(fptr, table);
    size_t row_size = 0;
    fscanf(fptr, DATABASE_TABLE_VALUE_SIZE "\n", &row_size);
    load_value(fptr, table);
    return table;
}

database *load_db(FILE *fptr) {
    char *dbname;
    size_t len;
    size_t table_size = 0;
    database *db = NULL;
    fscanf(fptr, PREFIX_DATABASE PREFIX_SEPARATOR "%zu" PREFIX_SEPARATOR, &len);
    dbname = calloc(len + 1, sizeof(char));
    fscanf(fptr, "%s\n", dbname);
    if (dbname == NULL)
	return NULL;
    db = new_database(dbname);
    free(dbname);
    if (db == NULL)
	return NULL;
    fscanf(fptr, DATABASE_TABLE_SIZE "\n", &table_size);
    if (table_size == 0)
	return db;
    db->tables = calloc(table_size + 1, sizeof(db_table *));
    if (db->tables == NULL) {
	free(db);
	return NULL;
    }
    for (size_t i = 0; i < table_size; i = ++db->table_size) {
	db->tables[i] = load_table(fptr);
    }
    return db;
}

app_state *load() {
    FILE *fptr = fopen(DATABASE_FILE, "r");
    if (fptr == NULL)
	return new_app_state();
    app_state *state = calloc(1, sizeof(app_state));

    fscanf(fptr, DATABASE_SIZE "\n", &state->database_size);
    state->database_capacity = state->database_size;
    state->databases = calloc(state->database_size, sizeof(database *));
    for (size_t dbi = 0; dbi < state->database_size; dbi++) {
	state->databases[dbi] = load_db(fptr);
    }
    return state;
}
