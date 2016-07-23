CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g -fwrapv

BIN = test


.PHONY: all clean remake run rerun

all: $(BIN)

clean:
	rm -f $(BIN)

remake: clean all

run: all
	./$(BIN) -q

rerun: clean run


$(BIN): $(wildcard *.c *.h)
	$(CC) $(CFLAGS) -o $@ $(filter %.c,$^) ../libtomjson.a
