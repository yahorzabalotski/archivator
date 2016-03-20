#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "decompress.h"
#include "compress.h"
#include "dbg.h"
#include "utill.h"
#include "code.h"

#define BUFFER_SIZE 1024

static void read_frequency(long long *frequency, FILE *ifile);
static void decode_file(long long *frequency, FILE *ifile, const char *ofile_name);

void decompress_file(const char *ifile_name, const char *ofile_name)
{
	FILE *ifile = fopen(ifile_name, "r");
	if(ifile == NULL) {
		log_info("Can't open '%s' file.", ifile_name);
		return;
	}

	long long *frequency = malloc(sizeof(*frequency) * DIFFERENT_SYMBOL);
	if(frequency != NULL) {
		read_frequency(frequency, ifile);
		decode_file(frequency, ifile, ofile_name);
		free(frequency);
	}

	fclose(ifile);
}

static void read_frequency(long long *frequency, FILE *ifile)
{
	int size = DIFFERENT_SYMBOL / BIT_COUNT;
	uint8_t *table = malloc(sizeof(*table) * size);

	if(table != NULL) {
		fread(table, sizeof(*table), size, ifile);
		bzero(frequency, sizeof(*frequency) * DIFFERENT_SYMBOL);

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

static void decode_file(long long *frequency, FILE *ifile, const char *ofile_name)
{
	FILE *ofile = fopen(ofile_name, "w");
	if(ofile == NULL) {
		log_info("Can't create '%s' file.", ofile_name);
		return;
	}

	uint8_t *input_buff = malloc(sizeof(*input_buff) * BUFFER_SIZE);
	uint8_t *output_buff = malloc(sizeof(*output_buff) * BUFFER_SIZE);

	if(input_buff && output_buff) {

		size_t pos = ftell(ifile);
		fseek(ifile, -1L, SEEK_END); 
		uint8_t last_bit_count = 0;
		fread(&last_bit_count, sizeof(last_bit_count), 1, ifile);
		long len = ftell(ifile) - pos - 2;
		fseek(ifile, pos, SEEK_SET);

		Node *root = build_tree(frequency, DIFFERENT_SYMBOL);
		Node *tmp = root;

		uint8_t mask = 0x80;
		int shift = 0;

		int j = 0;
		int k = 0;
		for(int i = 0; i < len / BUFFER_SIZE; i++) {
			fread(input_buff, sizeof(*input_buff), BUFFER_SIZE, ifile);

			j = 0;
			while(j < BUFFER_SIZE) {
				while(tmp->left || tmp->right) {
					if(input_buff[j] & mask) {
						tmp = tmp->right;
					} else {
						tmp = tmp->left;
					}

					input_buff[j] <<= 1;
					++shift;

					if(shift == BIT_COUNT) {
						shift = 0;
						j++;

						if(j == BUFFER_SIZE) {
							break;
						}
					}
				}

				if(!tmp->left && !tmp->right) {
					output_buff[k] = (uint8_t)tmp->num;
					tmp = root;
					++k;

					if(k == BUFFER_SIZE) {
						fwrite(output_buff, sizeof(*output_buff), BUFFER_SIZE, ofile);
						k = 0;
					}

				}
			}
		}

		int size = len % BUFFER_SIZE;
		fread(input_buff, sizeof(*input_buff), size, ifile);
		j = 0;
		while(j < size) {
			while(tmp->left || tmp->right) {
				if(input_buff[j] & mask) {
					tmp = tmp->right;
				} else {
					tmp = tmp->left;
				}


				input_buff[j] <<= 1;
				++shift;

				if(shift == BIT_COUNT) {
					shift = 0;
					j++;

					if(j == size) {
						break;
					}
				}
			}

			if(!tmp->left && !tmp->right) {
				output_buff[k] = (uint8_t)tmp->num;
				tmp = root;
				++k;

				if(k == BUFFER_SIZE) {
					fwrite(output_buff, sizeof(*output_buff), BUFFER_SIZE, ofile);
					k = 0;
				}

			}
		}

		fread(input_buff, sizeof(*input_buff), 1, ifile);

		while(last_bit_count != 0) {
			while(tmp->left || tmp->right) {
				if(*input_buff & mask) {
					tmp = tmp->right;
				} else {
					tmp = tmp->left;
				}

				*input_buff <<= 1;
				--last_bit_count;

				if(last_bit_count == 0) {
					break;
				}
			}

			if(!tmp->left && !tmp->right) {
				output_buff[k] = (uint8_t)tmp->num;
				tmp = root;
				++k;

				if(k == BUFFER_SIZE) {
					fwrite(output_buff, sizeof(*output_buff), BUFFER_SIZE, ofile);
					k = 0;
				}
			}
		}

		if(k != 0) {
			fwrite(output_buff, sizeof(*output_buff), k, ofile);
		}
		delete_huffman_tree(root);

	} else {
		log_info("Can't allocate memory.");
	}

	free(input_buff);
	free(output_buff);

	fclose(ofile);
}
