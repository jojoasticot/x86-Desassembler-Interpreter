#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "operation.h"
#include "minix/type.h"
#include "interrupt.h"

void update_reg(uint8_t reg, uint16_t value, int w)
{
    if (w == 1)
        registers[reg] = value;
    else
        registers[reg] = (registers[reg] & 0xff00) | (value & 0xff);
}

void move(operation * op)
{
    if (op->nb_operands != 2)
    {
        printf("Error: mov operation must have 2 operands\n");
        exit(1);
    }

    operand op1 = op->operands[0];
    operand op2 = op->operands[1];

    if (op1.type == OP_REG && op2.type == OP_IMM)
        update_reg(op1.value, op2.value, op->w);
    else if (op1.type == OP_REG && op2.type == OP_REG)
        update_reg(op1.value, registers[op2.value], op->w);
    else
    {
        printf("Error: mov operation not supported\n");
        exit(1);
    }
}

void sub(operation * op)
{
    if (op->nb_operands != 2)
    {
        printf("Error: sub operation must have 2 operands\n");
        exit(1);
    }

    if (op->op0_type == OP_MEM && op->op1_type == OP_IMM)
    {
        uint16_t adress = op->op0_value;
        uint16_t value = op->op1_value;
        if (op->w == 1)
            *(uint16_t *) &memory[adress] -= value;
        else
            *(uint8_t *) &memory[adress] -= value;
        
        printf("%s [%04x], %i, new: %04x\n", op->name, adress, value, memory[adress]);
    }
}

void interpreter(operation * op)
{
    char * name = op->name;
    print_operation(op);

    if (strcmp(name, "+mov") == 0)
        move(op);
    else if (strcmp(name, "+int") == 0)
    {
        if (op->operands[0].value == 0x20)
        {
            uint16_t adress = registers[BX];
            message * msg = (message *) &data[adress];
            interrupt(msg);
        }
    }
    else if (strcmp(name, "+sub") == 0)
        sub(op);
}