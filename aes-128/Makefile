# Makefile for AES-128 lab

CC = gcc
CFLAGS = -O2 -std=c11

.PHONY: all portable aesni clean

all: portable aesni

portable: aes128_portable.o test_aes_portable
test_aes_portable: aes128_portable.o test_aes.o
	$(CC) $(CFLAGS) -o test_aes_portable aes128_portable.o test_aes.o

aes128_portable.o: aes128_portable.c
	$(CC) $(CFLAGS) -c aes128_portable.c -o aes128_portable.o

test_aes.o: test_aes.c
	$(CC) $(CFLAGS) -c test_aes.c -o test_aes.o

aesni: aes128_portable.o aes128_aesni.o test_aes_aesni
test_aes_aesni: aes128_portable.o aes128_aesni.o test_aes.o
	$(CC) $(CFLAGS) -DUSE_AESNI -maes -msse4.1 -o test_aes_aesni aes128_portable.o aes128_aesni.o test_aes.o

aes128_aesni.o: aes128_aesni.c
	$(CC) $(CFLAGS) -c aes128_aesni.c -o aes128_aesni.o -maes -msse4.1

clean:
	rm -f *.o test_aes_portable test_aes_aesni
