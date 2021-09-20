CFLAGS ?= -g -O2 -std=gnu99 \
	-Wall -Wextra -Wshadow -Wpointer-arith -Wcast-qual -Wformat \
	-Wformat-security -Werror=format-security -Wstrict-prototypes \
	-D_FORTIFY_SOURCE=2 -fPIC -fno-strict-overflow

TEST_WRAPS := -Wl,-wrap=ioctl,-wrap=getrandom,-wrap=syscall

ifeq ($(shell uname -o), GNU/Linux)
	TEST_LDFLAGS := $(TEST_WRAPS)
endif

all: librandombytes.a

librandombytes.a: randombytes.o
	$(AR) -rcs librandombytes.a randombytes.o

randombytes.js: CC := ${EMSCRIPTEN}/emcc
randombytes.js: CFLAGS += -Wno-dollar-in-identifier-extension
randombytes.js: randombytes.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

randombytes_test: CFLAGS+=-Wno-implicit-function-declaration
randombytes_test: LDFLAGS:=$(TEST_LDFLAGS)
randombytes_test: randombytes_test.c

randombytes_test.js: CC := ${EMSCRIPTEN}/emcc
randombytes_test.js: CFLAGS += -Wno-dollar-in-identifier-extension
randombytes_test.js: randombytes_test.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

.PHONY: check
check: randombytes_test
	./randombytes_test

.PHONY: check
check-js: randombytes_test.js
	node randombytes_test.js

.PHONY: clean
clean:
	$(RM) librandombytes.a randombytes.o randombytes.js randombytes_test randombytes_test.o randombytes_test.js
