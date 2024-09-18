#include "frame-new.h"
#include "misc.h"
#include "fileio.h"
#include <stdlib.h>
#include <string.h>

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