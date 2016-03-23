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
static void encoded_file(long long *frequency,  const char *ofile_name, FILE *ifile);
static void write_frequency(long long *frequency, FILE *ofile);
static void write_file(Code *code, FILE *ifile, FILE *ofile);

void print_frequency(long long *frequency);
static void print_code(long long *frequency, Code *codes);

void compress_file(const char *ifile_name, const char *ofile_name)
{
	FILE *ifile = fopen(ifile_name, "r");
	if(!ifile) {
		printf("Can't open '%s' file.\n", ifile_name);
		return ;
	}

	long long *frequency = get_frequency(ifile);
	if(frequency) {
		rewind(ifile);
		encoded_file(frequency, ofile_name, ifile);
	} 

	free(frequency);
	fclose(ifile);
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

		for(int j = 0; j < len / BUFFER_SIZE; j++) {
			fread(buffer, sizeof(*buffer), BUFFER_SIZE, ifile);
			for(int i = 0; i < BUFFER_SIZE; i++) {
				++frequency[buffer[i]];
			}
		}

		int last = len % BUFFER_SIZE;
		fread(buffer, sizeof(*buffer), last, ifile);
		for(int i = 0; i < last; i++) {
			++frequency[buffer[i]];
		}
	} else {
		puts("Can't allocate memory for buffer.");
	}
	
	free(buffer);
	fseek(ifile, pos, SEEK_SET); //restore file pointer
	return frequency;
}


static void encoded_file(long long *frequency, const char *ofile_name, FILE *ifile)
{
	FILE *ofile = fopen(ofile_name, "w");
	check(ofile, "Can't create '%s'.", ofile_name);
	Code *code = generate_code(frequency, DIFFERENT_SYMBOL);
	//print_code(frequency, code);
	write_frequency(frequency, ofile);
	write_file(code, ifile, ofile);

	// free code
	for(int i = 0; i < DIFFERENT_SYMBOL; i++) {
		if(code[i].buff) {
			free(code[i].buff);
		}
	}
	free(code);
error:
	fclose(ofile);
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
	long len = get_file_size(ifile);
	fwrite(&len, sizeof(len), 1, ofile);

	uint8_t temp = 0;
	uint8_t buff = 0;
	int shift = BIT_COUNT;
	Code *byte = NULL;

	for(long i = 0; i < len; i++) {
		fread(&temp, sizeof(temp), 1, ifile);
		byte = &code[temp];
		
		for(int j = 0; j < byte->len - 1; j++) {
			temp = byte->buff[j];
			temp >>= BIT_COUNT - shift;
			buff = buff | temp;
			fwrite(&buff, sizeof(buff), 1, ofile);
			buff = byte->buff[j];
			buff <<= shift;
		}	

		temp = byte->buff[byte->len - 1];
		temp >>= BIT_COUNT - shift;
		buff = buff | temp;
		shift -= BIT_COUNT - byte->free_bit;

		if(shift <= 0) {
			fwrite(&buff, sizeof(buff), 1, ofile);
			buff = byte->buff[byte->len - 1];
			shift += BIT_COUNT;
			buff <<= shift - byte->free_bit;
		}

		if(i == len - 1 && shift != BIT_COUNT) {
			fwrite(&buff, sizeof(buff), 1, ofile);
		}
	}
	/*
	uint8_t sign_bit = BIT_COUNT - shift;
	if(shift == BIT_COUNT) {
		sign_bit = BIT_COUNT;
	}
	fwrite(&sign_bit, sizeof(sign_bit), 1, ofile);
	*/
}
// debug function 
void print_frequency(long long *frequency)
{
	for(int i = 0; i < DIFFERENT_SYMBOL; i++) {
		if(frequency[i]) {
			printf("%c - %lld\n", i, frequency[i]);
		}
	}
}

static void print_code(long long *frequency, Code *codes)
{
	for(int i = 0; i < DIFFERENT_SYMBOL; i++) {
		if(frequency[i]) {
			printf("%c - %lld\n", i, frequency[i]);
			Code_print(&codes[i]);
		}
	}
}
