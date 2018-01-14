#include "randombytes.h"
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>

int main(void)
{
    // Generate some random bytes and print them in hex
    int ret;
    uint8_t buf[20];
    size_t i;

    ret = randombytes(buf, sizeof(buf));
    if (ret != 0) {
        printf("Error in `randombytes`: %d\n", errno);
        return 1;
    }
    for (i = 0; i < sizeof(buf); ++i) {
        printf("%02hhx", buf[i]);
    }
    printf("\n");
    return 0;
}
