#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include "main.h"
#include "operation.h"
#include "minix/type.h"
#include "interrupt.h"
#include "interpreter.h"

int call_size = 0;

void update_reg(uint8_t reg, uint16_t value, int w)
{
    if (w == 1)
        registers[reg] = value;
    else
        registers[reg] = (registers[reg] & 0xff00) | (value & 0xff);
}

void print_memory(uint16_t adress, int w)
{
    if (w == 1)
        printf(" ;[%04x]%04x\n", adress, *(uint16_t *) &memory[adress]);
    else
        printf(" ;[%04x]%02x\n", adress, memory[adress]);
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
        print_memory(op->op1_value, op->w);
        update_reg(op->op0_value, *(uint16_t *) &memory[op->op1_value], op->w);
    }
    else if (op->op0_type == OP_MEM && op->op1_type == OP_REG)
    {
        print_memory(op->op0_value, op->w);
        *(uint16_t *) &memory[op->op0_value] = registers[op->op1_value];
    }
    else if (op->op0_type == OP_MEM && op->op1_type == OP_IMM)
    {
        print_memory(op->op0_value, op->w);
        *(uint16_t *) &memory[op->op0_value] = op->op1_value;
    }
    else
        errx(1, "Error: mov operation not supported");
}

void sub(operation * op)
{
    if (op->nb_operands != 2)
        errx(1, "Error: cmp operation must have 2 operands");

    uint16_t result = 0;
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
        print_memory(op->op0_value, op->w);
        val1 = *(uint16_t *) &memory[op->op0_value];
        val2 = op->op1_value;
    }
    else
    {
        errx(1, "Error: cmp operation not supported");
    }

    if (op->w == 1)
    {
        result = val1 - val2;
        flags[OF] = (val1 >> 15 != val2 >> 15) && (val2 >> 15 == result >> 15);
        flags[CF] = val1 < val2;
        flags[SF] = result >> 15;
        flags[ZF] = result == 0;
    }
    else
    {
        result = (val1 & 0xff00) | ((val1 & 0xff) - (val2 & 0xff));
        uint8_t sign1 = (val1 & 0x0080) >> 7;
        uint8_t sign2 = (val2 & 0x0080) >> 7;
        uint8_t signr = (result & 0x0080) >> 7;
        flags[OF] = sign1 != sign2 && sign2 == signr; // sign1 == 1 && sign2 == 0 && signr == 0 || sign1 == 0 && sign2 == 1 && signr == 1
        flags[CF] = (val1 & 0xff) < (val2 & 0xff);
        flags[SF] = (result & 0x0080) == 0x0080;
        flags[ZF] = (result & 0xff) == 0;
    }

    if (op->op0_type == OP_REG)
        update_reg(op->op0_value, result, op->w);
    else
        *(uint16_t *) &memory[op->op0_value] = result;
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
    print_memory(op->op1_value, op->w);
}

