#include <stdio.h>
#include <string.h>

#include "compress.h"
#include "decompress.h"

static void print_usage(const char * const name, const char * const compress,
		const char * const decompress);

int main(int argc, char *argv[])
{
	const char * const name 		= argv[0];
	const char * const compress 	= "-c";
	const char * const decompress 	= "-x";

	if(argc != 4) {
		print_usage(name, compress, decompress);
		return 0;
	}

	const char * const ifile = argv[1];
	const char * const opt	 = argv[2];
	const char * const ofile = argv[3];

	if(!strcmp(compress, opt)) {
		printf("Compress %s to %s ...\n", ifile, ofile);
		compress_file(ifile, ofile);
	} else if(!strcmp(decompress, opt)) {
		printf("Decompress %s to %s ...\n", ifile, ofile);
		decompress_file(ifile, ofile);	
	} else {
		printf("Can't recognized %s option.\n", opt);
		print_usage(name, compress, decompress);
	}

	return 0;
}

static void print_usage(const char * const name, const char * const compress,
		const char * const decompress)
{
	printf("Usage: %s ifile [%s|%s] ofile.\n", name, compress, decompress);
	printf("\t%s - compress\n", compress);
	printf("\t%s - decompress\n", decompress);
}
