#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <err.h>
#include <string.h>
#include "main.h"
#include "operation.h"
#include "disassembler.h"

uint16_t registers[8] = {0, 0, 0, 0, 0xffda, 0, 0, 0};
uint8_t * text;
uint8_t * data;
uint32_t PC;

int main(int argc, char* argv[])
{
    if (argc < 2)
        errx(1, "Argument missing: ./main <a.out>");

    char * filename = argv[1];
    FILE* file = fopen(filename, "r");
    printf("File: %s\n", filename);

    uint32_t text_length;
    uint32_t data_length;

    read_file(file, &text_length, &data_length);
    printf("Text length: %d\n", text_length);
    printf("Data length: %d\n", data_length);

    disassembler(text_length, data_length);

    fclose(file);
    return 0;
}
