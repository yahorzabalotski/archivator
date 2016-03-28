/**
 * @file decompress.h
 * @brief Header of the decompress functionality. 
 * @author Yahor Zabolotski
 * @date 2016-03-28
 */

#ifndef _decompress_h
#define _decompress_h

#include <stdio.h>

/**
 * @brief decompress ifile to ofile
 *
 * Decompress ifile to ofile, if ifile has correct file format, otherwise do nothing.
 * @param ifile pointer to input file; should be open for read
 * @param ofile pointer to output file; should be open for write
 */
void decompress_file(FILE *ifile, FILE *ofile);
#endif
