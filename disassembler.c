#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <err.h>
#include <string.h>
#include "main.h"
#include "operation.h"
#include "interpreter.h"

void pretty_print(uint8_t* bytes, int size, char* op)
{
    printf("%04x %04x %04x %04x %04x %04x %04x %04x ---- ", \
    registers[AX], registers[BX], registers[CX], registers[DX], \
    registers[SP], registers[BP], registers[SI], registers[DI]);

    char concatenated[2 * size + 1]; 
    concatenated[0] = '\0';

    for (int i = 0; i < size; i++)
    {
        char * temp;

        asprintf(&temp, "%02x", bytes[i]);

        strcat(concatenated, temp);

        free(temp);
    }

    printf("%04x: %-14s%s\n", PC, concatenated, op);
    free(op);
}

char* disp_string(int16_t disp)
{
    if (disp > 0)
    {
        char* string;
        asprintf(&string, "+%x", disp);
        return string;
    }
    else if (disp < 0)
    {
        char* string;
        asprintf(&string, "-%x", -disp);
        return string;
    }

    return "";
}

char* rm_string(uint8_t rm, int16_t disp)
{
    char* string = NULL;

    switch (rm)
    {
    case 0b000:
        asprintf(&string, "[bx+si%s]", disp_string(disp));
        break;
    case 0b001:
        asprintf(&string, "[bx+di%s]", disp_string(disp));
        break;
    case 0b010:
        asprintf(&string, "[bp+si%s]", disp_string(disp));
        break;
    case 0b011:
        asprintf(&string, "[bp+di%s]", disp_string(disp));
        break;
    case 0b100:
        asprintf(&string, "[si%s]", disp_string(disp));
        break;
    case 0b101:
        asprintf(&string, "[di%s]", disp_string(disp));
        break;
    case 0b110:
        asprintf(&string, "[bp%s]", disp_string(disp));
        break;
    case 0b111:
        asprintf(&string, "[bx%s]", disp_string(disp));
        break;
    default:
        break;
    }

    return string;
}

uint16_t compute_ea(uint8_t rm, int16_t disp)
{
    uint16_t ea = 0;

    switch (rm)
    {
    case 0b000:
        ea = registers[BX] + registers[SI] + disp;
        break;
    case 0b001:
        ea = registers[BX] + registers[DI] + disp;
        break;
    case 0b010:
        ea = registers[BP] + registers[SI] + disp;
        break;
    case 0b011:
        ea = registers[BP] + registers[DI] + disp;
        break;
    case 0b100:
        ea = registers[SI] + disp;
        break;
    case 0b101:
        ea = registers[DI] + disp;
        break;
    case 0b110:
        ea = registers[BP] + disp;
        break;
    case 0b111:
        ea = registers[BX] + disp;
        break;
    default:
        break;
    }

    return ea;
}

void v_w_mod_rm(char* op_name, int v, int w, uint8_t mod, uint8_t rm, uint8_t* bytes)
{

    uint16_t disp;
    char* string;

    switch (mod)
    {
    case 0b00:
        if (rm == 0b110)
        {
            // we have to read the disp byte
            bytes[2] = text[PC + 1];
            bytes[3] = text[PC + 2];
            disp = bytes[3] << 8 | bytes[2];

            if (v == 1)
                asprintf(&string, "%s [%04x], cl", op_name, disp);
            else
                asprintf(&string, "%s [%04x], 1", op_name, disp);

            pretty_print(bytes, 4, string);
            PC+=3;
        }
        else
        {
            if (v == 1)
                asprintf(&string, "%s %s, cl", op_name, rm_string(rm, 0));
            else
                asprintf(&string, "%s %s, 1", op_name, rm_string(rm, 0));

            pretty_print(bytes, 2, string);
            PC++;
        }
        
        break;
    case 0b10:
        // we have to read the disp word
        bytes[2] = text[PC + 1];
        bytes[3] = text[PC + 2];
        disp = bytes[3] << 8 | bytes[2];
        int16_t signed_disp = (int16_t)disp;

        if (v == 1)
            asprintf(&string, "%s %s, [CL]", op_name, rm_string(rm, signed_disp));
        else
            asprintf(&string, "%s %s, 1", op_name, rm_string(rm, signed_disp));

        pretty_print(bytes, 4, string);
        PC+=3;
        break;
    case 0b01:
        // we have to read the disp byte
        bytes[2] = text[PC + 1];
        disp = (int8_t)bytes[2];

        if (v == 1)
            asprintf(&string, "%s %s, cl", op_name, rm_string(rm, disp));
        else
            asprintf(&string, "%s %s, 1", op_name, rm_string(rm, disp));

        pretty_print(bytes, 3, string);
        PC+=2;
        break;
    case 0b11:
        if (v == 1)
            asprintf(&string, "%s %s, cl", op_name, registers_name[w][rm]);
        else
            asprintf(&string, "%s %s, 1", op_name, registers_name[w][rm]);
        pretty_print(bytes, 2, string);
        PC++;
        break;
    default:
        break;
    }
}

