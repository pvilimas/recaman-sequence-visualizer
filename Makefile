.PHONY: all build run

all: build run
	
build:
	gcc -Wall \
		-o build/recaman src/*.c \
		-I include -L lib -l raylib -lm -lX11

run:
	./build/recaman

