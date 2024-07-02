#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "minix/type.h"
#include "minix/callnr.h"

void interrupt(message * msg)
{
    switch (msg->m_type)
    {
        case EXIT:
            printf("<exit(%i)>\n", msg->m1_i1); // status of exit is stored in m1_i1
            exit(msg->m1_i1);
            break;
        case WRITE:
        {
            uint16_t msg_adress = msg->m1_p1;
            char s [msg->m1_i2 + 1];

            for (int i = 0; i < msg->m1_i2; i++)
                s[i] = memory[msg_adress + i];
            s[msg->m1_i2] = '\0';

            printf("<write(%i, 0x%04x, %i)%s => %i>\n", msg->m_source, msg_adress, msg->m1_i2, s, msg->m1_i2);
            break;
        }
        default:
            printf("Error: system call (%i) not supported\n", msg->m_type);
    }
}