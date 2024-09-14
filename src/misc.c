#include "misc.h"

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