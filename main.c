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

uint16_t registers[8] = {0, 0, 0, 0, 0xffff, 0, 0, 0};
uint8_t memory[0x10000];
uint8_t flags[4] = {0, 0, 0, 0};
uint8_t * text;
uint8_t * data;
uint32_t PC;

void push_stack(uint16_t value, uint8_t size)
{
    if (size == 1)
        memory[registers[SP]] = value;
    else
        * (uint16_t*) &memory[registers[SP]] = value;
    registers[SP] -= size;
}

static void init_stack(int argc, char **argv)
{
	char *env = "PATH=/bin";

	uint16_t len = strlen(env) + 1;

	for (int i = 0; i < argc; ++i)
		len += strlen(argv[i]) + 1;

	push_stack('\0', 1);
	for (int i = strlen(env)- 1; i >= 0; --i)
		push_stack(env[i], 1);

	// get addr
	uint16_t env_addr = registers[SP];
	uint16_t arg_addr[argc];

	// push each argv in reverse order and fetch their SP
	for (int i = argc - 1; i >= 0; --i)
	{
		push_stack('\0', 1);
		for (int j = strlen(argv[i]) - 1; j >= 0; --j)
			push_stack(argv[i][j], 1);
		arg_addr[i] = registers[SP];
	}

	// separator
	push_stack(0, 2);
	// push env addr
	push_stack(env_addr, 2);
	// separator
	push_stack(0, 2);

	// push each argv addr (in reverse order)
	for (int i = argc - 1; i >= 0; --i)
		push_stack(arg_addr[i], 2);
	push_stack(argc, 2);
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
    // * (uint16_t*) &memory [0xffd6] = 0xffde;

    init_stack(argc, argv);

    int i = 0xFFFF;
    while (i >= registers[SP])
    {
        printf("0x%04x: %02x\n", i, memory[i]);
        i--;
    }

    disassembler(text_length, data_length);

    fclose(file);
    return 0;
}
