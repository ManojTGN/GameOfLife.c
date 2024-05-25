CC = gcc
BINS = _build

all: $(BINS)

_build:
	if [ ! -d build ]; then mkdir build; fi
	$(CC) src/gameOfLife.c -o build/gameOfLife.exe

