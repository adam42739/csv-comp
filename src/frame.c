#include "frame.h"
#include "misc.h"
#include "fileio.h"
#include <stdlib.h>
#include <string.h>

#define TRUE 1
#define FALSE 0

typedef __uint8_t u8;
typedef __uint16_t u16;
typedef __uint32_t u32;
typedef __uint64_t u64;

enum param_dtype
{
    U8,
    U16,
    U32,
    U64,
    CHAR
};

static int csv_num_cols(char *bytes, int csv_size)
{
    int count = 0;
    int index = 0;
    while (index < csv_size)
    {
        if (bytes[index] == '\n')
        {
            ++count;
            break;
        }
        else if (bytes[index] == ',')
        {
            ++count;
        }
        ++index;
    }
    return count;
}

static int csv_num_rows(char *bytes, int csv_size)
{
    int count = 0;
    int index = 0;
    while (index < csv_size)
    {
        if (bytes[index] == '\n')
        {
            ++count;
        }
        ++index;
    }
    return count - 1;
}

static char *csv_get_string(char *bytes, int *index, int max_index)
{
    int str_len = 0;
    while (bytes[*index + str_len] != ',' && bytes[*index + str_len] != '\n' && *index + str_len < max_index)
    {
        ++str_len;
    }
    char *string = mem_alloc(sizeof(char) * (str_len + 1));
    for (int i = 0; i < str_len; ++i)
    {
        string[i] = bytes[*index];
        ++*index;
    }
    string[str_len] = '\0';
    ++*index;
    return string;
}

static char **csv_get_headers(char *bytes, int num_cols, int *index, int max_index)
{
    char **headers = mem_alloc(sizeof(char *) * num_cols);
    for (int i = 0; i < num_cols; ++i)
    {
        headers[i] = csv_get_string(bytes, index, max_index);
    }
    return headers;
}

static void csv_get_row(char ***cols, char *bytes, int row_index, int *index, int max_index, int num_cols)
{
    for (int i = 0; i < num_cols; ++i)
    {
        cols[i][row_index] = csv_get_string(bytes, index, max_index);
    }
}

static char ***csv_get_cols(char *bytes, int num_cols, int num_rows, int *index, int max_index)
{
    char ***cols = mem_alloc(sizeof(char **) * num_cols);
    for (int i = 0; i < num_cols; ++i)
    {
        cols[i] = mem_alloc(sizeof(char *) * num_rows);
    }
    for (int i = 0; i < num_rows; ++i)
    {
        csv_get_row(cols, bytes, i, index, max_index, num_cols);
    }
    return cols;
}

struct frame *frame_read_csv(char const *path)
{
    int csv_size = 0;
    int success = fileio_size(path, &csv_size);
    if (success)
    {
        char *bytes = mem_alloc(csv_size);
        success = fileio_read(path, bytes, csv_size, FILEIO_READ_ASCII);
        if (success)
        {
            int num_cols = csv_num_cols(bytes, csv_size);
            int num_rows = csv_num_rows(bytes, csv_size);
            int index = 0;
            char **headers = csv_get_headers(bytes, num_cols, &index, csv_size);
            char ***cols = csv_get_cols(bytes, num_cols, num_rows, &index, csv_size);
            struct frame *df = mem_alloc(sizeof(struct frame));
            df->num_cols = num_cols;
            df->num_rows = num_rows;
            df->headers = headers;
            df->cols = cols;
            return df;
        }
        free(bytes);
    }
    return NULL;
}

static int frame_csv_hsize(struct frame *df)
{
    int size = 0;
    for (int i = 0; i < df->num_cols; ++i)
    {
        size += strlen(df->headers[i]) + sizeof(char);
    }
    return size;
}

static int frame_csv_csize(struct frame *df)
{
    int size = 0;
    for (int i = 0; i < df->num_cols; ++i)
    {
        for (int j = 0; j < df->num_rows; ++j)
        {
            size += strlen(df->cols[i][j]) + sizeof(char);
        }
    }
    return size;
}

static int frame_csv_size(struct frame *df)
{
    int size = 0;
    size += frame_csv_hsize(df);
    size += frame_csv_csize(df);
    return size;
}

