#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

#include "dbg.h"
#include "compress.h"
#include "code.h"
#include "utill.h"

#define BUFFER_SIZE 1024

static long long *get_frequency(FILE *ifile);
static void write_frequency(long long *frequency, FILE *ofile);
static void write_file(Code *code, FILE *ifile, FILE *ofile);

void compress_file(FILE *ifile, FILE *ofile)
{
	long long *frequency = get_frequency(ifile);
	if(frequency == NULL) {
		log_err("Can't calculate frequency.");
		return;
	}

	rewind(ifile);
	Code *code = generate_code(frequency, DIFFERENT_SYMBOL);
	write_frequency(frequency, ofile);
	write_file(code, ifile, ofile);

	// free code
	for(int i = 0; i < DIFFERENT_SYMBOL; i++) {
		if(code[i].buff) {
			free(code[i].buff);
		}
	}
	free(code);

	free(frequency);
}

static long long *get_frequency(FILE *ifile)
{
	long long *frequency = calloc(DIFFERENT_SYMBOL, sizeof(*frequency));
	if(frequency == NULL) {
		log_info("Can't allocate memory.");
		return NULL;
	}

	long pos = ftell(ifile); // save file pointer

	uint8_t *buffer = malloc(sizeof(*buffer) * BUFFER_SIZE);

	if(buffer != NULL) {
		long len = 0;

		while((len = read_buffer(ifile, buffer, sizeof(*buffer) * BUFFER_SIZE))) { 
			for(int i = 0; i < len; i++) {
				++frequency[buffer[i]];
			}
		}

	} else {
		puts("Can't allocate memory for buffer.");
	}
	
	free(buffer);
	fseek(ifile, pos, SEEK_SET); //restore file pointer
	return frequency;
}

static void write_frequency(long long *frequency, FILE *ofile)
{
	int size = DIFFERENT_SYMBOL / BIT_COUNT;
	uint8_t *table = calloc(size, sizeof(*table)); 
	if(table == NULL) {
		log_info("Can't allocate memory for bit table.");
		return;
	}

	int i = 0;
	for(i = 0; i < size; i++) {
		for(int j = 0; j < BIT_COUNT; j++) {
			table[i] <<= 1;
			if(frequency[i * BIT_COUNT + j]) {
				table[i]++;
			}
		}
	}

	fwrite(table, sizeof(*table), size, ofile);

	for(i = 0; i < DIFFERENT_SYMBOL; i++) {
		if(frequency[i]) {
			fwrite(&frequency[i], sizeof(*frequency), 1, ofile);
		}
	}

	free(table);
}


/*
 * make read_buffer and write_buffer to performe systems calls time
 */

static void write_file(Code *code, FILE *ifile, FILE *ofile)
{
	uint8_t *input_buff = malloc(sizeof(*input_buff) * BUFFER_SIZE);
	if(input_buff == NULL) {
		log_info("Can't allocate memory.");
		return;
	}

	uint8_t *output_buff = calloc(sizeof(*output_buff), BUFFER_SIZE);
	if(output_buff == NULL) {
		log_info("Can't allocate memory.");
		free(input_buff);
		return;
	}

	long len = get_file_size(ifile);
	fwrite(&len, sizeof(len), 1, ofile);

	uint8_t temp = 0;
	int shift = BIT_COUNT;
	int size = read_buffer(ifile, input_buff, BUFFER_SIZE);
	int k = 0;
	Code *byte = NULL;

	while(size) {
		for(long i = 0; i < size; i++) {
			byte = &code[input_buff[i]];
			
			for(int j = 0; j < byte->len - 1; j++) {
				temp = byte->buff[j];
				temp >>= BIT_COUNT - shift;
				output_buff[k] = output_buff[k] | temp;
				++k;
				if(k == BUFFER_SIZE) {
					fwrite(output_buff, sizeof(*output_buff), BUFFER_SIZE, ofile);
					memset(output_buff, 0, sizeof(*output_buff) * BUFFER_SIZE);
					k = 0;
				}
				output_buff[k] = byte->buff[j];
				output_buff[k] <<= shift;
			}	

			temp = byte->buff[byte->len - 1];
			temp >>= BIT_COUNT - shift;
			output_buff[k] = output_buff[k] | temp;
			shift -= BIT_COUNT - byte->free_bit;

			if(shift <= 0) {
				++k;
				if(k == BUFFER_SIZE) {
					fwrite(output_buff, sizeof(*output_buff), BUFFER_SIZE, ofile);
					memset(output_buff, 0, sizeof(*output_buff) * BUFFER_SIZE);
					k = 0;
				}

				output_buff[k] = byte->buff[byte->len - 1];
				shift += BIT_COUNT;
				output_buff[k] <<= shift - byte->free_bit;
			}
		}

		size = read_buffer(ifile, input_buff, BUFFER_SIZE);
	}

	if(shift != BIT_COUNT) {
		++k;
	}
	fwrite(output_buff, sizeof(*output_buff), k, ofile);

	free(input_buff);
	free(output_buff);
}
