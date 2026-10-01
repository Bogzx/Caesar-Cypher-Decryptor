CC ?= cc
CFLAGS ?= -std=c99 -Wall -Wextra -Wpedantic -O2
LDLIBS = -lm
# Seconds each end-to-end test may run; sanitizer builds start slowly on some hosts.
TEST_TIMEOUT ?= 5

caesar: main.c
	$(CC) $(CFLAGS) -o $@ main.c $(LDLIBS)

test: caesar
	TEST_TIMEOUT=$(TEST_TIMEOUT) ./tests/run.sh ./caesar

clean:
	rm -f caesar

.PHONY: test clean
