#include "fileio.h"
#include <stdlib.h>
#include <stdio.h>
#include "misc.h"

#define TRUE 1
#define FALSE 0

int fileio_write(char const *path, void *bytes, int size, char const *mode)
{
    FILE *file = fopen(path, mode);
    if (file)
    {
        fwrite(bytes, size, 1, file);
        fclose(file);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

int fileio_size(char const *path, int *size)
{
    FILE *file = fopen(path, FILEIO_READ_BIN);
    if (file)
    {
        fseek(file, 0L, SEEK_END);
        *size = ftell(file);
        fclose(file);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

int fileio_read(char const *path, void *bytes, int size, char const *mode)
{
    FILE *file = fopen(path, mode);
    if (file)
    {
        fread(bytes, size, 1, file);
        fclose(file);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
