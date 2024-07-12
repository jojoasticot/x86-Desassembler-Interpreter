#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "minix/type.h"
#include "minix/callnr.h"

void interrupt(message * msg)
{
    // empty comment
    printf("\n");

    switch (msg->m_type)
    {
        case EXIT:
            printf("<exit(%i)>\n", msg->m1_i1); // status of exit is stored in m1_i1
            exit(msg->m1_i1);
            break;
        case WRITE:
        {
            registers[AX] = 0;
            uint16_t msg_adress = msg->m1_p1;
            char s [msg->m1_i2 + 1];

            for (int i = 0; i < msg->m1_i2; i++)
                s[i] = memory[msg_adress + i];
            s[msg->m1_i2] = '\0';

            printf("<write(%i, 0x%04x, %i)%s => %i>\n", msg->m1_i1, msg_adress, msg->m1_i2, s, msg->m1_i2);
            * (uint16_t*) &memory [registers[BX] + 2] = msg->m1_i2;
            break;
        }
        case IOCTL:
            registers[AX] = 0;

            int fd = msg->m2_i1;
            uint16_t request = msg->m2_i3;
            uint16_t data = msg->m2_p1;
            printf("<ioctl(%i, 0x%04x, 0x%04x)>\n", fd, request, data);

            * (uint16_t*) &memory [registers[BX] + 2] = 0xffea;
            break;
        case BRK:
            registers[AX] = 0;
            uint16_t brk = msg->m1_p1;
            printf("<brk(0x%04x) => 0>\n", brk);

            * (uint16_t*) &memory [registers[BX] + 2] = 0;
            * (uint16_t*) &memory [registers[BX] + 18] = brk;
            break;
        default:
            printf("Error: system call (%i) not supported\n", msg->m_type);
    }
}