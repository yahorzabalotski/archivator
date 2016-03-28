#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "decompress.h"
#include "dbg.h"
#include "utill.h"
#include "code.h"

static void read_frequency(long long *frequency, FILE *ifile);
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
	uint8_t *input_buff = malloc(sizeof(*input_buff) * BUFFER_SIZE);
	if(input_buff == NULL) {
		log_info("Can't allocate memory.");
		return;
	}

	uint8_t *output_buff = malloc(sizeof(*output_buff) * BUFFER_SIZE);
	if(output_buff == NULL) {
		log_info("Can't allocate memory.");
		free(input_buff);
		return;
	}

	Node *root = build_tree(frequency, DIFFERENT_SYMBOL);
	Node *tmp = root;

	long count = 0;
	fread(&count, sizeof(count), 1, ifile);

	uint8_t mask = 0x80;
	int shift = 0;
	int size = read_buffer(ifile, input_buff, BUFFER_SIZE);
	int i = 0;
	int j = 0;

	while(count != 0 && size != 0) {
		tmp = root;
		while(tmp->left || tmp->right) {
			if(input_buff[i] & mask) {
				tmp = tmp->right;
			} else {
				tmp = tmp->left;
			}

			input_buff[i] <<= 1;
			++shift;

			if(shift == BIT_COUNT) {
				shift = 0;
				++i;

				if(i == size) {
					size = read_buffer(ifile, input_buff, BUFFER_SIZE);
					i = 0;
				}
			}
		}

		output_buff[j] = (uint8_t)tmp->num;
		++j;
		--count;

		if(j == BUFFER_SIZE) {
			fwrite(output_buff, sizeof(*output_buff), BUFFER_SIZE, ofile);
			j = 0;
		}
	}

	if(j != 0) {
		fwrite(output_buff, sizeof(*output_buff), j, ofile);
	}

	if(count != 0) {
		log_info("Incorrect ifile format.");
	}

	delete_huffman_tree(root);
	free(input_buff);
	free(output_buff);
}
