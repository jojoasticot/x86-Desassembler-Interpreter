CC = gcc
CFLAGS = -Wall -Wextra
# LDLIBS = -lSDL2 -lSDL2_image -lm

OBJECTS = main.o disassembler.o

main: $(OBJECTS)

clean:
	rm *.[od]
	rm main
