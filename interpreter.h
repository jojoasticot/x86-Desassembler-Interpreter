#pragma once

void interpreter(operation * op);

typedef struct
{
    char * name;
    void (*func)(operation *);
} FunctionMap;