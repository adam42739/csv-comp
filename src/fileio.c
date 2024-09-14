#include "fileio.h"

void fileio_write(const char *path, void *bytes, int size, const char *mode)
{
    FILE *file = fopen(path, mode);
    if (file)
    {
        fwrite(bytes, size, 1, file);
    }
}

void fileio_read(const char *path, void **bytes, int *size, const char *mode)
{
    FILE *file = fopen(path, mode);
    if (file)
    {
        fseek(file, 0L, SEEK_END);
        int file_size = ftell(file);
        rewind(file);
        if (file_size > *size)
        {
            free(*bytes);
            *bytes = mem_alloc(file_size);
        }
        *size = file_size;
        fread(*bytes, file_size, 1, file);
    }
}
