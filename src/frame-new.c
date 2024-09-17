#include "frame-new.h"
#include "misc.h"
#include "fileio.h"
#include <stdlib.h>

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

static void **csv_get_cols(char *bytes, int num_cols, int num_rows, enum csv_cdtype *cdtypes, int *index, int max_index)
{
}

#include <stdio.h>

struct frame *frame_read_csv(char const *path, enum csv_cdtype *cdtypes)
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
            void **cols = csv_get_cols(bytes, num_cols, num_rows, cdtypes, &index, csv_size);
        }
        free(bytes);
    }
    return NULL;
}

void frame_write_csv(struct frame *df, char const *path)
{
}