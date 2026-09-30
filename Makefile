CC ?= cc
CFLAGS ?= -std=c99 -Wall -Wextra -Wpedantic -O2
LDLIBS = -lm

caesar: main.c
	$(CC) $(CFLAGS) -o $@ main.c $(LDLIBS)

test: caesar
	./tests/run.sh ./caesar

clean:
	rm -f caesar

.PHONY: test clean
