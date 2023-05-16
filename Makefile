GCC = gcc
CFLAGS = -g -Wall -Wpedantic

all: clean out_dir out

out:
	$(GCC) $(CFLAGS) -o bin/cefunge cefunge.c

out_dir:
	mkdir bin

clean:
	rm -rf bin