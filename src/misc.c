#include "misc.h"
#include <stdlib.h>

void *mem_alloc(int size)
{
    void *ptr = malloc(size);
    if (ptr)
    {
        return ptr;
    }
    else
    {
        exit(0);
    }
}

#include <stdio.h>

void *mem_realloc(void *ptr, int size)
{
    ptr = realloc(ptr, size);
    if (ptr)
    {
        return ptr;
    }
    else
    {
        exit(0);
    }
}