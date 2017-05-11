#include "randombytes.h"
#include <stdio.h>

int main()
{
	char buf[32];
	size_t idx;

	randombytes(buf, sizeof buf);
	for (idx = 0; idx < sizeof buf; idx++) {
		printf("%02hhx", buf[idx]);
	}
	printf("\n");
	return 0;
}
