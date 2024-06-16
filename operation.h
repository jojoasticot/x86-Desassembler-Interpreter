#include <stdint.h>
#ifndef operation_h
#define operation_h

// Registers names:

#define AL 0b000
#define CL 0b001
#define DL 0b010
#define BL 0b011
#define AH 0b100
#define CH 0b101
#define DH 0b110
#define BH 0b111

#define AX 0b000
#define CX 0b001
#define DX 0b010
#define BX 0b011
#define SP 0b100
#define BP 0b101
#define SI 0b110
#define DI 0b111

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

#endif