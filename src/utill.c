#include <stdio.h>
#include <stdint.h>

#include "utill.h"

long get_file_size(FILE *file)
{
	long pos = ftell(file);
	fseek(file, 0L, SEEK_END);
	long res = ftell(file) - pos;
	fseek(file, pos, SEEK_SET);
	return res;
}

long read_buffer(FILE *ifile, uint8_t *buffer, long size)
{
	size_t pos = ftell(ifile);
	fseek(ifile, 0L, SEEK_END);
	int len = ftell(ifile) - pos;

	long read_count = (size > len) ? (len) : (size);  

	fseek(ifile, pos, SEEK_SET);
	fread(buffer, sizeof(*buffer), read_count, ifile);

	return read_count;
}
