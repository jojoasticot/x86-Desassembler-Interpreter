#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <err.h>
#include <string.h>
#include "main.h"
#include "operation.h"

char* registers_name[2][8] = {
    {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"},
    {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"},
};

void pretty_print(uint32_t i, uint8_t* bytes, int size, char* op)
{
    char concatenated[2 * size + 1]; 
    concatenated[0] = '\0';

    for (int i = 0; i < size; i++)
    {
        char * temp;

        asprintf(&temp, "%02x", bytes[i]);

        strcat(concatenated, temp);
    }

    printf("%04x: %-14s%s\n", i, concatenated, op);
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

void v_w_mod_rm(char* op_name, uint32_t* i, int v, int w, uint8_t mod, uint8_t rm, uint8_t* bytes)
{

    uint16_t disp;
    char* string;

    switch (mod)
    {
    case 0b00:
        if (rm == 0b110)
        {
            // we have to read the disp byte
            bytes[2] = text[*i + 1];
            bytes[3] = text[*i + 2];
            disp = bytes[3] << 8 | bytes[2];

            if (v == 1)
                asprintf(&string, "%s [%04x], cl", op_name, disp);
            else
                asprintf(&string, "%s [%04x], 1", op_name, disp);

            pretty_print(*i, bytes, 4, string);
            (*i)+=3;
        }
        else
        {
            if (v == 1)
                asprintf(&string, "%s %s, cl", op_name, rm_string(rm, 0));
            else
                asprintf(&string, "%s %s, 1", op_name, rm_string(rm, 0));

            pretty_print(*i, bytes, 2, string);
            (*i)++;
        }
        
        break;
    case 0b10:
        // we have to read the disp word
        bytes[2] = text[*i + 1];
        bytes[3] = text[*i + 2];
        disp = bytes[3] << 8 | bytes[2];
        int16_t signed_disp = (int16_t)disp;

        if (v == 1)
            asprintf(&string, "%s %s, [CL]", op_name, rm_string(rm, signed_disp));
        else
            asprintf(&string, "%s %s, 1", op_name, rm_string(rm, signed_disp));

        pretty_print(*i, bytes, 4, string);
        (*i)+=3;
        break;
    case 0b01:
        // we have to read the disp byte
        bytes[2] = text[*i + 1];
        disp = (int8_t)bytes[2];

        if (v == 1)
            asprintf(&string, "%s %s, cl", op_name, rm_string(rm, disp));
        else
            asprintf(&string, "%s %s, 1", op_name, rm_string(rm, disp));

        pretty_print(*i, bytes, 3, string);
        (*i)+=2;
        break;
    case 0b11:
        if (v == 1)
            asprintf(&string, "%s %s, cl", op_name, registers_name[w][rm]);
        else
            asprintf(&string, "%s %s, 1", op_name, registers_name[w][rm]);
        pretty_print(*i, bytes, 2, string);
        (*i)++;
        break;
    default:
        break;
    }
}

void mod_reg_rm(char* op_name, uint8_t current, uint32_t* i, int d, int w)
{
    uint8_t bytes[4];
    bytes[0] = current;
    current = text[*i + 1];
    bytes[1] = current;
    uint8_t mod = MOD(current);
    uint8_t reg = REG(current);
    uint8_t rm = RM(current);
    uint16_t disp;
    char* string;

    switch (mod)
    {
    case 0b00:
        if (rm == 0b110)
        {
            // we have to read the disp byte
            bytes[2] = text[*i + 2];
            bytes[3] = text[*i + 3];
            disp = bytes[3] << 8 | bytes[2];

            if (d == 1)
                asprintf(&string, "%s %s, [%04x]", op_name, registers_name[w][reg], disp);
            else
                asprintf(&string, "%s [%04x], %s", op_name, disp, registers_name[w][reg]);

            pretty_print(*i, bytes, 4, string);
            (*i)+=3;
        }
        else
        {
            if (d == 1)
                asprintf(&string, "%s %s, %s", op_name, registers_name[w][reg], rm_string(rm, 0));
            else
                asprintf(&string, "%s %s, %s", op_name, rm_string(rm, 0), registers_name[w][reg]);

            pretty_print(*i, bytes, 2, string);
            (*i)++;
        }
        
        break;
    case 0b10:
        // we have to read the disp word
        bytes[2] = text[*i + 2];
        bytes[3] = text[*i + 3];
        disp = bytes[3] << 8 | bytes[2];
        int16_t signed_disp = (int16_t)disp;

        if (d == 1)
            asprintf(&string, "%s %s, %s", op_name, registers_name[w][reg], rm_string(rm, signed_disp));
        else
            asprintf(&string, "%s %s, %s", op_name, rm_string(rm, signed_disp), registers_name[w][reg]);

        pretty_print(*i, bytes, 4, string);
        (*i)+=3;
        break;
    case 0b01:
        // we have to read the disp byte
        bytes[2] = text[*i + 2];
        disp = (int8_t)bytes[2];

        if (d == 1)
            asprintf(&string, "%s %s, %s", op_name, registers_name[w][reg], rm_string(rm, disp));
        else
            asprintf(&string, "%s %s, %s", op_name, rm_string(rm, disp), registers_name[w][reg]);

        pretty_print(*i, bytes, 3, string);
        (*i)+=2;
        break;
    case 0b11:
        if (d == 1)
            asprintf(&string, "%s %s, %s", op_name, registers_name[w][reg], registers_name[w][rm]);
        else
            asprintf(&string, "%s %s, %s", op_name, registers_name[w][rm], registers_name[w][reg]);
        pretty_print(*i, bytes, 2, string);
        (*i)++;
        break;
    default:
        break;
    }
}

void w_mod_reg_rm(char* op_name, uint8_t current, uint32_t * i)
{
    int w = LASTBIT1(current);
    mod_reg_rm(op_name, current, i, 0, w);
}

void d_v_mod_reg_rm(char* op_name, uint8_t current, uint32_t * i)
{
    int d = LASTBIT2(current);
    int w = LASTBIT1(current);
    mod_reg_rm(op_name, current, i, d, w);
}

void w_reg_data(char * op_name, uint8_t current, uint32_t * i)
{
    uint8_t bytes[4];
    bytes[0] = current;
    int w = (current & 0b00001000) >> 3;
    uint8_t reg = (current & 0b00000111);
    uint16_t data;
    current = text[*i + 1];
    bytes[1] = current;
    data = bytes[1];

    if (w)
    {
        bytes[2] = text[*i + 2];
        data = bytes[2] << 8 | bytes[1];
    }

    char* string;
    asprintf(&string, "%s %s, %04x", op_name, registers_name[w][reg], data);
    pretty_print(*i, bytes, 2 + w, string);
    (*i) += 1 + w;
}

int read_data(uint16_t* p_data, uint8_t* bytes, int s, int w, int idx, uint32_t * i)
{
    uint16_t data;
    uint8_t current = text[*i + idx];
    bytes[idx] = current;
    data = current;
    int acc = 0;

    if (s == 0 && w == 1)
    {
        bytes[idx + 1] = text[*i + idx + 1];
        data = bytes[idx + 1] << 8 | bytes[idx];
        acc = 1;
    }
    if (p_data != NULL)
        *p_data = data;
    return acc;
}

void s_w_data(char* op_name, uint8_t mod, uint8_t rm, uint32_t * i, int s, int w, uint8_t * bytes)
{
    uint16_t data;
    uint16_t disp;
    int acc;
    char* string;

    switch (mod)
    {
    case 0b00:
        if (rm == 0b110)
        {
            // we have to read the disp byte
            bytes[2] = text[*i + 2];
            bytes[3] = text[*i + 3];
            disp = bytes[3] << 8 | bytes[2];
            acc = read_data(&data, bytes, s, w, 4, i);

            if (acc == 1)
                asprintf(&string, "%s [%04x], %04x", op_name, disp, data); 
            else
                asprintf(&string, "%s [%04x], %x", op_name, disp, data);

            pretty_print(*i, bytes, 5 + acc, string);
            (*i)+= 4 + acc;
        }
        else
        {
            acc = read_data(&data, bytes, s, w, 2, i);

            if (acc == 1)
                asprintf(&string, "%s %s, %04x", op_name, rm_string(rm, 0), data); 
            else
                asprintf(&string, "%s %s, %x", op_name, rm_string(rm, 0), data);

            pretty_print(*i, bytes, 3 + acc, string);
            (*i) += 2 + acc;
        }
        
        break;
    case 0b10:
        // we have to read the disp word
        bytes[2] = text[*i + 2];
        bytes[3] = text[*i + 3];
        disp = bytes[3] << 8 | bytes[2];
        disp = (int16_t)disp;
        acc = read_data(&data, bytes, s, w, 4, i);

        if (acc == 1)
            asprintf(&string, "%s %s, %04x", op_name, rm_string(rm, disp), data); 
        else
            asprintf(&string, "%s %s, %x", op_name, rm_string(rm, disp), data);

        pretty_print(*i, bytes, 5 + acc, string);
        (*i) += 4 + acc;
        break;
    case 0b01:
        // we have to read the disp byte
        bytes[2] = text[*i + 2];
        disp = (int8_t)bytes[2];
        acc = read_data(&data, bytes, s, w, 3, i);

        if (acc == 1)
            asprintf(&string, "%s %s, %04x", op_name, rm_string(rm, disp), data); 
        else
            asprintf(&string, "%s %s, %x", op_name, rm_string(rm, disp), data);

        pretty_print(*i, bytes, 4 + acc, string);
        (*i) += 3 + acc;
        break;
    case 0b11:
        acc = read_data(&data, bytes, s, w, 2, i);

        if (acc == 1)
            asprintf(&string, "%s %s, %04x", op_name, registers_name[w][rm], data); 
        else
        {
            int8_t signed_data = (int8_t)data;
            asprintf(&string, "%s %s, %s%x", op_name, registers_name[w][rm], signed_data < 0 ? "-" : "", signed_data < 0 ? -signed_data : signed_data);
        }

        pretty_print(*i, bytes, 3 + acc, string);
        (*i) += 2 + acc;
        break;
    default:
        break;
    }
}

void reg(char* op_name, uint8_t current, uint32_t * i)
{
    uint8_t bytes[2];
    bytes[0] = current;
    uint8_t reg = current & 0b00000111;
    char* string;
    if (strcmp(op_name, "xchg") == 0)
        asprintf(&string, "%s %s, ax", op_name, registers_name[1][reg]);
    else
        asprintf(&string, "%s %s", op_name, registers_name[1][reg]);
    pretty_print(*i, bytes, 1, string);
}

void jump_short(char* op_name, uint8_t current, uint32_t * i)
{
    uint8_t bytes[4];
    bytes[0] = current;
    current = text[*i + 1];
    bytes[1] = current;
    int8_t signed_disp = (int8_t)bytes[1];

    uint16_t addr = *i + signed_disp + 2;
    char* string;
    asprintf(&string, "%s %04x", op_name, addr);
    pretty_print(*i, bytes, 2, string);
    (*i)++;
}

void jump_long(char* op_name, uint8_t current, uint32_t * i)
{
    uint8_t bytes[4];
    bytes[0] = current;
    uint16_t disp;
    bytes[1] = text[*i + 1];
    bytes[2] = text[*i + 2];
    disp = bytes[2] << 8 | bytes[1];
    int16_t signed_disp = (int16_t)disp;

    uint16_t addr = *i + signed_disp + 3;
    char* string;
    asprintf(&string, "%s %04x", op_name, addr);
    pretty_print(*i, bytes, 3, string);
    (*i) += 2;
}

void call(char* op_name, uint32_t * i, int w, uint8_t mod, uint8_t rm, uint8_t * bytes, operation * op)
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
            bytes[2] = text[*i + 2];
            bytes[3] = text[*i + 3];
            disp = bytes[3] << 8 | bytes[2];

            asprintf(&string, "%s [%04x]", op_name, disp);
            op->operands[0].type = OP_MEM;
            op->operands[0].value = disp;

            pretty_print(*i, bytes, 4, string);
            (*i)+=3;
        }
        else
        {
            asprintf(&string, "%s %s", op_name, rm_string(rm, 0));
            op->operands[0].type = OP_MEM;
            op->operands[0].value = compute_ea(rm, 0);

            pretty_print(*i, bytes, 2, string);
            (*i)++;
        }
        
        break;
    case 0b10:
        // we have to read the disp word
        bytes[2] = text[*i + 2];
        bytes[3] = text[*i + 3];
        disp = bytes[3] << 8 | bytes[2];
        int16_t signed_disp = (int16_t)disp;

        asprintf(&string, "%s %s", op_name, rm_string(rm, signed_disp));
        op->operands[0].type = OP_MEM;
        op->operands[0].value = compute_ea(rm, signed_disp);

        pretty_print(*i, bytes, 4, string);
        (*i)+=3;
        break;
    case 0b01:
        // we have to read the disp byte
        bytes[2] = text[*i + 2];
        disp = (int8_t)bytes[2];

        asprintf(&string, "%s %s", op_name, rm_string(rm, disp)); 
        op->operands[0].type = OP_MEM;
        op->operands[0].value = compute_ea(rm, disp);

        pretty_print(*i, bytes, 3, string);
        (*i)+=2;
        break;
    case 0b11:

        asprintf(&string, "%s %s", op_name, registers_name[w][rm]);
        op->operands[0].type = OP_REG;
        op->operands[0].value = rm;

        pretty_print(*i, bytes, 2, string);
        (*i)++;
        break;
    default:
        break;
    }
}