operation * mod_reg_rm(char* op_name, uint8_t current, int d, int w)
{
    operation * op = malloc(sizeof(operation));
    op->name = op_name;
    op->nb_operands = 2;
    op->w = w;
    int reg_idx = d == 1 ? 0 : 1;
    int mem_idx = 1 - reg_idx;

    uint8_t bytes[4];
    bytes[0] = current;
    current = text[PC + 1];
    bytes[1] = current;
    uint8_t mod = MOD(current);
    uint8_t reg = REG(current);
    uint8_t rm = RM(current);
    uint16_t disp;
    char* string;

    switch (mod)
    {
    case 0b00:
        op->operands[reg_idx].type = OP_REG;
        op->operands[reg_idx].value = reg;
        op->operands[mem_idx].type = OP_MEM;

        if (rm == 0b110)
        {
            // we have to read the disp byte
            bytes[2] = text[PC + 2];
            bytes[3] = text[PC + 3];
            disp = bytes[3] << 8 | bytes[2];

            if (d == 1)
                asprintf(&string, "%s %s, [%04x]", op_name, registers_name[w][reg], disp);
            else
                asprintf(&string, "%s [%04x], %s", op_name, disp, registers_name[w][reg]);

            op->operands[mem_idx].value = disp;

            pretty_print(bytes, 4, string);
            PC+=3;
        }
        else
        {
            if (d == 1)
                asprintf(&string, "%s %s, %s", op_name, registers_name[w][reg], rm_string(rm, 0));
            else
                asprintf(&string, "%s %s, %s", op_name, rm_string(rm, 0), registers_name[w][reg]);
            
            op->operands[mem_idx].value = compute_ea(rm, 0);

            pretty_print(bytes, 2, string);
            PC++;
        }
        
        break;
    case 0b10:
        // we have to read the disp word
        bytes[2] = text[PC + 2];
        bytes[3] = text[PC + 3];
        disp = bytes[3] << 8 | bytes[2];
        int16_t signed_disp = (int16_t)disp;

        if (d == 1)
            asprintf(&string, "%s %s, %s", op_name, registers_name[w][reg], rm_string(rm, signed_disp));
        else
            asprintf(&string, "%s %s, %s", op_name, rm_string(rm, signed_disp), registers_name[w][reg]);
        
        op->operands[reg_idx].type = OP_REG;
        op->operands[reg_idx].value = reg;
        op->operands[mem_idx].type = OP_MEM;
        op->operands[mem_idx].value = compute_ea(rm, signed_disp);

        pretty_print(bytes, 4, string);
        PC+=3;
        break;
    case 0b01:
        // we have to read the disp byte
        bytes[2] = text[PC + 2];
        disp = (int8_t)bytes[2];

        if (d == 1)
            asprintf(&string, "%s %s, %s", op_name, registers_name[w][reg], rm_string(rm, disp));
        else
            asprintf(&string, "%s %s, %s", op_name, rm_string(rm, disp), registers_name[w][reg]);
        
        op->operands[reg_idx].type = OP_REG;
        op->operands[reg_idx].value = reg;
        op->operands[mem_idx].type = OP_MEM;
        op->operands[mem_idx].value = compute_ea(rm, disp);

        pretty_print(bytes, 3, string);
        PC+=2;
        break;
    case 0b11:

        if (d == 1)
            asprintf(&string, "%s %s, %s", op_name, registers_name[w][reg], registers_name[w][rm]);
        else
            asprintf(&string, "%s %s, %s", op_name, registers_name[w][rm], registers_name[w][reg]);
        
        op->operands[reg_idx].type = OP_REG;
        op->operands[reg_idx].value = reg;
        op->operands[mem_idx].type = OP_REG;
        op->operands[mem_idx].value = rm;

        pretty_print(bytes, 2, string);
        PC++;
        break;
    default:
        break;
    }
    return op;
}

