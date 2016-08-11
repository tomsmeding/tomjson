CC = gcc

CFLAGS = -Wall -Wextra -std=c11 -fwrapv -fPIC
ifdef DEBUG
	CFLAGS += -g -DDEBUG
else
	CFLAGS += -O2
endif

# Set to /usr/local to install in the system directories
PREFIX = $(HOME)/prefix


# ---------------------------------------------------------

NAME = tomjson

ifeq ($(PREFIX),)
	PREFIX = /usr/local
endif

SRC_FILES = $(wildcard *.c)
HEADER_FILES = $(wildcard *.h)
OBJECT_FILES = $(patsubst %.c,%.o,$(SRC_FILES))

UNAME = $(shell uname)

ifeq ($(UNAME),Darwin)
	DYLIB_EXT = dylib
	DYLIB_FLAGS = -dynamiclib
else
	DYLIB_EXT = so
	DYLIB_FLAGS = -shared
endif


# Don't remove intermediate files
.SECONDARY:


.PHONY: all clean install uninstall remake reinstall dynamiclib staticlib test

all: dynamiclib staticlib test

clean:
	rm -f *.$(DYLIB_EXT) *.a *.o
	make -C test clean

install: all
	install lib$(NAME).$(DYLIB_EXT) $(PREFIX)/lib
	install lib$(NAME).a $(PREFIX)/lib
	install $(NAME).h $(PREFIX)/include

uninstall:
	rm -f $(PREFIX)/lib/lib$(NAME).$(DYLIB_EXT)
	rm -f $(PREFIX)/lib/lib$(NAME).a
	rm -f $(PREFIX)/include/$(NAME).h

remake: clean all

reinstall: clean install

dynamiclib: lib$(NAME).$(DYLIB_EXT)

staticlib: lib$(NAME).a

test: staticlib
	make -C test rerun


%.o: %.c $(HEADER_FILES)
	$(CC) $(CFLAGS) -c -o $@ $<

%.$(DYLIB_EXT): $(OBJECT_FILES)
	$(CC) $(CFLAGS) $(DYLIB_FLAGS) -o $@ $^ -lm

%.a: $(OBJECT_FILES)
	ar -cr $@ $^
