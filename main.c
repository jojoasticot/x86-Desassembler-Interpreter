#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <err.h>
#include <string.h>
#include "main.h"
#include "operation.h"
#include "disassembler.h"


char* registers_name[2][8] = {
    {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"},
    {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"},
};

uint16_t registers[8] = {0, 0, 0, 0, 0xffd4, 0, 0, 0};
uint8_t memory[0xFFFF];
uint8_t flags[4] = {0, 0, 0, 0};
uint8_t * text;
uint8_t * data;
uint32_t PC;

void print_stack()
{
    printf("Stack:\n");
    for (int i = registers[SP]; i < 0xFFFF; i++)
        printf("0x%04x: 0x%02x\n", i, memory[i]);
}

int main(int argc, char* argv[])
{
    if (argc < 2)
        errx(1, "Argument missing: ./main <a.out>");

    char * filename = argv[1];
    FILE* file = fopen(filename, "r");

    uint32_t text_length;
    uint32_t data_length;

    read_file(file, &text_length, &data_length);
    printf("Text length: %d\n", text_length);
    printf("Data length: %d\n", data_length);
    memory[0xffd4] = 1;
    * (uint16_t*) &memory [0xffd6] = 0xffde;

    disassembler(text_length, data_length);

    fclose(file);
    return 0;
}
