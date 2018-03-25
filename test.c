#include "randombytes.h"

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void randombytes_checked(uint8_t *buf, size_t n) {
	int ret = randombytes(buf, n);
	if (ret != 0) {
		printf("Error in `randombytes`: %d\n", errno);
		abort();
	}
}

int main(void)
{
	uint8_t buf1[20] = {0}, buf2[20] = {0};

	randombytes_checked(buf1, sizeof(buf1));
	assert(memcmp(buf1, buf2, 20) != 0);
	randombytes_checked(buf2, sizeof(buf2));
	assert(memcmp(buf1, buf2, 20) != 0);

	for (unsigned int i = 0; i < sizeof(buf1); ++i) {
		printf("%02hhx", buf1[i]);
	}
	printf("\n");
	for (unsigned int i = 0; i < sizeof(buf2); ++i) {
		printf("%02hhx", buf2[i]);
	}
	printf("\n");
	return 0;
}
