#include "randombytes.h"
#include <stdio.h>
#include <stdlib.h>

int main()
{
	char *buf;
	buf = calloc(1024*1024*500, sizeof(char));

	size_t idx;

	randombytes(buf, 1024*1024*500);
	for (idx = 0; idx < 1024*1024*500; idx++) {
		// printf("%02hhx", buf[idx]);
	}
	printf("\n");

	free(buf);
	return 0;
}
