#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "dbg.h"
#include "code.h"
#include "pqueue.h"

#include <limits.h>

#define BIT_COUNT 8


static int Node_compare(const void *a, const void *b)
{
	const Node *node_a = a;
	const Node *node_b = b;
	
	if(node_a->amount < node_b->amount) {
		return -1;
	} else if(node_a->amount == node_b->amount) {
		return 0;
	} else {
		return 1;
	}
}

static Node *create_frequency_nodes(long long *frequency, int size)
{
	Node *nodes = malloc(sizeof(*nodes) * size);
	check(nodes, "Can't allocate memory.");

	for(int i = 0; i < size; i++) {
		nodes[i].left 	= NULL;
		nodes[i].right	= NULL;
		nodes[i].code 	= NULL;
		nodes[i].num 	= i;
		nodes[i].amount = frequency[i];
	}

error:
	return nodes;
}

static Code *create_codes(Node *nodes, int size)
{
	Code *codes = malloc(sizeof(*codes) * size);
	check(codes, "Can't allocate memory.");

	for(int i = 0; i < size; i++) {
		nodes[i].code = &codes[i];
	}

error:
	return codes;
}

static pQueue *push_frequency_nodes(Node *nodes, int size)
{
	pQueue *pq = pQueue_create(Node_compare);
	check(pq, "Can't create pq.");

	for(int i = 0; i < size; i++) {
		if(nodes[i].amount) {
			pQueue_push(pq, &nodes[i]);
		}
	}

error:
	return pq;
}

static Node *build_huffman_tree(pQueue *pq, int size)
{
	Node *left = NULL;
	Node *right = NULL;

	for(int i = 0; i < size - 1; i++) {
		left = pQueue_pop(pq);
		right = pQueue_pop(pq);

		Node *node = malloc(sizeof(*node));
		node->left = left;
		node->right = right;

		node->amount = 0L;
		if(left) {
			node->amount += left->amount;
		}
		if(right) {
			node->amount += right->amount;
		}

		pQueue_push(pq, node);

		if(!right) {
			break;
		}
	}

	return pQueue_pop(pq);
}

void Code_print(Code *code)
{
	printf("FREE BIT: %d LEN: %d\n", code->free_bit, code->len);
	for(int i = 0; i < code->len; i++) {
		printf("%x", code->buff[i]);
	}
	printf("\n");
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
	//printf("str : %s\n" , str);
	int len = strlen(str);
	code->len = len / BIT_COUNT; 

	if(len % BIT_COUNT) {
		code->len += 1;
	}
	//printf("LENGTH: %d '%s'\n", code->len, str);

	code->free_bit = code->len * BIT_COUNT - len;

	code->buff = malloc(sizeof(*code->buff) * code->len);
	for(int i = 0; i < code->len; i++) {
		code->buff[i] = convert(str);
		str += BIT_COUNT;
	}

	//Code_print(code);
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
		//if(root->left || root->right) {
		free(root);
		//}
	}
}

void print_huffman_tree(Node *root)
{
	if(root) {
		print_huffman_tree(root->left);
		if(!root->left && !root->right) {
			printf("%c - %lld\n", root->num, root->amount);
		}
		print_huffman_tree(root->right);
	}
}

int get_count(Node *root)
{
	if(root) {
		return get_count(root->left) + get_count(root->right) + 1;
	}

	return 0;
}



Code *generate_code(long long *frequency, int size)
{
	Code *codes = malloc(sizeof(*codes) * size);
	if(codes == NULL) {
		log_info("Can't allocate memory.");
		return NULL;
	}

	bzero(codes, sizeof(*codes) * size);
	pQueue *pq = pQueue_create(Node_compare);
	if(pq) {
		for(int i = 0; i < size; i++) {
			if(frequency[i]) {
				Node *node = malloc(sizeof(*node));
				node->left = NULL;
				node->right = NULL;
				node->amount = frequency[i];
				node->num = i;
				node->code = &codes[i];
				pQueue_push(pq, node);
			} 	
		}

		Node *root = build_huffman_tree(pq, size);
		//print_huffman_tree(root);
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
			Node *node = malloc(sizeof(*node));
			node->left = NULL;
			node->right = NULL;
			node->amount = frequency[i];
			node->num = i;
			node->code = NULL;
			pQueue_push(pq, node);
		} 	
	}

	Node *root = build_huffman_tree(pq, size);
	pQueue_delete(pq);

	return root;
}
