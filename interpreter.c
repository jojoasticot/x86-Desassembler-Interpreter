#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
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
        errx(1, "Error: mov operation must have 2 operands");

    if (op->op0_type == OP_REG && op->op1_type == OP_IMM)
    {
        update_reg(op->op0_value, op->op1_value, op->w);
        // empty comment
        printf("\n");
    }
    else if (op->op0_type == OP_REG && op->op1_type == OP_REG)
    {
        update_reg(op->op0_value, registers[op->op1_value], op->w);
        // empty comment
        printf("\n");
    }
    else if (op->op0_type == OP_REG && op->op1_type == OP_MEM)
    {
        update_reg(op->op0_value, *(uint16_t *) &memory[op->op1_value], op->w);
        printf(" ;[%04x]%04x\n", op->op1_value, *(uint16_t *) &memory[op->op1_value]);
    }
    else
        errx(1, "Error: mov operation not supported");
}

void sub(operation * op)
{
    if (op->nb_operands != 2)
        errx(1, "Error: sub operation must have 2 operands");

    if (op->op0_type == OP_MEM && op->op1_type == OP_IMM)
    {
        uint16_t adress = op->op0_value;
        uint16_t value = op->op1_value;

        printf(" ;[%04x]%04x\n", adress, *(uint16_t *) &memory[adress]);
        if (op->w == 1)
            *(uint16_t *) &memory[adress] -= value;
        else
            *(uint8_t *) &memory[adress] -= value;
    }
    else
        errx(1, "Error: sub operation not supported");
}

void xor(operation * op)
{
    if (op->nb_operands != 2)
        errx(1, "Error: xor operation must have 2 operands");

    // empty comment
    printf("\n");
    flags[OF] = 0;
    flags[CF] = 0;

    if (op->op0_type == OP_REG && op->op1_type == OP_REG)
    {
        uint8_t reg1 = op->op0_value;
        uint8_t reg2 = op->op1_value;

        if (op->w == 1)
            registers[reg1] ^= registers[reg2];
        else
            registers[reg1] = (registers[reg1] & 0xff00) | (registers[reg1] ^ registers[reg2]);

        flags[SF] = registers[reg1] >> 15;
        flags[ZF] = registers[reg1] == 0;
    }
    else
        errx(1, "Error: xor operation not supported");
}

void lea(operation * op)
{
    if (op->nb_operands != 2)
        errx(1, "Error: lea operation must have 2 operands");
    if (op->op0_type != OP_REG || op->op1_type != OP_MEM)
        errx(1, "Error: lea operation not supported");
    
    update_reg(op->op0_value, op->op1_value, op->w);
    printf(" ;[%4x]%x\n", op->op1_value, *(uint16_t *) &memory[op->op1_value]);
}

void interpreter(operation * op)
{
    char * name = op->name;

    if (strcmp(name, "+mov") == 0)
        move(op);
    else if (strcmp(name, "+int") == 0)
    {
        if (op->op0_value == 0x20)
        {
            uint16_t adress = registers[BX];
            message * msg = (message *) &memory[adress];
            interrupt(msg);
        }
        else
            errx(1, "Error: interrupt not supported");
    }
    else if (strcmp(name, "+sub") == 0)
        sub(op);
    else if (strcmp(name, "+xor") == 0)
        xor(op);
    else if (strcmp(name, "+lea") == 0)
        lea(op);
    else if (strcomp(name, "+add") == 0)
        add(op);
    else
        printf(" | not done\n");
}