static void csv_dump_string(char *bytes, int *index, char *string)
{
    int len = strlen(string);
    for (int i = 0; i < len; ++i)
    {
        bytes[*index] = string[i];
        ++*index;
    }
    bytes[*index] = ',';
    ++*index;
}

static void frame_write_csvh(char *bytes, struct frame *df, int *index)
{
    for (int i = 0; i < df->num_cols; ++i)
    {
        csv_dump_string(bytes, index, df->headers[i]);
    }
    bytes[*index - 1] = '\n';
}

static void frame_write_csvc(char *bytes, struct frame *df, int *index)
{
    for (int i = 0; i < df->num_rows; ++i)
    {
        for (int j = 0; j < df->num_cols; ++j)
        {
            csv_dump_string(bytes, index, df->cols[j][i]);
        }
        bytes[*index - 1] = '\n';
    }
}

int frame_write_csv(struct frame *df, char const *path)
{
    int size = frame_csv_size(df);
    char *bytes = mem_alloc(size);
    int index = 0;
    frame_write_csvh(bytes, df, &index);
    frame_write_csvc(bytes, df, &index);
    int success = fileio_write(path, bytes, size, FILEIO_WRITE_ASCII);
    free(bytes);
    return success;
}

void frame_free(struct frame *df)
{
    for (int i = 0; i < df->num_cols; ++i)
    {
        free(df->headers[i]);
    }
    free(df->headers);
    for (int i = 0; i < df->num_cols; ++i)
    {
        for (int j = 0; j < df->num_rows; ++j)
        {
            free(df->cols[i][j]);
        }
        free(df->cols[i]);
    }
    free(df->cols);
    free(df);
}

