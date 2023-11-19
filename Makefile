all: serpent

WARNINGS = -Wall
DEBUG = -ggdb -fno-omit-frame-pointer
OPTIMIZE = -O2

serpent: Makefile serpent.c
	$(CC) -o $@ $(WARNINGS) $(DEBUG) $(OPTIMIZE) serpent.c

clean:
	rm -f serpent

install:
	echo "Installing is not supported"

run:
	./serpent

