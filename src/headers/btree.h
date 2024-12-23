#ifndef BTREE_H
#define BTREE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct btree_node {
	void *data;
	size_t key;
	struct btree_node* left;
	struct btree_node* right;
} btree_node;


typedef struct {
	btree_node* root;
} btree;

typedef enum operation_result {
    SUCCESS,
    ERROR,
    OVERFLOW,
    UNDERFLOW,
} operation_result;

// void print_btree_node(btree_node *node, int depth);
void print_btree(const btree *tree);
btree_node* new_node(void* data, size_t k);
operation_result insert_data(btree* tree, void* data, size_t k);
btree_node* search_node(btree* tree, size_t k);
void print_btree_node(btree_node *node, int depth);
#endif
