/**
 * @file code.h
 * @brief Build the Huffman tree and generate codes for symbols
 * @author Yahor Zabolotski
 * @date 2016-03-31
 */

#ifndef _code_h
#define _code_h

#include <stdint.h>

/**
 * @brief This structure uses for represent the Huffman code for a symbol.
 */
typedef struct Code {
	int free_bit; 	/**< number of bits, which unused in last buff element */
	int len;		/**< amount of element in the buff array */
	uint8_t *buff;	/**< pointer to array, which hold the bit code representation*/
} Code;

/**
 * @brief This structure uses for build the Huffman tree.
 */
typedef struct Node {
	struct Node *left;	/**< pointer to left son */
	struct Node *right;	/**< pointer to right son */

	int num;			/**< number of element in the frequency array */
	long long amount;	/**< frequency of the num */
	Code *code;			/**< pointer to code representation for num */
} Node;

/**
 * @brief generate code for appropriate frequency
 *
 * @param frequency pointer to frequency array
 * @param size frequency array size
 *
 * @return pointer to codes array with length equal to the size 
 */
Code *generate_code(long long *frequency, int size);

/**
 * @brief build Huffman tree for frequency
 *
 * @param frequency pointer to frequency array
 * @param size frequency array size
 *
 * @return pointer to the tree root
 */
Node *build_tree(long long *frequency, int size);

/**
 * @brief remove the Huffman tree
 *
 * @param root pointer to the Huffman tree root
 */
void delete_huffman_tree(Node *root);

#endif
