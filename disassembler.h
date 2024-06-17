#include <stdio.h>
#include <stdlib.h>
#ifndef disassembler_h
#define disassembler_h

void read_file(FILE* file, uint32_t* text_length, uint32_t* data_length);
void disassembler(uint32_t text_length, uint32_t data_length);

#endif