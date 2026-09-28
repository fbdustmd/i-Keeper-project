CC = gcc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic
CPPFLAGS ?=
LDFLAGS ?=
LDLIBS ?=

.PHONY: all clean
all: build/netsentry

build/netsentry: src/main.c
	mkdir -p build
	$(CC) $(CPPFLAGS) $(CFLAGS) src/main.c $(LDFLAGS) $(LDLIBS) -o $@

clean:
	rm -f build/netsentry build/netsentry.exe
