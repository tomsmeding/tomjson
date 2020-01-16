CC = afl-clang-fast
CFLAGS = -Wall -Wextra -std=c11 -O2 -g -fwrapv -I..

all: fuzz

clean:
	rm -f fuzz


fuzz: fuzz.c
	$(CC) $(CFLAGS) -o $@ $< ../libtomjson.a