static int bytes_load(void *bytes, int *index, int size, void *data, enum param_dtype dtype)
{
    switch (dtype)
    {
    case U8:
    {
        if (size >= *index + sizeof(u8))
        {
            *(u8 *)data = *(u8 *)(bytes + *index);
            *index += sizeof(u8);
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    case U16:
    {
        if (size >= *index + sizeof(u8))
        {
            *(u16 *)data = *(u16 *)(bytes + *index);
            *index += sizeof(u16);
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    case U32:
    {
        if (size >= *index + sizeof(u32))
        {
            *(u32 *)data = *(u32 *)(bytes + *index);
            *index += sizeof(u32);
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    case U64:
    {
        if (size >= *index + sizeof(u64))
        {
            *(u64 *)data = *(u64 *)(bytes + *index);
            *index += sizeof(u64);
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    case CHAR:
    {
        if (size >= *index + sizeof(char))
        {
            *(char *)data = *(char *)(bytes + *index);
            *index += sizeof(char);
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    break;
    default:
        return FALSE;
        break;
    }
}

static void bytes_parse_string(char **string, void *bytes, int *index, int size)
{
    int length = 0;
    while (*index + length < size)
    {
        if (*(char *)(bytes + *index + length) == '\0')
        {
            break;
        }
        else
        {
            ++length;
        }
    }
    *string = mem_alloc(sizeof(char) * (length + 1));
    for (int i = 0; i < length; ++i)
    {
        (*string)[i] = *(char *)(bytes + *index);
        ++*index;
    }
    (*string)[length] = '\0';
    ++*index;
}

static void bytes_parseh(void *bytes, int *index, int size, int num_cols, char ***headers)
{
    *headers = mem_alloc(sizeof(char *) * num_cols);
    for (int i = 0; i < num_cols; ++i)
    {
        bytes_parse_string(&(*headers)[i], bytes, index, size);
    }
}

#include <stdio.h>

static struct frame *bytes_parse_df(void *bytes, int size)
{
    int index = 0;
    int success = 1;
    int num_rows = 0;
    success = bytes_load(bytes, &index, size, (void *)&num_rows, U64);
    if (!success)
    {
        return NULL;
    }
    int num_cols = 0;
    success = bytes_load(bytes, &index, size, (void *)&num_cols, U64);
    if (!success)
    {
        return NULL;
    }
    char **headers = NULL;
    bytes_parseh(bytes, &index, size, num_cols, &headers);
}

struct frame *frame_read_bin(char const *path)
{
    int size = 0;
    int success = fileio_size(path, &size);
    if (success)
    {
        void *bytes = mem_alloc(size);
        fileio_read(path, bytes, size, FILEIO_READ_BIN);
        struct frame *df = bytes_parse_df(bytes, size);
        free(bytes);
        return df;
    }
    else
    {
        return NULL;
    }
}

#define KB 1024
#define MB 1024 * KB

#define BYTES_INIT_SIZE 1 * MB
#define BYTES_INC_SIZE 1 * MB

static void bytes_dump(void **bytes, int *index, int *size, void *data, enum param_dtype dtype)
{
    switch (dtype)
    {
    case U8:
    {
        if (*size < *index + sizeof(u8))
        {
            *bytes = mem_realloc(*bytes, *size + BYTES_INC_SIZE);
        }
        *((u8 *)(*bytes + *index)) = *(u8 *)data;
        *index += sizeof(u8);
    }
    break;
    case U16:
    {
        if (*size < *index + sizeof(u16))
        {
            *bytes = mem_realloc(*bytes, *size + BYTES_INC_SIZE);
        }
        *((u16 *)(*bytes + *index)) = *(u16 *)data;
        *index += sizeof(u16);
    }
    break;
    case U32:
    {
        if (*size < *index + sizeof(u32))
        {
            *bytes = mem_realloc(*bytes, *size + BYTES_INC_SIZE);
        }
        *((u32 *)(*bytes + *index)) = *(u32 *)data;
        *index += sizeof(u32);
    }
    break;
    case U64:
    {
        if (*size < *index + sizeof(u64))
        {
            *bytes = mem_realloc(*bytes, *size + BYTES_INC_SIZE);
        }
        *((u64 *)(*bytes + *index)) = *(u64 *)data;
        *index += sizeof(u64);
    }
    break;
    case CHAR:
    {
        if (*size < *index + sizeof(char))
        {
            *bytes = mem_realloc(*bytes, *size + BYTES_INC_SIZE);
        }
        *((char *)(*bytes + *index)) = *(char *)data;
        *index += sizeof(char);
    }
    break;
    default:
        break;
    }
}

static void bytes_dump_string(char const *string, void *bytes, int *index, int *size)
{
    int length = strlen(string);
    for (int i = 0; i < length; ++i)
    {
        bytes_dump(&bytes, index, size, (void *)&string[i], CHAR);
    }
    bytes_dump(&bytes, index, size, (void *)&string[length], CHAR);
}

static void frame_bin_bytesh(struct frame *df, void *bytes, int *index, int *size)
{
    for (int i = 0; i < df->num_cols; ++i)
    {
        bytes_dump_string(df->headers[i], bytes, index, size);
    }
}

static int string_numeric(char const *string)
{
    int index = 0;
    int numeric = TRUE;
    while (string[index] != '\0')
    {
        if (!(string[index] >= '0' && string[index] <= '9'))
        {
            if (!(string[index] == '.'))
            {
                numeric = FALSE;
                break;
            }
        }
        ++index;
    }
    return numeric;
}

// CSVCDT_PKTINT
// CSVCDT_MPLINT
// CSVCDT_STRING

static void frame_bin_bytescl(struct frame *df, void *bytes, int *index, int *size, int row)
{
    for (int i = 0; i < df->num_rows; ++i)
    {
        if (string_numeric(df->cols[i][row]))
        {
        }
        else
        {
            size += strlen(df->cols[i][row]) + 1;
        }
    }
}

static void frame_bin_bytesc(struct frame *df, void *bytes, int *index, int *size)
{
    for (int i = 0; i < df->num_cols; ++i)
    {
        frame_bin_bytescl(df, bytes, index, size, i);
    }
}

static void frame_bin_bytes(struct frame *df, void *bytes, int *index, int *size)
{
    bytes_dump(&bytes, index, size, &df->num_rows, U64);
    bytes_dump(&bytes, index, size, &df->num_cols, U64);
    frame_bin_bytesh(df, bytes, index, size);
    // frame_bin_bytesc(df, bytes, index, size);
}

int frame_write_bin(struct frame *df, char const *path)
{
    int size = BYTES_INIT_SIZE;
    void *bytes = mem_alloc(size);
    int index = 0;
    frame_bin_bytes(df, bytes, &index, &size);
    int success = fileio_write(path, bytes, index, FILEIO_WRITE_BIN);
    free(bytes);
    return success;
}