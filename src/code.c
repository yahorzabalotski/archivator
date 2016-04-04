#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "dbg.h"
#include "code.h"
#include "pqueue.h"
#include "extention.h"

#include <limits.h>

/**
 * @brief Compare two Node element.
 *
 * @param a pointer to the first Node element
 * @param b pointer to the second Node element
 *
 * @return 1 if *a > *b, 0 if *a == *b, -1 either
 */
static int Node_compare(const void *a, const void *b)
{
	const Node *pa = a;
	const Node *pb = b;
	
	return (pa->amount > pb->amount) - (pb->amount > pa->amount);
}

static Node *Node_create(long long amount, int num, Code *code)
{
	Node *node = malloc(sizeof(*node));
	if(node) {
		*node = (Node){	.left = NULL, 
						.right = NULL,
					   	.num = num, 
						.amount = amount, 
						.code = code}; 
	}

	return node;
}

static Node *build_huffman_tree(pQueue *pq, int size)
{
	Node *left = pQueue_pop(pq);
	if(left == NULL) {
		return NULL;
	}

	Node *right = pQueue_pop(pq);

	while(right) {
		Node *node = malloc(sizeof(*node));
		if(node == NULL) {
			log_info("Can't allocate memory.");
			return NULL;
		}
		
		node->left 	= left;
		node->right = right;

		node->amount = 0L;
		if(left) {
			node->amount += left->amount;
		}
		if(right) {
			node->amount += right->amount;
		}

		pQueue_push(pq, node);
		left = pQueue_pop(pq);
		right = pQueue_pop(pq);
	}

	return left;
}

static void appropriate_char_code(Node *root, uint8_t *buff, int sign_bit_count)
{
	if(root) {
		int len = (sign_bit_count + BIT_COUNT - 1) / BIT_COUNT;
		if(!root->left && !root->right) {
			root->code->buff = malloc(sizeof(*(root->code->buff)) * len);
			root->code->len = len;
			root->code->free_bit = BIT_COUNT - sign_bit_count % BIT_COUNT;
			if(root->code->free_bit == BIT_COUNT) {
				root->code->free_bit = 0;
			}
			memcpy(root->code->buff, buff, len);
			uint8_t mask = 0xFF >> (BIT_COUNT - root->code->free_bit);
			root->code->buff[len - 1] &= ~mask;
			/*
			for(int j = root->code->free_bit - 1; j >= 0; j--) {
				root->code->buff[len - 1] &= ~(1 << j);
			}
			*/
		}
		
		buff[sign_bit_count / BIT_COUNT] &= ~(1 << (BIT_COUNT - sign_bit_count % BIT_COUNT - 1));
		appropriate_char_code(root->left, buff, sign_bit_count + 1);

		buff[sign_bit_count / BIT_COUNT] |= (1 << (BIT_COUNT - sign_bit_count % BIT_COUNT - 1));
		appropriate_char_code(root->right, buff, sign_bit_count + 1);
	}
}

void delete_huffman_tree(Node *root)
{
	if(root) {
		delete_huffman_tree(root->left);
		delete_huffman_tree(root->right);
		free(root);
	}
}

static Code* push_init_nodes(pQueue *pq, long long *frequency, int size)
{
	if(pq == NULL) {
		return NULL;
	}

	Code *codes = calloc(size, sizeof(*codes));
	if(codes == NULL) {
		log_info("Can't allocate memory.");
		return NULL;
	}

	int i = 0;
	for(; i < size; i++) {
		if(frequency[i]) {
			Node *node = Node_create(frequency[i], i, &codes[i]);
			if(node == NULL) {
				log_info("Can't allocate memory.");
				break;
			}

			pQueue_push(pq, node);
		} 	
	}

	if(i != size) {
		return NULL;
	}

	return codes;
}


Code *generate_code(long long *frequency, int size)
{
	pQueue *pq = pQueue_create(Node_compare);
	if(pq == NULL) {
		return NULL;
	}
	
	Code *codes = push_init_nodes(pq, frequency, size);
	if(codes != NULL) {
		Node *root = build_huffman_tree(pq, size);
		uint8_t buff[DIFFERENT_SYMBOL * 2] = {};
		appropriate_char_code(root, buff, 0);
		delete_huffman_tree(root);
	}
	pQueue_delete(pq);
	return codes;
}

Node *build_tree(long long *frequency, int size)
{
	pQueue *pq = pQueue_create(Node_compare);
	for(int i = 0; i < size; i++) {
		if(frequency[i]) {
			Node *node = Node_create(frequency[i], i, NULL);
			pQueue_push(pq, node);
		} 	
	}

	Node *root = build_huffman_tree(pq, size);
	pQueue_delete(pq);

	return root;
}
