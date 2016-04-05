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
