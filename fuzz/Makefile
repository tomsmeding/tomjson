CC = afl-clang-fast
CFLAGS = -Wall -Wextra -std=c11 -O2 -g -fwrapv -I..

AFL_LOCATION = $(HOME)/programs/afl-install-dir

ENVVARS = AFL_USE_ASAN=1 AFL_PATH=$(AFL_LOCATION)/lib/afl PATH="$(AFL_LOCATION)/bin:$(PATH)"

.PHONY: all clean run resume

all: fuzz

clean:
	rm -f fuzz

run: fuzz
	env $(ENVVARS) afl-fuzz -i testdir -o outdir -m none -x /usr/share/afl/dictionaries/json.dict ./fuzz @@

resume: fuzz
	env $(ENVVARS) afl-fuzz -i - -o outdir -m none -x /usr/share/afl/dictionaries/json.dict ./fuzz @@

fuzz: fuzz.c
	env $(ENVVARS) $(CC) $(CFLAGS) -o $@ $< ../libtomjson.a