operation * w_mod_reg_rm(char* op_name, uint8_t current)
{
    int w = LASTBIT1(current);
    return mod_reg_rm(op_name, current, 0, w);
}

operation * d_v_mod_reg_rm(char* op_name, uint8_t current)
{
    int d = LASTBIT2(current);
    int w = LASTBIT1(current);
    return mod_reg_rm(op_name, current, d, w);
}

operation * w_reg_data(char * op_name, uint8_t current)
{
    operation * op = malloc(sizeof(operation));
    op->name = op_name;
    op->nb_operands = 2;

    uint8_t bytes[4];
    bytes[0] = current;
    int w = (current & 0b00001000) >> 3;
    uint8_t reg = (current & 0b00000111);
    uint16_t data;
    current = text[PC + 1];
    bytes[1] = current;
    data = bytes[1];

    op->operands[0].type = OP_REG;
    op->operands[0].value = reg;
    op->operands[1].type = OP_IMM;
    op->w = w;

    if (w)
    {
        bytes[2] = text[PC + 2];
        data = bytes[2] << 8 | bytes[1];
    }

    op->operands[1].value = data;

    char* string;
    asprintf(&string, "%s %s, %04x", op_name, registers_name[w][reg], data);
    pretty_print(bytes, 2 + w, string);
    PC += 1 + w;
    return op;
}

int read_data(uint16_t* p_data, uint8_t* bytes, int s, int w, int idx)
{
    uint16_t data;
    uint8_t current = text[PC + idx];
    bytes[idx] = current;
    data = current;
    int acc = 0;

    if (s == 0 && w == 1)
    {
        bytes[idx + 1] = text[PC + idx + 1];
        data = bytes[idx + 1] << 8 | bytes[idx];
        acc = 1;
    }
    if (p_data != NULL)
        *p_data = data;
    return acc;
}

