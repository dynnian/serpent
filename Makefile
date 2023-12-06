all: serpent

WARNINGS = -Wall
DEBUG = -ggdb -fno-omit-frame-pointer -lncurses
OPTIMIZE = -O2

serpent: Makefile serpent.c serpent.h
	$(CC) -o $@ $(WARNINGS) $(DEBUG) $(OPTIMIZE) serpent.c

clean:
	rm -f serpent

install:
	echo "Installing is not supported"

run:
	./serpent

