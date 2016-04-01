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

static uint8_t convert(char *str)
{
	uint8_t chunk = 0;
	int i;
	for(i = 0; str[i] != '\0' && i < BIT_COUNT; i++) {
		chunk <<= 1;
		if(str[i] == '1') {
			chunk += 1;
		}
	}

	chunk <<= (BIT_COUNT - i);

	return chunk;
}

static void get_code(Code *code, char *str)
{
	int len = strlen(str);
	code->len = len / BIT_COUNT; 

	if(len % BIT_COUNT) {
		code->len += 1;
	}

	code->free_bit = code->len * BIT_COUNT - len;

	code->buff = malloc(sizeof(*code->buff) * code->len);
	for(int i = 0; i < code->len; i++) {
		code->buff[i] = convert(str);
		str += BIT_COUNT;
	}
}

static void appropriate_char_code(Node *root, char *str)
{
	if(root) {
		if(!root->left && !root->right) {
			get_code(root->code, str);
		}
		int len = strlen(str);
		char *new_code = malloc(sizeof(*new_code) * (len + 2));
		strcpy(new_code, str);
		new_code[len + 1] = '\0';
		new_code[len] = '0';
		appropriate_char_code(root->left, new_code);
		new_code[len] = '1';
		appropriate_char_code(root->right, new_code);
		free(new_code);
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
		appropriate_char_code(root, "");
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
