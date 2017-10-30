CFLAGS = -m64 -std=c99 -pedantic -Wall -Wshadow -Wpointer-arith -Wcast-qual \
        -Wstrict-prototypes -Wmissing-prototypes -g -O2

all: librandombytes.a

librandombytes.a: randombytes.o
	$(AR) -rcs librandombytes.a randombytes.o

.PHONY: clean
clean:
	$(RM) librandombytes.a randombytes.o
