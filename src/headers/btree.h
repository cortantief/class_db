#ifndef BTREE_H
#define BTREE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
// #include "table.h"

enum coltype {
    Nil,
    STRING,
    INT,
};

union coldata {
    char *str;
    int32_t i32;
};

typedef struct {
    enum coltype type;
    char *name;
} db_col;

typedef struct btree_node {
    union coldata *data;
    size_t key;
    struct btree_node *left;
    struct btree_node *right;
} btree_node;

typedef struct {
    btree_node *root;
} btree;

typedef enum operation_result {
    SUCCESS,
    ERROR,
    OVERFLOW,
    UNDERFLOW,
} operation_result;

void print_btree_node(btree_node *node, int depth);
btree_node *new_node(union coldata *data, size_t k);
operation_result insert_data(btree_node *tree, union coldata *data, size_t k);
btree_node *search_node(btree_node *tree, size_t k);
#endif
