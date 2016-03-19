#ifndef _code_h
#define _code_h

#include <stdint.h>

typedef struct Code {
	int free_bit;
	int len;
	uint8_t *buff;
} Code;

typedef struct Node {
	struct Node *left;
	struct Node *right;

	int num;
	long long amount;
	Code *code;
} Node;

Code *generate_code(long long *frequency, int size);
Node *build_tree(long long *frequency, int size);
void delete_huffman_tree(Node *root);

#endif
