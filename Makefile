GCC = gcc
CFLAGS = -Wall -Wpedantic
SOURCES = cefunge.c error.c

all: clean out

run: all
	@echo "Running program:\n"
	@./cefunge examples/HelloWorld.befunge

out:
	$(GCC) $(CFLAGS) -o cefunge $(SOURCES)

clean:
	rm -rf cefunge