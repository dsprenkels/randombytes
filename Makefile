CFLAGS = -Wall -Wpointer-arith -g -O2

all: librandombytes.a

librandombytes.a: randombytes.o
	$(AR) -rcs librandombytes.a randombytes.o

.PHONY: clean
clean:
	$(RM) librandombytes.a randombytes.o
