/**
 * @file compress.c
 * @brief module which implement compres.h file
 * @author Yahor Zabolotski
 * @date 2016-03-28
 */

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

/**
 * @brief calculate byte frequency in the ifile
 *
 * @param ifile file pointer, in which frequency are calculate; should be open for read
 *
 * @return array with byte frequency; amount of elements equal to DIFFERENT_SYMBOL
 */
static long long *get_frequency(FILE *ifile);

/**
 * @brief write byte frequency in ifile
 *
 * @param frequency pointer to byte frequency array with DIFFERENT_SYMBOL amount of element
 * @param f_size size of the frequency array, should be multipe of the number of bits
 * @param ofile pointer to file, where frequency will be write
 */
static void write_frequency(long long *frequency, int f_size, FILE *ofile);

/**
 * @brief encodes ifile to ofile according code for bytes
 *
 * @param code pointer to array of Code, which contain in code[byte] Code for this byte;
 * the array should be DIFFERENT_SYMBOL size
 * @param ifile pointer to source file; should be open for read
 * @param ofile pointer to result file; should be open for write
 */
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
	write_frequency(frequency, DIFFERENT_SYMBOL, ofile);
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
		long len = get_file_size(ifile);

		for(int i = len / BUFFER_SIZE; i > 0; i--) {
			fread(buffer, sizeof(*buffer), BUFFER_SIZE, ifile);
			for(int j = BUFFER_SIZE - 1; j >= 0; j--) {
				++frequency[buffer[j]];
			}
		}

		int size = len % BUFFER_SIZE;
		fread(buffer, sizeof(*buffer), size, ifile);
		for(int i = size - 1; i >= 0; i--) {
			++frequency[buffer[i]];
		}

	} else {
		puts("Can't allocate memory for buffer.");
	}
	
	free(buffer);
	fseek(ifile, pos, SEEK_SET); //restore file pointer
	return frequency;
}

static void write_frequency(long long *frequency, int f_size, FILE *ofile)
{
	int size = f_size / BIT_COUNT;
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

	for(i = 0; i < f_size; i++) {
		if(frequency[i]) {
			fwrite(&frequency[i], sizeof(*frequency), 1, ofile);
		}
	}

	free(table);
}

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
//	int size = read_buffer(ifile, input_buff, BUFFER_SIZE);
	int size = get_file_size(ifile) / BUFFER_SIZE;
	int k = 0;
	Code *byte = NULL;

	for(; size > 0; size--) {
		fread(input_buff, sizeof(*input_buff), BUFFER_SIZE, ifile);
		for(long i = 0; i < BUFFER_SIZE; i++) {
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
	}

	size = get_file_size(ifile); 
	fread(input_buff, sizeof(*input_buff), size, ifile);
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

	if(shift != BIT_COUNT) {
		++k;
	}
	fwrite(output_buff, sizeof(*output_buff), k, ofile);

	free(input_buff);
	free(output_buff);
}
