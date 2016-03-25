#ifndef _compress_h
#define _compress_h

#include <stdio.h>

void compress_file(FILE *ifile, FILE *ofile);
void print_frequency(long long *frequency);

#define DIFFERENT_SYMBOL 256
#define BIT_COUNT 8

#endif
