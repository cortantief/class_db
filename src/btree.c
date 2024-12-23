#include "btree.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

btree_node* new_node(void* data, size_t k) {
	btree_node* node;

	node = malloc(sizeof(btree_node));
	if (node == NULL)
		return NULL;
	node->key = k;
	node->left = NULL;
	node->right = NULL;
	node->data = data;
}

operation_result insert_data(btree* tree, void* data, size_t k) {
	btree_node* tmp = NULL;
	btree_node* parent = NULL;
	if (tree == NULL)
		return ERROR;
	
	if (tree->root == NULL) {
		if ((tree->root = new_node(data, k)) == NULL)
			return ERROR;
		return SUCCESS;
	}

	tmp = tree->root;
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
		} else if(tmp->key > k) {
			if(tmp->right == NULL) {
				tmp->right = new_node(data, k);
				return tmp->right == NULL ? ERROR: SUCCESS;
			}
			tmp = tmp->right;
		}
	}
	
}


void print_btree_node(btree_node *node, int depth) {
    if (node == NULL) {
        return;
    }
    print_btree_node(node->right, depth + 1);

    for (int i = 0; i < depth; i++) {
        printf("    ");
    }

    printf("%zu\n", node->key);
    print_btree_node(node->left, depth + 1);
}

void print_btree(const btree *tree) {
    if (tree == NULL || tree->root == NULL) {
        printf("The tree is empty.\n");
        return;
    }
    print_btree_node(tree->root, 0);
}
