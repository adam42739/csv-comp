#include <stdio.h>
#include "misc.h"

#define FILEIO_WRITE_BIN "wb"
#define FILEIO_READ_BIN "rb"

#define FILEIO_WRITE_ASCII "w"
#define FILEIO_READ_ASCII "r"

void fileio_write(const char* path, void* bytes, int size, const char* mode);

void fileio_read(const char* path, void** bytes, int* size, const char* mode);