operation * s_w_data(char* op_name, uint8_t mod, uint8_t rm, int s, int w, uint8_t * bytes)
{
    operation * op = malloc(sizeof(operation));
    op->name = op_name;
    op->nb_operands = 2;
    op->w = w;

    uint16_t data;
    uint16_t disp;
    int acc;
    char* string;
    op->op1_type = OP_IMM;

    switch (mod)
    {
    case 0b00:
        op->op0_type = OP_MEM;

        if (rm == 0b110)
        {
            // we have to read the disp byte
            bytes[2] = text[PC + 2];
            bytes[3] = text[PC + 3];
            disp = bytes[3] << 8 | bytes[2];
            acc = read_data(&data, bytes, s, w, 4);

            if (acc == 1)
                asprintf(&string, "%s [%04x], %04x", op_name, disp, data); 
            else
                asprintf(&string, "%s [%04x], %x", op_name, disp, data);

            op->op0_value = disp;

            pretty_print(bytes, 5 + acc, string);
            PC += 4 + acc;
        }
        else
        {
            acc = read_data(&data, bytes, s, w, 2);

            if (acc == 1)
                asprintf(&string, "%s %s, %04x", op_name, rm_string(rm, 0), data); 
            else
                asprintf(&string, "%s %s, %x", op_name, rm_string(rm, 0), data);

            op->op0_value = compute_ea(rm, 0);

            pretty_print(bytes, 3 + acc, string);
            PC += 2 + acc;
        }
        
        break;
    case 0b10:
        op->op0_type = OP_MEM;

        // we have to read the disp word
        bytes[2] = text[PC + 2];
        bytes[3] = text[PC + 3];
        disp = bytes[3] << 8 | bytes[2];
        disp = (int16_t)disp;
        acc = read_data(&data, bytes, s, w, 4);

        if (acc == 1)
            asprintf(&string, "%s %s, %04x", op_name, rm_string(rm, disp), data); 
        else
            asprintf(&string, "%s %s, %x", op_name, rm_string(rm, disp), data);

        op->op0_value = compute_ea(rm, disp);

        pretty_print(bytes, 5 + acc, string);
        PC += 4 + acc;
        break;
    case 0b01:
        op->op0_type = OP_MEM;

        // we have to read the disp byte
        bytes[2] = text[PC + 2];
        disp = (int8_t)bytes[2];
        acc = read_data(&data, bytes, s, w, 3);

        if (acc == 1)
            asprintf(&string, "%s %s, %04x", op_name, rm_string(rm, disp), data); 
        else
            asprintf(&string, "%s %s, %x", op_name, rm_string(rm, disp), data);
        
        op->op0_value = compute_ea(rm, disp);

        pretty_print(bytes, 4 + acc, string);
        PC += 3 + acc;
        break;
    case 0b11:
        op->op0_type = OP_REG;

        acc = read_data(&data, bytes, s, w, 2);

        if (acc == 1)
            asprintf(&string, "%s %s, %04x", op_name, registers_name[w][rm], data); 
        else
        {
            int8_t signed_data = (int8_t)data;
            asprintf(&string, "%s %s, %s%x", op_name, registers_name[w][rm], signed_data < 0 ? "-" : "", signed_data < 0 ? -signed_data : signed_data);
        }

        op->op0_value = rm;

        pretty_print(bytes, 3 + acc, string);
        PC += 2 + acc;
        break;
    default:
        break;
    }

    op->op1_value = data;
    return op;
}

void reg(char* op_name, uint8_t current)
{
    uint8_t bytes[2];
    bytes[0] = current;
    uint8_t reg = current & 0b00000111;
    char* string;
    if (strcmp(op_name, "xchg") == 0)
        asprintf(&string, "%s %s, ax", op_name, registers_name[1][reg]);
    else
        asprintf(&string, "%s %s", op_name, registers_name[1][reg]);
    pretty_print(bytes, 1, string);
}

void jump_short(char* op_name, uint8_t current)
{
    uint8_t bytes[4];
    bytes[0] = current;
    current = text[PC + 1];
    bytes[1] = current;
    int8_t signed_disp = (int8_t)bytes[1];

    uint16_t addr = PC + signed_disp + 2;
    char* string;
    asprintf(&string, "%s %04x", op_name, addr);
    pretty_print(bytes, 2, string);
    PC++;
}

void jump_long(char* op_name, uint8_t current)
{
    uint8_t bytes[4];
    bytes[0] = current;
    uint16_t disp;
    bytes[1] = text[PC + 1];
    bytes[2] = text[PC + 2];
    disp = bytes[2] << 8 | bytes[1];
    int16_t signed_disp = (int16_t)disp;

    uint16_t addr = PC + signed_disp + 3;
    char* string;
    asprintf(&string, "%s %04x", op_name, addr);
    pretty_print(bytes, 3, string);
    PC += 2;
}

