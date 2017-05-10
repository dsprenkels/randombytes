CFLAGS = -Wall -g -O2
SRCS = randombytes.c
OBJS := ${SRCS:.c=.o}

all: librandombytes.a

librandombytes.a: $(OBJS)
	$(AR) -rcs librandombytes.a $^

.PHONY: clean
clean:
	$(RM) *.o *.gch *.a *.out