void in_out(char* op_name, uint8_t current, uint32_t * i, int has_port)
{
    uint8_t bytes[2];
    bytes[0] = current;
    int w = LASTBIT1(current);
    char* string;
    if (has_port)
    {
        uint8_t port = text[*i + 1];
        bytes[1] = port;
        asprintf(&string, "%s %s, %02x", op_name, registers_name[w][0], port);
    }
    else
        asprintf(&string, "%s %s, dx", op_name, registers_name[0][w]);
    pretty_print(*i, bytes, 1 + has_port, string);
    (*i) += has_port;
}

void just_command(char * op_name, uint8_t current, uint32_t * i)
{
    uint8_t bytes[2];
    bytes[0] = current;
    char* string;
    asprintf(&string, "%s", op_name);
    pretty_print(*i, bytes, 1, string);
}

void command_arg(char * op_name, uint8_t current, uint32_t * i)
{
    uint8_t bytes[2];
    bytes[0] = current;
    uint8_t arg = text[*i + 1];
    bytes[1] = arg;
    char* string;
    asprintf(&string, "%s %02x", op_name, arg);
    pretty_print(*i, bytes, 2, string);
    (*i)++;
}

void immediate_from_acc(char * op_name, uint8_t current, uint32_t * i)
{
    uint8_t bytes[3];
    bytes[0] = current;
    int w = LASTBIT1(current);
    uint16_t arg = text[*i + 1];
    bytes[1] = arg;
    char* string;
    if (w == 1)
    {
        bytes[2] = text[*i + 2];
        arg = bytes[2] << 8 | bytes[1];
        int16_t signed_arg = (int16_t)arg;
        asprintf(&string, "%s %s, %04x", op_name, registers_name[w][0], signed_arg);
    }
    else
    {
        int8_t signed_arg = (int8_t)arg;
        asprintf(&string, "%s %s, %x", op_name, registers_name[w][0], signed_arg);
    }
    pretty_print(*i, bytes, 2 + w, string);
    (*i) += 1 + w;

}