void call(char* op_name, int w, uint8_t mod, uint8_t rm, uint8_t * bytes, operation * op)
{
    uint16_t disp;
    char* string;
    op->name = op_name;
    op->nb_operands = 1;
    op->w = w;

    switch (mod)
    {
    case 0b00:
        if (rm == 0b110)
        {
            // we have to read the disp byte
            bytes[2] = text[PC + 2];
            bytes[3] = text[PC + 3];
            disp = bytes[3] << 8 | bytes[2];

            asprintf(&string, "%s [%04x]", op_name, disp);
            op->operands[0].type = OP_MEM;
            op->operands[0].value = disp;

            pretty_print(bytes, 4, string);
            PC+=3;
        }
        else
        {
            asprintf(&string, "%s %s", op_name, rm_string(rm, 0));
            op->operands[0].type = OP_MEM;
            op->operands[0].value = compute_ea(rm, 0);

            pretty_print(bytes, 2, string);
            PC++;
        }
        
        break;
    case 0b10:
        // we have to read the disp word
        bytes[2] = text[PC + 2];
        bytes[3] = text[PC + 3];
        disp = bytes[3] << 8 | bytes[2];
        int16_t signed_disp = (int16_t)disp;

        asprintf(&string, "%s %s", op_name, rm_string(rm, signed_disp));
        op->operands[0].type = OP_MEM;
        op->operands[0].value = compute_ea(rm, signed_disp);

        pretty_print(bytes, 4, string);
        PC+=3;
        break;
    case 0b01:
        // we have to read the disp byte
        bytes[2] = text[PC + 2];
        disp = (int8_t)bytes[2];

        asprintf(&string, "%s %s", op_name, rm_string(rm, disp)); 
        op->operands[0].type = OP_MEM;
        op->operands[0].value = compute_ea(rm, disp);

        pretty_print(bytes, 3, string);
        PC+=2;
        break;
    case 0b11:

        asprintf(&string, "%s %s", op_name, registers_name[w][rm]);
        op->operands[0].type = OP_REG;
        op->operands[0].value = rm;

        pretty_print(bytes, 2, string);
        PC++;
        break;
    default:
        break;
    }
}

void in_out(char* op_name, uint8_t current, int has_port)
{
    uint8_t bytes[2];
    bytes[0] = current;
    int w = LASTBIT1(current);
    char* string;
    if (has_port)
    {
        uint8_t port = text[PC + 1];
        bytes[1] = port;
        asprintf(&string, "%s %s, %02x", op_name, registers_name[w][0], port);
    }
    else
        asprintf(&string, "%s %s, dx", op_name, registers_name[0][w]);
    pretty_print(bytes, 1 + has_port, string);
    PC += has_port;
}

void just_command(char * op_name, uint8_t current)
{
    uint8_t bytes[2];
    bytes[0] = current;
    char* string;
    asprintf(&string, "%s", op_name);
    pretty_print(bytes, 1, string);
}

operation * command_arg(char * op_name, uint8_t current)
{
    operation * op = malloc(sizeof(operation));
    op->name = op_name;
    op->nb_operands = 1;

    uint8_t bytes[2];
    bytes[0] = current;
    uint8_t arg = text[PC + 1];
    bytes[1] = arg;
    op->operands[0].type = OP_IMM;
    op->operands[0].value = arg;
    char* string;
    asprintf(&string, "%s %02x", op_name, arg);
    pretty_print(bytes, 2, string);
    PC++;
    return op;
}

void immediate_from_acc(char * op_name, uint8_t current)
{
    uint8_t bytes[3];
    bytes[0] = current;
    int w = LASTBIT1(current);
    uint16_t arg = text[PC + 1];
    bytes[1] = arg;
    char* string;
    if (w == 1)
    {
        bytes[2] = text[PC + 2];
        arg = bytes[2] << 8 | bytes[1];
        int16_t signed_arg = (int16_t)arg;
        asprintf(&string, "%s %s, %04x", op_name, registers_name[w][0], signed_arg);
    }
    else
    {
        int8_t signed_arg = (int8_t)arg;
        asprintf(&string, "%s %s, %x", op_name, registers_name[w][0], signed_arg);
    }
    pretty_print(bytes, 2 + w, string);
    PC += 1 + w;

}

void mem_acc(char * op_name, uint8_t current, int d)
{
    uint8_t bytes[3];
    bytes[0] = current;
    int w = LASTBIT1(current);
    uint16_t arg = text[PC + 1];
    bytes[1] = arg;
    char* string;
    if (w == 1)
    {
        bytes[2] = text[PC + 2];
        arg = bytes[2] << 8 | bytes[1];
        int16_t signed_arg = (int16_t)arg;
        if (d == 0)
            asprintf(&string, "%s ax, [%04x]", op_name, signed_arg);
        else
            asprintf(&string, "%s [%04x], ax", op_name, signed_arg);
    }
    else
    {
        int8_t signed_arg = (int8_t)arg;
        if (d == 0)
            asprintf(&string, "%s al, [%x]", op_name, signed_arg);
        else
            asprintf(&string, "%s [%x], al", op_name, signed_arg);
    }
    pretty_print(bytes, 2 + w, string);
    PC += 1 + w;
}

