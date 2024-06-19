CC = gcc
CFLAGS = -Wall -Wextra
# LDLIBS = -lSDL2 -lSDL2_image -lm

OBJECTS = main.o disassembler.o operation.o interpreter.o interrupt.o

main: $(OBJECTS)

clean:
	rm *.[od]
	rm main
	rm a.out
