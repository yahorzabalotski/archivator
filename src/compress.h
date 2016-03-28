/**
 * @file compress.h
 * @brief Header of the compress functionality 
 * @author Yahor Zabolotski
 * @date 2016-03-28
 */

#ifndef _compress_h
#define _compress_h

#include <stdio.h>

/**
 * @brief compress ifile to ofile
 *
 * @param ifile input file pointer, should be open for read
 * @param ofile output file pointer, should be open for write
 */
void compress_file(FILE *ifile, FILE *ofile);
void print_frequency(long long *frequency);

#define DIFFERENT_SYMBOL 256
#define BIT_COUNT 8

#endif
