#include <stdint.h>
#ifndef operation_h
#define operation_h

// Operation types:

#define OP_REG 0b00
#define OP_MEM 0b01
#define OP_IMM 0b10

typedef struct operand
{
    uint8_t type;
    uint16_t value;
} operand;

typedef struct operation
{
    char * name;
    uint8_t nb_operands;
    int w;
    operand operands[3];
} operation;

void print_operation(operation * op);

#define op0_type operands[0].type
#define op1_type operands[1].type
#define op0_value operands[0].value
#define op1_value operands[1].value

#endif