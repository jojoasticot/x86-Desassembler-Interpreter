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
        printf(" ;[%04x]%04x\n", op->op1_value, *(uint16_t *) &memory[op->op1_value]);
        update_reg(op->op0_value, *(uint16_t *) &memory[op->op1_value], op->w);
    }
    else if (op->op0_type == OP_MEM && op->op1_type == OP_REG)
    {
        printf(" ;[%04x]%04x\n", op->op0_value, *(uint16_t *) &memory[op->op0_value]);
        *(uint16_t *) &memory[op->op0_value] = registers[op->op1_value];
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

void add(operation * op)
{
    if (op->nb_operands != 2)
        errx(1, "Error: add operation must have 2 operands");
    
    // empty comment
    printf("\n");
    uint16_t result = 0;
    uint16_t val1 = 0;
    uint16_t val2 = 0;

    if (op->op0_type == OP_REG && op->op1_type == OP_REG)
    {
        val1 = registers[op->op0_value];
        val2 = registers[op->op1_value];
    }
    else if (op->op0_type == OP_REG && op->op1_type == OP_IMM)
    {
        val1 = registers[op->op0_value];
        val2 = op->op1_value;
    }
    else if (op->op0_type == OP_MEM && op->op1_type == OP_REG)
    {
        val1 = *(uint16_t *) &memory[op->op0_value];
        val2 = registers[op->op1_value];
    }
    else if (op->op0_type == OP_MEM && op->op1_type == OP_IMM)
    {
        val1 = *(uint16_t *) &memory[op->op0_value];
        val2 = op->op1_value;
    }
    else
        errx(1, "Error: add operation not supported");
    
    if (op->w == 1)
    {
        result = val1 + val2;
        flags[OF] = (result >> 15 != val1 >> 15) && (val2 >> 15 == val1 >> 15);
        flags[CF] = result < val1;
        flags[SF] = result >> 15;
        flags[ZF] = result == 0;
    }
    else
    {
        result = (val1 & 0xff00) | (val1 + val2);
        uint8_t sign1 = (val1 & 0x0080) >> 7;
        uint8_t sign2 = (val2 & 0x0080) >> 7;
        uint8_t signr = (result & 0x0080) >> 7;
        flags[OF] = (signr != sign1) && (sign1 == sign2);
        flags[CF] = result < (val1 & 0xff);
        flags[SF] = (result & 0x0080) == 0x0080;
        flags[ZF] = (result & 0xff) == 0;
    }

    if (op->op0_type == OP_REG)
        registers[op->op0_value] = result;
    else
        *(uint16_t *) &memory[op->op0_value] = result;
}

void cmp(operation * op)
{
    if (op->nb_operands != 2)
        errx(1, "Error: cmp operation must have 2 operands");

    uint16_t temp = 0;
    uint16_t val1 = 0;
    uint16_t val2 = 0;

    if (op->op0_type == OP_REG && op->op1_type == OP_IMM)
    {
        printf("\n");
        val1 = registers[op->op0_value];
        val2 = op->op1_value;
    }
    else if (op->op0_type == OP_MEM && op->op1_type == OP_IMM)
    {
        printf(" ;[%04x]%04x\n", op->op0_value, *(uint16_t *) &memory[op->op0_value]);
        val1 = *(uint16_t *) &memory[op->op0_value];
        val2 = op->op1_value;
    }
    else
    {
        errx(1, "Error: cmp operation not supported");
    }

    if (op->w == 1)
    {
        temp = val1 - val2;
        flags[OF] = (val1 >> 15 != val2 >> 15) && (val2 >> 15 == temp >> 15);
        flags[CF] = val1 < val2;
        flags[SF] = temp >> 15;
        flags[ZF] = temp == 0;
    }
    else
    {
        temp = (val1 & 0xff00) | ((val1 & 0xff) - (val2 & 0xff));
        uint8_t sign1 = (val1 & 0x0080) >> 7;
        uint8_t sign2 = (val2 & 0x0080) >> 7;
        uint8_t signr = (temp & 0x0080) >> 7;
        flags[OF] = sign1 != sign2 && sign2 == signr; // sign1 == 1 && sign2 == 0 && signr == 0 || sign1 == 0 && sign2 == 1 && signr == 1
        flags[CF] = (val1 & 0xff) < (val2 & 0xff);
        flags[SF] = (temp & 0x0080) == 0x0080;
        flags[ZF] = (temp & 0xff) == 0;
    }
}

void jnb(operation * op)
{
    if (op->nb_operands != 1)
        errx(1, "Error: jnb operation must have 1 operand");

    if (flags[CF] == 0)
    {
        PC = op->op0_value - 1; // PC will be incremented at the end of the loop
        printf(" ;%04x\n", PC);
    }
    else
    {
        printf("\n");
        PC++;
    }
}

void jne(operation * op)
{
    if (op->nb_operands != 1)
        errx(1, "Error: jne operation must have 1 operand");

    if (flags[ZF] == 0)
    {
        PC = op->op0_value - 1; // PC will be incremented at the end of the loop
        printf(" ;%04x\n", PC);
    }
    else
    {
        printf("\n");
        PC++;
    }
}

void test(operation * op) 
{
    if (op->nb_operands != 2)
        errx(1, "Error: test operation must have 2 operands");

    flags[OF] = 0;
    flags[CF] = 0;

    //empty comment
    printf("\n");

    if (op->op0_type == OP_REG && op->op1_type == OP_IMM)
    {
        uint16_t reg = registers[op->op0_value];
        uint16_t imm = op->op1_value;
        uint16_t result = reg & imm;

        if (op->w == 1)
        {
            flags[SF] = result >> 15;
            flags[ZF] = result == 0;
        }
        else
        {
            flags[SF] = (result & 0x0080) == 0x0080;
            flags[ZF] = (result & 0xff) == 0;
        }
    }
    else
        errx(1, "Error: test operation not supported");
}

void push(operation * op)
{
    if (op->nb_operands != 1)
        errx(1, "Error: push operation must have 1 operand");

    if (op->op0_type == OP_REG)
    {
        printf("\n");
        uint16_t reg = registers[op->op0_value];
        registers[SP] -= 2;
        *(uint16_t *) &memory[registers[SP]] = reg;
    }
    else
        errx(1, "Error: push operation not supported");
}

void _call(operation * op)
{
    if (op->nb_operands != 1)
        errx(1, "Error: call operation must have 1 operand");

    if (op->op0_type == OP_IMM)
    {
        printf("\n");
        registers[SP] -= 2;
        *(uint16_t *) &memory[registers[SP]] = PC;
        PC = op->op0_value - 1; // PC will be incremented at the end of the loop
    }
    else
    {
        printf("\n");
        PC += 2;
    }
}

void jmp(operation * op)
{
    if (op->nb_operands != 1)
        errx(1, "Error: jmp operation must have 1 operand");

    if (op->op0_type == OP_IMM)
    {
        printf("\n");
        PC = op->op0_value - 1; // PC will be incremented at the end of the loop
    }
    else
    {
        printf("\n");
        PC += 1 + op->w;
    }

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
    else if (strcmp(name, "+add") == 0)
        add(op);
    else if (strcmp(name, "+cmp") == 0)
        cmp(op);
    else if (strcmp(name, "+jnb") == 0)
        jnb(op);
    else if (strcmp(name, "+jne") == 0)
        jne(op);
    else if (strcmp(name, "+test") == 0)
        test(op);
    else if (strcmp(name, "+push") == 0)
        push(op);
    else if (strcmp(name, "+call") == 0)
        _call(op);
    else if (strcmp(name, "+jmp") == 0)
        jmp(op);
    else
        printf(" | not done\n");
}