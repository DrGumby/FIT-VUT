SHELL = /bin/bash
CC = gcc
CFLAGS = -O2 -std=c99 -Wall -Wextra -pedantic -g
.PHONY = primes steg-decode primes-i
PROGRAM1 = eratosthenes.c primes.c error.c
PROGRAM2 = steg-decode.c ppm.c error.c eratosthenes.c
all: primes steg-decode

primes: primes.c
	$(CC) $(CFLAGS) $(PROGRAM1) -o primes -lm
	$(CC) $(CFLAGS) $(PROGRAM1) -o primes-i -lm -DUSE_INLINE
steg-decode: 
	$(CC) $(CFLAGS) $(PROGRAM2) -o steg-decode -lm

run: primes
	time ./primes
clean:
	rm -f $(.PHONY)