void add(operation * op)
{
    if (op->nb_operands != 2)
        errx(1, "Error: add operation must have 2 operands");
    
    // empty comment
    uint16_t result = 0;
    uint16_t val1 = 0;
    uint16_t val2 = 0;

    if (op->op0_type == OP_REG && op->op1_type == OP_REG)
    {
        printf("\n");
        val1 = registers[op->op0_value];
        val2 = registers[op->op1_value];
    }
    else if (op->op0_type == OP_REG && op->op1_type == OP_IMM)
    {
        printf("\n");
        val1 = registers[op->op0_value];
        val2 = op->op1_value;
    }
    else if (op->op0_type == OP_MEM && op->op1_type == OP_REG)
    {
        print_memory(op->op0_value, op->w);
        val1 = *(uint16_t *) &memory[op->op0_value];
        val2 = registers[op->op1_value];
    }
    else if (op->op0_type == OP_MEM && op->op1_type == OP_IMM)
    {
        print_memory(op->op0_value, op->w);
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
        print_memory(op->op0_value, op->w);
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

    printf("\n");
    if (flags[CF] == 0)
    {
        PC = op->op0_value - 1; // PC will be incremented at the end of the loop
    }
    else
    {
        PC++;
    }
}

void jne(operation * op)
{
    if (op->nb_operands != 1)
        errx(1, "Error: jne operation must have 1 operand");

    printf("\n");
    if (flags[ZF] == 0)
    {
        PC = op->op0_value - 1; // PC will be incremented at the end of the loop
    }
    else
    {
        PC++;
    }
}

void jl(operation * op)
{
    if (op->nb_operands != 1)
        errx(1, "Error: jl operation must have 1 operand");

    printf("\n");
    if (flags[SF] != flags[OF])
        PC = op->op0_value - 1; // PC will be incremented at the end of the loop
    else
        PC++;
}

void jle(operation * op)
{
    if (op->nb_operands != 1)
        errx(1, "Error: jle operation must have 1 operand");

    printf("\n");
    if (flags[ZF] == 1 || flags[SF] != flags[OF])
        PC = op->op0_value - 1; // PC will be incremented at the end of the loop
    else
        PC++;
}

void jb(operation * op)
{
    if (op->nb_operands != 1)
        errx(1, "Error: jb operation must have 1 operand");
    
    printf("\n");
    if (flags[CF] == 1)
        PC = op->op0_value - 1; // PC will be incremented at the end of the loop
    else
        PC++;
}

void jbe(operation * op)
{
    if (op->nb_operands != 1)
        errx(1, "Error: jbe operation must have 1 operand");
    
    printf("\n");
    if (flags[CF] == 1 || flags[ZF] == 1)
        PC = op->op0_value - 1; // PC will be incremented at the end of the loop
    else
        PC++;
}

void jo(operation * op)
{
    if (op->nb_operands != 1)
        errx(1, "Error: jp operation must have 1 operand");
    
    printf("\n");
    if (flags[OF] == 1)
        PC = op->op0_value - 1; // PC will be incremented at the end of the loop
    else
        PC++;
}

void js(operation * op)
{
    if (op->nb_operands != 1)
        errx(1, "Error: js operation must have 1 operand");
    
    printf("\n");
    if (flags[SF] == 1)
        PC = op->op0_value - 1; // PC will be incremented at the end of the loop
    else
        PC++;
}

void test(operation * op) 
{
    if (op->nb_operands != 2)
        errx(1, "Error: test operation must have 2 operands");

    flags[OF] = 0;
    flags[CF] = 0;

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
        print_memory(op->op0_value, op->w);
        val1 = *(uint16_t *) &memory[op->op0_value];
        val2 = op->op1_value;
    }
    else
        errx(1, "Error: test operation not supported");

    uint16_t result = val1 & val2;

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
    else if (op->op0_type == OP_IMM)
    {
        printf("\n");
        registers[SP] -= 2;
        *(uint16_t *) &memory[registers[SP]] = op->op0_value;
    }
    else // OP_MEM
    {
        print_memory(op->op0_value, op->w);
        registers[SP] -= 2;
        *(uint16_t *) &memory[registers[SP]] = *(uint16_t *) &memory[op->op0_value];
    }
}

void _call(operation * op)
{
    if (op->nb_operands != 1)
        errx(1, "Error: call operation must have 1 operand");

    call_size = 2 + op->w;

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
        errx(1, "Error: jmp operation not supported");
    }
}

void pop(operation * op)
{
    if (op->nb_operands != 1)
        errx(1, "Error: pop operation must have 1 operand");

    if (op->op0_type == OP_REG)
    {
        printf("\n");
        registers[op->op0_value] = *(uint16_t *) &memory[registers[SP]];
        registers[SP] += 2;
    }
    else if (op->op0_type == OP_MEM)
    {
        print_memory(op->op0_value, op->w);
        *(uint16_t *) &memory[op->op0_value] = *(uint16_t *) &memory[registers[SP]];
        registers[SP] += 2;
    }
    else
    {
        errx(1, "Error: pop operation not supported");
    }
}

void ret(operation * op)
{
    if (op->nb_operands != 0)
        errx(1, "Error: ret operation must have 0 operand");

    printf("\n");
    PC = *(uint16_t *) &memory[registers[SP]] - 1 + call_size; // PC will be incremented at the end of the loop
    registers[SP] += 2;
}

void or(operation * op)
{
    if (op->nb_operands != 2)
        errx(1, "Error: or operation must have 2 operands");

    // empty comment
    printf("\n");
    flags[OF] = 0;
    flags[CF] = 0;

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
        errx(1, "Error: or operation not supported");
    
    if (op->w == 1)
    {
        result = val1 | val2;
        flags[SF] = result >> 15;
        flags[ZF] = result == 0;
    }
    else
    {
        result = (val1 & 0xff00) | (val1 | val2);
        flags[SF] = (result & 0x0080) == 0x0080;
        flags[ZF] = (result & 0xff) == 0;
    }

    if (op->op0_type == OP_REG)
        registers[op->op0_value] = result;
    else
        *(uint16_t *) &memory[op->op0_value] = result;
}

