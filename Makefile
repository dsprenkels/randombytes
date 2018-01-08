CFLAGS += -g -O2 -m64 -std=c99 -pedantic \
	-Wall -Wshadow -Wpointer-arith -Wcast-qual -Wformat -Wformat-security \
	-Werror=format-security -Wstrict-prototypes -Wmissing-prototypes \
	-D_FORTIFY_SOURCE=2 -fPIC -fno-strict-overflow

all: librandombytes.a

librandombytes.a: randombytes.o
	$(AR) -rcs librandombytes.a randombytes.o

test: randombytes.o

.PHONY: check
check: test
	./test

.PHONY: clean
clean:
	$(RM) librandombytes.a randombytes.o test test.o
