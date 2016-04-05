/**
 * @file decompress.c
 * @brief decompress.h implementation
 * @author Yahor Zabolotski
 * @date 2016-04-05
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "decompress.h"
#include "dbg.h"
#include "utill.h"
#include "code.h"

/**
 * @brief read frequency from the file
 *
 * @param frequency pointer to the frequency buffer, which should be no less than DIFFERENT_SYMBOL
 * @param ifile 	input file
 */
static void read_frequency(long long *frequency, FILE *ifile);

/**
 * @brief decode code from file and write source symbols to ofile
 *
 * @param frequency array of frequency for every bytes, amount of element equal DIFFERENT_SYMBOL
 * @param ifile input file
 * @param ofile output file
 */
static void decode_file(long long *frequency, FILE *ifile, FILE *ofile);

void decompress_file(FILE *ifile, FILE *ofile)
{
	long long *frequency = malloc(sizeof(*frequency) * DIFFERENT_SYMBOL);
	if(frequency != NULL) {
		read_frequency(frequency, ifile);
		decode_file(frequency, ifile, ofile);
		free(frequency);
	}
}

static void read_frequency(long long *frequency, FILE *ifile)
{
	int size = DIFFERENT_SYMBOL / BIT_COUNT;
	uint8_t *table = calloc(size, sizeof(*table));

	if(table != NULL) {
		fread(table, sizeof(*table), size, ifile);

		uint8_t mask = 0x80;
		for(int i = 0; i < size; i++) {
			for(int j = 0 ; j < BIT_COUNT; j++) {
				if(table[i] & mask) {
					fread(&frequency[i * BIT_COUNT + j], sizeof(*frequency), 1, ifile);
				}
				table[i] <<= 1;
			}
		}
	}

	free(table);
}

static void decode_file(long long *frequency, FILE *ifile, FILE *ofile)
{
	uint8_t input_buff[BUFFER_SIZE] = {};
	uint8_t output_buff[BUFFER_SIZE] = {};

	Node *root = build_tree(frequency, DIFFERENT_SYMBOL);
	Node *tmp = root;

	long count = 0;
	fread(&count, sizeof(count), 1, ifile);

	uint8_t mask = 0x80;
	int size = get_file_size(ifile);
	int shift = 0;
	int i = 0;
	int j = 0;
	int k = 0;

	int full_buffer_count = size / BUFFER_SIZE;
	if(size % BUFFER_SIZE == 0) {
		--size;
	}

	int output_buff_write_count = 0;

	for(i = 0; i < full_buffer_count; ++i) {
		fread(input_buff, sizeof(*input_buff), BUFFER_SIZE, ifile);

		for(j = 0; j < BUFFER_SIZE; ++j) {
			for(shift = BIT_COUNT; shift > 0; --shift) {
				tmp = (input_buff[j] & mask) ? (tmp->right) : (tmp->left);
				input_buff[j] <<= 1;

				if(tmp->left == NULL) {
					output_buff[k] = (uint8_t) tmp->num;
					tmp = root;
					if(++k == BUFFER_SIZE) {
						++output_buff_write_count;
						fwrite(output_buff, sizeof(*output_buff), k, ofile);
						k = 0;
					}
				}
			}
		}
	}

	count -= output_buff_write_count * BUFFER_SIZE + k;

	shift = 0;

	int last_buffer_size = size % BUFFER_SIZE;
	if(last_buffer_size == 0) {
		last_buffer_size = BUFFER_SIZE;
	}

	fread(input_buff, sizeof(*input_buff), size % BUFFER_SIZE, ifile);

	for(j = 0; j < last_buffer_size && count > 0;) {
		for(; shift < BIT_COUNT;) {
			tmp = (input_buff[j] & mask) ? (tmp->right) : (tmp->left);
			input_buff[j] <<= 1;
			++shift;

			if(tmp->left == NULL) {
				output_buff[k] = (uint8_t) tmp->num;
				tmp = root;
				--count;
				if(++k == BUFFER_SIZE) {
					fwrite(output_buff, sizeof(*output_buff), k, ofile);
					k = 0;
				}
				break;
			}

		}

		if(shift == BIT_COUNT) {
			shift = 0;
			++j;
		}
	}

	if(k != 0) {
		fwrite(output_buff, sizeof(*output_buff), k, ofile);
	}

	if(count != 0) {
		log_info("Incorrect ifile format.");
	}

	delete_huffman_tree(root);
}
