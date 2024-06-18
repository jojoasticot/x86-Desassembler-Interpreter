#include "operation.h"
#include "main.h"
#include <stdio.h>

void print_operation(operation * op)
{
    printf("%s ", op->name);
    for (int i = 0; i < op->nb_operands; i++)
    {
        if (op->operands[i].type == OP_REG)
        {
            printf("%s", registers_name[op->w][op->operands[i].value]);
        }
        else if (op->operands[i].type == OP_MEM)
        {
            printf("0x%x", op->operands[i].value);
        }
        else if (op->operands[i].type == OP_IMM)
        {
            printf("0x%x", op->operands[i].value);
        }
        if (i < op->nb_operands - 1)
            printf(", ");
    }
    printf("\n");
}