void je(operation * op)
{
    if (op->nb_operands != 1)
        errx(1, "Error: je operation must have 1 operand");

    printf("\n");
    if (flags[ZF] == 1)
        PC = op->op0_value - 1; // PC will be incremented at the end of the loop
    else
        PC++;
}

void jnl(operation * op)
{
    if (op->nb_operands != 1)
        errx(1, "Error: jnl operation must have 1 operand");

    printf("\n");
    if (flags[SF] == flags[OF])
        PC = op->op0_value - 1; // PC will be incremented at the end of the loop
    else
       PC++;
}

void dec(operation * op)
{
    if (op->nb_operands != 1)
        errx(1, "Error: dec operation must have 1 operand");

    uint16_t value = 0;

    if (op->op0_type == OP_REG)
    {
        printf("\n");
        value = registers[op->op0_value];
    }
    else if (op->op0_type == OP_MEM)
    {
        print_memory(op->op0_value, op->w);
        value = *(uint16_t *) &memory[op->op0_value];
    }
    else
        errx(1, "Error: dec operation not supported");

    if (op->w == 1)
    {
        flags[OF] = value == 0x8000;
        value--;
        flags[SF] = value >> 15;
        flags[ZF] = value == 0;
    }
    else
    {
        flags[OF] = (value & 0xff) == 0x80;
        value = (value & 0xff00) | ((value & 0xff) - 1);
        flags[SF] = (value & 0x0080) == 0x0080;
        flags[ZF] = (value & 0xff) == 0;
    }

    if (op->op0_type == OP_REG)
        registers[op->op0_value] = value;
    else
        *(uint16_t *) &memory[op->op0_value] = value;
}

void cbw(operation * op)
{
    if (op->nb_operands != 0)
        errx(1, "Error: cbw operation must have 0 operand");

    printf("\n");
    registers[AX] = (registers[AL] & 0x80) == 0x80 ? 0xff00 | registers[AL] : 0x0000 | registers[AL];
}

void inc(operation * op)
{
    if (op->nb_operands != 1)
        errx(1, "Error: dec operation must have 1 operand");

    uint16_t value = 0;

    if (op->op0_type == OP_REG)
    {
        printf("\n");
        value = registers[op->op0_value];
    }
    else if (op->op0_type == OP_MEM)
    {
        print_memory(op->op0_value, op->w);
        value = *(uint16_t *) &memory[op->op0_value];
    }
    else
        errx(1, "Error: dec operation not supported");

    if (op->w == 1)
    {
        flags[OF] = value == 0x7FFF;
        value++;
        flags[SF] = value >> 15;
        flags[ZF] = value == 0;
    }
    else
    {
        flags[OF] = (value & 0xff) == 0x7F;
        value = (value & 0xff00) | ((value & 0xff) + 1);
        flags[SF] = (value & 0x0080) == 0x0080;
        flags[ZF] = (value & 0xff) == 0;
    }

    if (op->op0_type == OP_REG)
        registers[op->op0_value] = value;
    else
        *(uint16_t *) &memory[op->op0_value] = value;
}

// TODO fix print good adress

void interpreter(operation * op)
{
    static FunctionMap func_map[] = 
    {
        {"+mov", move},
        {"+sub", sub},
        {"+xor", xor},
        {"+lea", lea},
        {"+add", add},
        {"+cmp", cmp},
        {"+jnb", jnb},
        {"+jne", jne},
        {"+je", je},
        {"+jnl", jnl},
        {"+jl", jl},
        {"+jle", jle},
        {"+jb", jb},
        {"+jbe", jbe},
        {"+jo", jo},
        {"+js", js},
        {"+test", test},
        {"+test byte", test},
        {"+push", push},
        {"+call", _call},
        {"+jmp", jmp},
        {"+jmp short", jmp},
        {"+pop", pop},
        {"+ret", ret},
        {"+or", or},
        {"+dec", dec},
        {"+cbw", cbw},
        {"+inc", inc},
        {NULL, NULL}
    };
    char * name = op->name;

    if (strcmp(name, "+int") == 0)
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
    else
    {
        int i = 0;

        while (func_map[i].name != NULL)
        {
            if (strcmp(name, func_map[i].name) == 0)
            {
                func_map[i].func(op);
                break;
            }

            i++;
        }

        if (func_map[i].name == NULL)
            errx(1, "Error: operation not supported");
    }
}