#include "database.h"
#include "btree.h"
#include "constants.h"
#include "table.h"
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

bool insert_data_table(database *db, char *table_name, size_t key,
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
    if (table->root != NULL)
	return _insert_data(table->root, data, key);
    table->root = new_node(data, key);
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