void rep(uint8_t current)
{
    uint8_t bytes[2];
    bytes[0] = current;
    current = text[PC + 1];
    bytes[1] = current;
    int w = LASTBIT1(current);
    char* string;
    char* arg = NULL;
    char w_str;

    switch(BM7(current))
    {
        case MOVS:
            arg = "movs";
            break;
        case CMPS:
            arg = "cmps";
            break;
        case SCAS:
            arg = "scas";
            break;
        case LODS:
            arg = "lods";
            break;
        case STOS:
            arg = "stos";
            break;
        default:
            printf("undefined\n");
            break;
    }

    if (w == 1)
        w_str = 'w';
    else
        w_str = 'b';

    asprintf(&string, "rep %s%c", arg, w_str);
    pretty_print(bytes, 2, string);
    PC++;
}

// Special values:
// 0b1111111 (push, inc, dec, call, call, jmp, jmp)

operation * special1(uint8_t current)
{
    operation * op = malloc(sizeof(operation));
    uint8_t bytes[6];
    bytes[0] = current;
    int w = LASTBIT1(current);
    current = text[PC + 1];
    bytes[1] = current;
    uint8_t flag = FLAG(current);
    uint8_t mod = MOD(current);
    uint8_t rm = RM(current);

    switch(flag)
    {
        case PUSH1:
            call("push", 1, mod, rm, bytes, op);
            break;
        case CALL2:
            call("call", 1, mod, rm, bytes, op);
            break;
        case CALL4:
            call("call", 1, mod, rm, bytes, op);
            break;
        case JMP3:
            call("jmp", 1, mod, rm, bytes, op);
            break;
        case JMP5:
            call("jmp", 1, mod, rm, bytes, op);
            break;
        case INC1:
            call("inc", w, mod, rm, bytes, op);
            break;
        case DEC1:
            call("dec", w, mod, rm, bytes, op);
            break;
        default:
            printf("undefined\n");
            break;
    }
    return op;
}

// 0b100000 (add, addc, cmp, sub, ssb, or, xor, and)

operation * special2(uint8_t current)
{
    uint8_t bytes[6];
    bytes[0] = current;
    int s = LASTBIT2(current);
    int w = LASTBIT1(current);
    current = text[PC + 1];
    bytes[1] = current;
    uint16_t flag = FLAG(current);
    uint16_t mod = MOD(current);
    uint16_t rm = RM(current);
    char * op_name = "";

    switch(flag)
    {
        case ADD2:
            op_name = "add";
            break;
        case SSB2:
            op_name = "ssb";
            break;
        case CMP2:
            if (w == 1)
                op_name = "cmp";
            else
                op_name = "cmp byte";
            break;
        case OR2:
            op_name = "or";
            break;
        case SUB2:
            op_name = "+sub";
            break;
        case AND2:
            op_name = "and";
            break;
        default:
            printf("undefined\n");
            break;
    }
    return s_w_data(op_name, mod, rm, s, w, bytes);
}

// 0b1111011 (neg, mul, imul, div, idiv, not, test)

void special3(uint8_t current)
{
    operation * op = malloc(sizeof(operation));
    uint8_t bytes[6];
    bytes[0] = current;
    int w = LASTBIT1(current);
    current = text[PC + 1];
    bytes[1] = current;
    uint8_t flag = FLAG(current);
    uint8_t mod = MOD(current);
    uint8_t rm = RM(current);

    switch(flag)
    {
        case NEG:
            call("neg", w, mod, rm, bytes, op);
            break;
        case TEST2:
            if (w == 0 && mod == 0b01)
                s_w_data("test byte", mod, rm, 0, w, bytes);
            else
                s_w_data("test", mod, rm, 0, w, bytes);
            break;
        case MUL:
            call("mul", w, mod, rm, bytes, op);
            break;
        case IMUL:
            call("imul", w, mod, rm, bytes, op);
            break;
        case DIV:
            call("div", w, mod, rm, bytes, op);
            break;
        case IDIV: 
            call("idiv", w, mod, rm, bytes, op);
            break;
        default:
            printf("undefined\n");
            break;
    }
}

