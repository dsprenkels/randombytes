CFLAGS += -g -O2 -m64 -std=c99 -pedantic \
	-Wall -Wshadow -Wpointer-arith -Wcast-qual -Wformat -Wformat-security \
	-Werror=format-security -Wstrict-prototypes -Wmissing-prototypes \
	-D_FORTIFY_SOURCE=2 -fPIC -fno-strict-overflow

all: librandombytes.a

librandombytes.a: randombytes.o
	$(AR) -rcs librandombytes.a randombytes.o

randombytes.js: CC := ${EMSCRIPTEN}/emcc
randombytes.js: CFLAGS += -Wno-dollar-in-identifier-extension
randombytes.js: randombytes.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

test: randombytes.o test.o

test.js: CC := ${EMSCRIPTEN}/emcc
test.js: CFLAGS += -Wno-dollar-in-identifier-extension
test.js: randombytes.c test.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

test: randombytes.o test.o

.PHONY: check
check: test
	./test

.PHONY: check
check-js: test.js
	node test.js

.PHONY: clean
clean:
	$(RM) librandombytes.a randombytes.o randombytes.js test test.o test.js
