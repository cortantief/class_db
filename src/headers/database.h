#ifndef DATABASE_H
#define DATABASE_H

#include "table.h"

typedef struct {
    char *name;
    db_table **tables;
    size_t table_size;
} database;

bool insert_table(database *db, db_table *table);
bool insert_table_col(db_table *table, char *name, enum coltype type);
bool new_database_table(database *db, char *name);
bool free_database(database *db);
bool delete_data_table(database *db, char *table_name, size_t key,
		       union coldata *data);
bool update_data_table(database *db, char *table_name, size_t key,
		       union coldata *data);
db_table *get_table_by_name(database *db, char *name);
bool insert_data_table(database *db, char *table_name, size_t key,
		       union coldata *data);
database *new_database(char *name);
bool insert_col_table(database *db, char *table_name, char *col_name,
		      enum coltype type);
int exec_select_query(database *db, char *sql);
int exec_insert_query(database *db, char *sql);
#endif
