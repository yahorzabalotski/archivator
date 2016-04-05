/**
 * @file huff.c
 * @brief This file is the main unit of the application.
 * @author Yahor Zabolotski
 * @date 2016-03-28
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compress.h"
#include "decompress.h"
#include "dbg.h"

/**
 * @brief print usage message
 *
 * @param name a name of the program
 * @param compress a name of the compression option
 * @param decompress a name of the decompression option
 */
static void print_usage(const char * const name, const char * const compress,
		const char * const decompress);

/**
 * @brief Main function of the software.
 *
 * Parse command line arguments, check option name and call corresponding 
 * compress or decompress function.
 */
int main(int argc, char *argv[])
{
	const char * const name 		= argv[0];
	const char * const compress 	= "-c";
	const char * const decompress 	= "-x";

	if(argc != 4) {
		print_usage(name, compress, decompress);
		return 0;
	}

	const char * const ifile_name 	= argv[1];
	const char * const opt	 		= argv[2];
	const char * const ofile_name 	= argv[3];

	FILE *ifile = fopen(ifile_name, "r");
	if(ifile == NULL) {
		log_err("Can't open '%s' file.", ifile_name);
		exit(0);
	}

	FILE *ofile = fopen(ofile_name, "w");
	if(ofile == NULL) {
		log_err("Can't open '%s' file.", ofile_name);
		fclose(ifile);
		exit(0);
	}

	if(!strcmp(compress, opt)) {
		compress_file(ifile, ofile);
	} else if(!strcmp(decompress, opt)) {
		decompress_file(ifile, ofile);	
	} else {
		printf("Can't recognized %s option.\n", opt);
		print_usage(name, compress, decompress);
	}

	fclose(ifile);
	fclose(ofile);

	exit(0);
}

static void print_usage(const char * const name, const char * const compress,
		const char * const decompress)
{
	printf("Usage: %s ifile [%s|%s] ofile.\n", name, compress, decompress);
	printf("\t%s - compress\n", compress);
	printf("\t%s - decompress\n", decompress);
}
