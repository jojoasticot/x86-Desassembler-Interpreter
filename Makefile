CC = gcc
CFLAGS = -Wall -Wextra -fsanitize=address
# LDLIBS = -lSDL2 -lSDL2_image -lm

OBJECTS = main.o

main: $(OBJECTS)

clean:
	rm *.[od]
	rm main