// 0b110100 (shl, shr, sar, rol, ror, rcl, rcr)

void special4(uint8_t current)
{
    uint8_t bytes[6];
    bytes[0] = current;
    int v = LASTBIT2(current);
    int w = LASTBIT1(current);
    current = text[PC + 1];
    bytes[1] = current;
    uint16_t flag = FLAG(current);
    uint16_t mod = MOD(current);
    uint16_t rm = RM(current);

    switch(flag)
    {
        case SHL:
            v_w_mod_rm("shl", v, w, mod, rm, bytes);
            break;
        case SHR:
            v_w_mod_rm("shr", v, w, mod, rm, bytes);
            break;
        case SAR:
            v_w_mod_rm("sar", v, w, mod, rm, bytes);
            break;
        case ROL:
            v_w_mod_rm("rol", v, w, mod, rm, bytes);
            break;
        case ROR:
            v_w_mod_rm("ror", v, w, mod, rm, bytes);
            break;
        case RCL:
            v_w_mod_rm("rcl", v, w, mod, rm, bytes);
            break;
        case RCR:
            v_w_mod_rm("rcr", v, w, mod, rm, bytes);
            break;
        default:
            printf("undefined\n");
            break;
    }
}

void read_file(FILE* file, uint32_t* text_length, uint32_t* data_length)
{
    uint32_t header;
    fread(&header, sizeof(header), 1, file);
    fread(&header, sizeof(header), 1, file);

    fread(text_length, sizeof(*text_length), 1, file);
    fread(data_length, sizeof(*data_length), 1, file);

    fread(&header, sizeof(header), 1, file);
    fread(&header, sizeof(header), 1, file);

    fread(&header, sizeof(header), 1, file);
    fread(&header, sizeof(header), 1, file);
    // read useless data (ez)

    printf("size: %u\n", (unsigned int) *text_length);
    text = malloc(*text_length * sizeof(uint8_t));
    data = malloc(*data_length * sizeof(uint8_t));

    uint8_t current;

    for (uint32_t i = 0; i < *text_length; i++)
    {
        fread(&current, sizeof(current), 1, file);
        // printf("i:%i, current:%02x (text)\n", i, current);
        text[i] = current;
    }

    for (uint32_t i = 0; i < *data_length; i++)
    {
        fread(&current, sizeof(current), 1, file);
        // printf("i:%i, current:%02x (data)\n", i, current);
        data[i] = current;
        memory[i] = current;
    }
}

