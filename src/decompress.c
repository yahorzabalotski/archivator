#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "decompress.h"
#include "compress.h"
#include "dbg.h"
#include "utill.h"
#include "code.h"

static void read_frequency(long long *frequency, FILE *ifile);
static void decode_file(long long *frequency, FILE *ifile, const char *ofile_name);

void decompress_file(const char *ifile_name, const char *ofile_name)
{
	FILE *ifile = fopen(ifile_name, "r");
	check(ifile, "Can't open '%s' file.", ifile_name);

	long long *frequency = malloc(sizeof(*frequency) * DIFFERENT_SYMBOL);
	check(frequency, "Can't allocate memory.");
	
	read_frequency(frequency, ifile);
	//printf("READ FROM IFILE FREQUENCES\n");
	//print_frequency(frequency);
	decode_file(frequency, ifile, ofile_name);

	free(frequency);
error:
	fclose(ifile);
}

static void read_frequency(long long *frequency, FILE *ifile)
{
	int size = DIFFERENT_SYMBOL / BIT_COUNT;
	uint8_t *table = malloc(sizeof(*table) * size);
	check(table, "Can't allocate memory.");
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

error:
	free(table);
}

static void decode_file(long long *frequency, FILE *ifile, const char *ofile_name)
{
	FILE *ofile = fopen(ofile_name, "w");
	check(ofile, "Can't create '%s' file.", ofile_name);

	size_t pos = ftell(ifile);
	fseek(ifile, -1L, SEEK_END); 
	uint8_t sign_bit = 0;
	fread(&sign_bit, sizeof(sign_bit), 1, ifile);
	long len = ftell(ifile) - pos - 2;
	fseek(ifile, pos, SEEK_SET);

	Node *root = build_tree(frequency, DIFFERENT_SYMBOL);
	Node *tmp = NULL;
	uint8_t buff = 0;
	uint8_t out = 0;
	uint8_t mask = 0x80;
	int shift = 0;

	printf("LEN: %ld\n", len);
	fread(&buff, sizeof(buff), 1, ifile);
	while(len) {
		tmp = root;
		while(tmp->right || tmp->left) {
			if(buff & mask) {
				tmp = tmp->right;
			} else {
				tmp = tmp->left;
			}

			buff <<= 1;
			shift++;
			if(shift == BIT_COUNT) {
				shift = 0;
				fread(&buff, sizeof(buff), 1, ifile);
				len--;
				if(!len) {
					goto lastiteration;
				}
			}
		}
		
		out = (uint8_t)tmp->num;
		fwrite(&out, sizeof(out), 1, ofile);
	}


lastiteration:
	if(!tmp || (!tmp->left && !tmp->right)) {
		tmp = root;
	}
	if(!sign_bit) {
		sign_bit = BIT_COUNT;
	}
	//printf("SIGN %d\n" , sign_bit);
	while(sign_bit) {
		while(tmp->right || tmp->left) {
			if(buff & mask) {
				tmp = tmp->right;
			} else {
				tmp = tmp->left;
			}

			buff <<= 1;
			sign_bit--;
		}
		
		out = (uint8_t)tmp->num;
		fwrite(&out, sizeof(out), 1, ofile);
		tmp = root;
	}

error:
	fclose(ofile);
	delete_huffman_tree(root);
}
