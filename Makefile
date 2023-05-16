GCC = gcc
CFLAGS = -Wall -Wpedantic
SOURCES = cefunge.c error.c

all: clean out_dir out

run: all
	@echo "Running program:\n"
	@./bin/cefunge

out:
	$(GCC) $(CFLAGS) -o bin/cefunge $(SOURCES)

out_dir:
	mkdir bin

clean:
	rm -rf bin