void disassembler(uint32_t text_length)
{
    printf(" AX   BX   CX   DX   SP   BP   SI   DI  FLAGS IP\n");

    uint8_t current;
    operation * op;

    for (PC = 0; PC < text_length - 1; PC++)
    {
        current = text[PC];
        op = NULL;

        // printf("%04x, %02x ", PC, current);
        if (BM7(current) == SPECIAL1)
            op = special1(current);
        else if (BM6(current) == SPECIAL2)
            special2(current);
        else if (BM7(current) == SPECIAL3)
            special3(current);
        else if (BM6(current) == SPECIAL4)
            special4(current);
        else if (BM6(current) == MOV1)
            op = d_v_mod_reg_rm("mov", current);
        else if (BM4(current) == MOV3)
            op = w_reg_data("+mov", current);
        else if (BM6(current) == XOR1)
            op = d_v_mod_reg_rm("xor", current);
        else if (BM6(current) == ADD1)
            op = d_v_mod_reg_rm("add", current);
        else if (BM6(current) == CMP1)
            op = d_v_mod_reg_rm("cmp", current);
        else if (BM6(current) == OR1)
            op = d_v_mod_reg_rm("or", current);
        else if (current == LEA)
            op = mod_reg_rm("lea", current, 1, 1);
        else if (current == JE)
            jump_short("je", current);
        else if (current == JL)
            jump_short("jl", current);
        else if (current == JLE)
            jump_short("jle", current);
        else if (current == JB)
            jump_short("jb", current);
        else if (current == JBE)
            jump_short("jbe", current);
        else if (current == JP)
            jump_short("jp", current);
        else if (current == JO)
            jump_short("jo", current);
        else if (current == JS)
            jump_short("js", current);
        else if (current == JNE)
            jump_short("jne", current);
        else if (current == JNL)
            jump_short("jnl", current);
        else if (current == JNLE)
            jump_short("jnle", current);
        else if (current == JNB)
            jump_short("jnb", current);
        else if (current == JNBE)
            jump_short("jnbe", current);
        else if (current == JNO)
            jump_short("jno", current);
        else if (current == JNS)
            jump_long("jns", current);
        else if (BM5(current) == PUSH2)
            reg("push", current);
        else if (current == CALL1)
            jump_long("call", current);
        else if (current == JMP1)
            jump_long("jmp", current);
        else if (current == JMP2)
            jump_short("jmp short", current);
        else if (BM5(current) == DEC2)
            reg("dec", current);
        else if (BM5(current) == INC2)
            reg("inc", current);
        else if (current == HLT)
            just_command("hlt", current);
        else if (BM5(current) == POP2)
            reg("pop", current);
        else if (BM6(current) == AND1)
            op = d_v_mod_reg_rm("and", current);
        else if (BM7(current) == AND3)
            immediate_from_acc("and", current);
        else if (BM7(current) == IN1)
            in_out("in", current, 1);
        else if (BM7(current) == IN2)
            in_out("in", current, 0);   
        else if (BM7(current) == OUT1)
            in_out("out", current, 1);
        else if (BM7(current) == OUT2)
            in_out("out", current, 0);
        else if (BM6(current) == SSB1)
            op = d_v_mod_reg_rm("sbb", current);
        else if (BM6(current) == SUB1)
            op = d_v_mod_reg_rm("sub", current);
        else if (current == INT1)
            op = command_arg("+int", current);
        else if (current == RET1 || current == RET3)
            just_command("ret", current);
        else if (current == XLAT)
            just_command("xlat", current);
        else if (current == CBW)
            just_command("cbw", current);
        else if (current == CWD)
            just_command("cwd", current);
        else if (BM7(current) == SUB3)
            immediate_from_acc("sub", current);
        else if (BM7(current) == MOV2)
        {
            uint8_t bytes[6];
            bytes[0] = current;
            int w = LASTBIT1(current);
            current = text[PC + 1];
            bytes[1] = current;
            if (w == 1)
                s_w_data("mov", MOD(current), RM(current), 0, w, bytes);
            else
                s_w_data("mov byte", MOD(current), RM(current), 0, w, bytes);
        }
        else if (BM7(current) == ADD3)
            immediate_from_acc("add", current);
        else if (BM7(current) == CMP3)
            immediate_from_acc("cmp", current);
        else if (current == CLD)
            just_command("cld", current);
        else if (BM7(current) == TEST3)
            immediate_from_acc("test", current);
        else if (BM7(current) == REP)
            rep(current);
        else if (current == STD)
            just_command("std", current);
        else if (BM6(current) == ADC1)
            op = d_v_mod_reg_rm("adc", current);
        else if (BM6(current) == ADC3)
            immediate_from_acc("adc", current);
        else if(BM7(current) == TEST1)
            op = w_mod_reg_rm("test", current);
        else if(BM7(current) == XCHG1)
            op = w_mod_reg_rm("xchg", current);
        else if(BM6(current) == XCHG2)
            reg("xchg", current);
        else if (current == RET2)
        {
            uint8_t bytes[3];
            bytes[0] = current;
            uint16_t data;
            read_data(&data, bytes, 0, 1, 1);
            char* string;
            asprintf(&string, "ret %04x", data);
            pretty_print(bytes, 3, string);
            PC += 2;
        }
        else if (current == LOOP)
            jump_short("loop", current);
        else if (BM7(current) == MOV4)
            mem_acc("mov", current, 0);
        else if (BM7(current) == MOV5)
            mem_acc("mov", current, 1);
        else
            printf("undefined\n");

        if (op != NULL)
        {
            interpreter(op);
            free(op);
        }
    }

    if (PC == text_length - 1)
    {
        uint8_t bytes[] = {0};
        pretty_print(bytes, 1, "(undefined)");
    }
}