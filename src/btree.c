#include "btree.h"
#include "table.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

btree_node *new_node(union coldata *data, size_t k) {
    btree_node *node;

    node = malloc(sizeof(btree_node));
    if (node == NULL)
	return NULL;
    node->key = k;
    node->left = NULL;
    node->right = NULL;
    node->data = data;
    return node;
}

operation_result insert_data(btree_node *tree, union coldata *data, size_t k) {
    btree_node *tmp = NULL;
    if (tree == NULL)
	return ERROR;

    tmp = tree;
    while (tmp != NULL) {
	if (tmp->key == k) {
	    tmp->data = data;
	    return SUCCESS;
	}
	if (tmp->key < k) {
	    if (tmp->left == NULL) {
		tmp->left = new_node(data, k);
		return tmp->left == NULL ? ERROR : SUCCESS;
	    }
	    tmp = tmp->left;
	} else if (tmp->key > k) {
	    if (tmp->right == NULL) {
		tmp->right = new_node(data, k);
		return tmp->right == NULL ? ERROR : SUCCESS;
	    }
	    tmp = tmp->right;
	}
    }
    return ERROR;
}

btree_node *search_node(btree_node *tree, size_t k) {
    if (k == 0)
	return NULL;

    btree_node *cnode = tree;
    while (cnode != NULL) {
	if (cnode->key == k)
	    return cnode;
	else if (cnode->key < k)
	    cnode = cnode->left;
	else
	    cnode = cnode->right;
    }
    return cnode;
}

