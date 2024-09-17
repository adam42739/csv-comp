#pragma once

#define FILEIO_WRITE_BIN "wb"
#define FILEIO_READ_BIN "rb"

#define FILEIO_WRITE_ASCII "w"
#define FILEIO_READ_ASCII "r"

int fileio_write(char const *path, void *bytes, int size, char const *mode);

int fileio_size(char const *path, int* size);

int fileio_read(char const *path, void *bytes, int size, char const *mode);