void mem_acc(char * op_name, uint8_t current, uint32_t * i, int d)
{
    uint8_t bytes[3];
    bytes[0] = current;
    int w = LASTBIT1(current);
    uint16_t arg = text[*i + 1];
    bytes[1] = arg;
    char* string;
    if (w == 1)
    {
        bytes[2] = text[*i + 2];
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
    pretty_print(*i, bytes, 2 + w, string);
    (*i) += 1 + w;
}

void rep(uint8_t current, uint32_t * i)
{
    uint8_t bytes[2];
    bytes[0] = current;
    current = text[*i + 1];
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
    pretty_print(*i, bytes, 2, string);
    (*i)++;
}

// Special values:
// 0b1111111 (push, inc, dec, call, call, jmp, jmp)

operation * special1(uint8_t current, uint32_t * i)
{
    operation * op = malloc(sizeof(operation));
    uint8_t bytes[6];
    bytes[0] = current;
    int w = LASTBIT1(current);
    current = text[*i + 1];
    bytes[1] = current;
    uint8_t flag = FLAG(current);
    uint8_t mod = MOD(current);
    uint8_t rm = RM(current);

    switch(flag)
    {
        case PUSH1:
            call("push", i, 1, mod, rm, bytes, op);
            break;
        case CALL2:
            call("call", i, 1, mod, rm, bytes, op);
            break;
        case CALL4:
            call("call", i, 1, mod, rm, bytes, op);
            break;
        case JMP3:
            call("jmp", i, 1, mod, rm, bytes, op);
            break;
        case JMP5:
            call("jmp", i, 1, mod, rm, bytes, op);
            break;
        case INC1:
            call("inc", i, w, mod, rm, bytes, op);
            break;
        case DEC1:
            call("dec", i, w, mod, rm, bytes, op);
            break;
        default:
            printf("undefined\n");
            break;
    }
    return op;
}

// 0b100000 (add, addc, cmp, sub, ssb, or, xor, and)

void special2(uint8_t current, uint32_t * i)
{
    uint8_t bytes[6];
    bytes[0] = current;
    int s = LASTBIT2(current);
    int w = LASTBIT1(current);
    current = text[*i + 1];
    bytes[1] = current;
    uint16_t flag = FLAG(current);
    uint16_t mod = MOD(current);
    uint16_t rm = RM(current);

    switch(flag)
    {
        case ADD2:
            s_w_data("add", mod, rm, i, s, w, bytes);
            break;
        case SSB2:
            s_w_data("sbb", mod, rm, i, s, w, bytes);
            break;
        case CMP2:
            if (w == 1)
                s_w_data("cmp", mod, rm, i, s, w, bytes);
            else
                s_w_data("cmp byte", mod, rm, i, s, w, bytes);
            break;
        case OR2:
            s_w_data("or", mod, rm, i, s, w, bytes);
            break;
        case SUB2:
            s_w_data("sub", mod, rm, i, s, w, bytes);
            break;
        case AND2:
            s_w_data("and", mod, rm, i, s, w, bytes);
            break;
        default:
            printf("undefined\n");
            break;
    }
}

// 0b1111011 (neg, mul, imul, div, idiv, not, test)

void special3(uint8_t current, uint32_t * i)
{
    operation * op = malloc(sizeof(operation));
    uint8_t bytes[6];
    bytes[0] = current;
    int w = LASTBIT1(current);
    current = text[*i + 1];
    bytes[1] = current;
    uint8_t flag = FLAG(current);
    uint8_t mod = MOD(current);
    uint8_t rm = RM(current);

    switch(flag)
    {
        case NEG:
            call("neg", i, w, mod, rm, bytes, op);
            break;
        case TEST2:
            if (w == 0 && mod == 0b01)
                s_w_data("test byte", mod, rm, i, 0, w, bytes);
            else
                s_w_data("test", mod, rm, i, 0, w, bytes);
            break;
        case MUL:
            call("mul", i, w, mod, rm, bytes, op);
            break;
        case IMUL:
            call("imul", i, w, mod, rm, bytes, op);
            break;
        case DIV:
            call("div", i, w, mod, rm, bytes, op);
            break;
        case IDIV: 
            call("idiv", i, w, mod, rm, bytes, op);
            break;
        default:
            printf("undefined\n");
            break;
    }
}

// 0b110100 (shl, shr, sar, rol, ror, rcl, rcr)

void special4(uint8_t current, uint32_t * i)
{
    uint8_t bytes[6];
    bytes[0] = current;
    int v = LASTBIT2(current);
    int w = LASTBIT1(current);
    current = text[*i + 1];
    bytes[1] = current;
    uint16_t flag = FLAG(current);
    uint16_t mod = MOD(current);
    uint16_t rm = RM(current);

    switch(flag)
    {
        case SHL:
            v_w_mod_rm("shl", i, v, w, mod, rm, bytes);
            break;
        case SHR:
            v_w_mod_rm("shr", i, v, w, mod, rm, bytes);
            break;
        case SAR:
            v_w_mod_rm("sar", i, v, w, mod, rm, bytes);
            break;
        case ROL:
            v_w_mod_rm("rol", i, v, w, mod, rm, bytes);
            break;
        case ROR:
            v_w_mod_rm("ror", i, v, w, mod, rm, bytes);
            break;
        case RCL:
            v_w_mod_rm("rcl", i, v, w, mod, rm, bytes);
            break;
        case RCR:
            v_w_mod_rm("rcr", i, v, w, mod, rm, bytes);
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
        text[i] = current;
    }

    for (uint32_t i = 0; i < *data_length; i++)
    {
        fread(&current, sizeof(current), 1, file);
        data[i] = current;
    }
}

void disassembler(uint32_t text_length, uint32_t data_length)
{

    uint8_t current;
    operation * op;

    for (PC = 0; PC < text_length - 1; PC++)
    {
        current = text[PC];
        printf("%04x, %02x ", PC, current);
        if (BM7(current) == SPECIAL1)
            op = special1(current, &PC);
        else if (BM6(current) == SPECIAL2)
            special2(current, &PC);
        else if (BM7(current) == SPECIAL3)
            special3(current, &PC);
        else if (BM6(current) == SPECIAL4)
            special4(current, &PC);
        else if (BM6(current) == MOV1)
            d_v_mod_reg_rm("mov", current, &PC);
        else if (BM4(current) == MOV3)
            w_reg_data("mov", current, &PC);
        else if (BM6(current) == XOR1)
            d_v_mod_reg_rm("xor", current, &PC);
        else if (BM6(current) == ADD1)
            d_v_mod_reg_rm("add", current, &PC);
        else if (BM6(current) == CMP1)
            d_v_mod_reg_rm("cmp", current, &PC);
        else if (BM6(current) == OR1)
            d_v_mod_reg_rm("or", current, &PC);
        else if (current == LEA)
            mod_reg_rm("lea", current, &PC, 1, 1);
        else if (current == JE)
            jump_short("je", current, &PC);
        else if (current == JL)
            jump_short("jl", current, &PC);
        else if (current == JLE)
            jump_short("jle", current, &PC);
        else if (current == JB)
            jump_short("jb", current, &PC);
        else if (current == JBE)
            jump_short("jbe", current, &PC);
        else if (current == JP)
            jump_short("jp", current, &PC);
        else if (current == JO)
            jump_short("jo", current, &PC);
        else if (current == JS)
            jump_short("js", current, &PC);
        else if (current == JNE)
            jump_short("jne", current, &PC);
        else if (current == JNL)
            jump_short("jnl", current, &PC);
        else if (current == JNLE)
            jump_short("jnle", current, &PC);
        else if (current == JNB)
            jump_short("jnb", current, &PC);
        else if (current == JNBE)
            jump_short("jnbe", current, &PC);
        else if (current == JNO)
            jump_short("jno", current, &PC);
        else if (current == JNS)
            jump_long("jns", current, &PC);
        else if (BM5(current) == PUSH2)
            reg("push", current, &PC);
        else if (current == CALL1)
            jump_long("call", current, &PC);
        else if (current == JMP1)
            jump_long("jmp", current, &PC);
        else if (current == JMP2)
            jump_short("jmp short", current, &PC);
        else if (BM5(current) == DEC2)
            reg("dec", current, &PC);
        else if (BM5(current) == INC2)
            reg("inc", current, &PC);
        else if (current == HLT)
            just_command("hlt", current, &PC);
        else if (BM5(current) == POP2)
            reg("pop", current, &PC);
        else if (BM6(current) == AND1)
            d_v_mod_reg_rm("and", current, &PC);
        else if (BM7(current) == AND3)
            immediate_from_acc("and", current, &PC);
        else if (BM7(current) == IN1)
            in_out("in", current, &PC, 1);
        else if (BM7(current) == IN2)
            in_out("in", current, &PC, 0);   
        else if (BM7(current) == OUT1)
            in_out("out", current, &PC, 1);
        else if (BM7(current) == OUT2)
            in_out("out", current, &PC, 0);
        else if (BM6(current) == SSB1)
            d_v_mod_reg_rm("sbb", current, &PC);
        else if (BM6(current) == SUB1)
            d_v_mod_reg_rm("sub", current, &PC);
        else if (current == INT1)
            command_arg("int", current, &PC);
        else if (current == RET1 || current == RET3)
            just_command("ret", current, &PC);
        else if (current == XLAT)
            just_command("xlat", current, &PC);
        else if (current == CBW)
            just_command("cbw", current, &PC);
        else if (current == CWD)
            just_command("cwd", current, &PC);
        else if (BM7(current) == SUB3)
            immediate_from_acc("sub", current, &PC);
        else if (BM7(current) == MOV2)
        {
            uint8_t bytes[6];
            bytes[0] = current;
            int w = LASTBIT1(current);
            current = text[PC + 1];
            bytes[1] = current;
            if (w == 1)
                s_w_data("mov", MOD(current), RM(current), &PC, 0, w, bytes);
            else
                s_w_data("mov byte", MOD(current), RM(current), &PC, 0, w, bytes);
        }
        else if (BM7(current) == ADD3)
            immediate_from_acc("add", current, &PC);
        else if (BM7(current) == CMP3)
            immediate_from_acc("cmp", current, &PC);
        else if (current == CLD)
            just_command("cld", current, &PC);
        else if (BM7(current) == TEST3)
            immediate_from_acc("test", current, &PC);
        else if (BM7(current) == REP)
            rep(current, &PC);
        else if (current == STD)
            just_command("std", current, &PC);
        else if (BM6(current) == ADC1)
            d_v_mod_reg_rm("adc", current, &PC);
        else if (BM6(current) == ADC3)
            immediate_from_acc("adc", current, &PC);
        else if(BM7(current) == TEST1)
            w_mod_reg_rm("test", current, &PC);
        else if(BM7(current) == XCHG1)
            w_mod_reg_rm("xchg", current, &PC);
        else if(BM6(current) == XCHG2)
            reg("xchg", current, &PC);
        else if (current == RET2)
        {
            uint8_t bytes[3];
            bytes[0] = current;
            uint16_t data;
            read_data(&data, bytes, 0, 1, 1, &PC);
            char* string;
            asprintf(&string, "ret %04x", data);
            pretty_print(PC, bytes, 3, string);
            PC += 2;
        }
        else if (current == LOOP)
            jump_short("loop", current, &PC);
        else if (BM7(current) == MOV4)
            mem_acc("mov", current, &PC, 0);
        else if (BM7(current) == MOV5)
            mem_acc("mov", current, &PC, 1);
        else
            printf("undefined\n");
    }

    if (PC == text_length - 1)
    {
        uint8_t bytes[] = {0};
        pretty_print(text_length - 1, bytes, 1, "(undefined)");
    }
}