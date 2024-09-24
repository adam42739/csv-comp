#include "frame.h"
#include "misc.h"
#include "fileio.h"
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <stdio.h>

#define TRUE 1
#define FALSE 0

#define MAX(A, B) (A > B ? A : B)

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

enum binbytes_indchar
{
    STRING_SAMES = 0,
    STRING_DIFFS = 1,
    U8_SAMES = 2,
    U8_DIFFS = 3,
    U8_INC = 4,
    U8_DEC = 5,
    U16_SAMES = 6,
    U16_DIFFS = 7,
    U16_INC = 8,
    U16_DEC = 9,
    U32_SAMES = 10,
    U32_DIFFS = 11,
    U32_INC = 12,
    U32_DEC = 13,
    U64_SAMES = 14,
    U64_DIFFS = 15,
    U64_INC = 16,
    U64_DEC = 17,
};

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

static int bytes_parse_string(char **string, void *bytes, int *index, int size)
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
    return length;
}

static void bytes_parseh(void *bytes, int *index, int size, int num_cols, char ***headers)
{
    *headers = mem_alloc(sizeof(char *) * num_cols);
    for (int i = 0; i < num_cols; ++i)
    {
        (void)bytes_parse_string(&(*headers)[i], bytes, index, size);
    }
}

static int load_strsame(void *bytes, int *index, int size, char **col, int *row)
{
    int success = TRUE;
    u16 num_same = 0;
    success = bytes_load(bytes, index, size, (void *)&num_same, U16);
    if (!success)
    {
        return FALSE;
    }
    int length = 0;
    for (int i = 0; i < num_same; ++i)
    {
        length = bytes_parse_string(&col[*row], bytes, index, size);
        *index -= length + 1;
        ++*row;
    }
    *index += length + 1;
    return TRUE;
}

static int load_strdiff(void *bytes, int *index, int size, char **col, int *row)
{
    int success = TRUE;
    u16 num_diff;
    success = bytes_load(bytes, index, size, (void *)&num_diff, U16);
    if (!success)
    {
        return FALSE;
    }
    for (int i = 0; i < num_diff; ++i)
    {
        (void)bytes_parse_string(&col[*row], bytes, index, size);
        ++*row;
    }
    return TRUE;
}

static char *u64_to_string(u64 value, u8 factor, int *len)
{
    int num_buff;
    if (num_buff == 0)
    {
        num_buff = 1;
    }
    else
    {
        num_buff = (int)floor(log10((double)value)) + 1;
    }
    char *buffer = mem_alloc(sizeof(char) * (num_buff + 1));
    sprintf(buffer, "%llu", value);
    buffer[num_buff] = '\0';
    int num_str = MAX(factor, num_buff) + 1;
    if (factor == 0)
    {
        --num_str;
    }
    if (factor >= num_buff)
    {
        ++num_str;
    }
    char *string = mem_alloc(sizeof(char) * (num_str + 1));
    int str_index = num_str - 1;
    int buf_index = num_buff - 1;
    int count = 0;
    while (str_index >= 0)
    {
        if (count == (int)factor && factor != 0)
        {
            string[str_index] = '.';
        }
        else if (buf_index >= 0)
        {
            string[str_index] = buffer[buf_index];
            --buf_index;
        }
        else
        {
            string[str_index] = '0';
        }
        --str_index;
        ++count;
    }
    if (factor >= num_buff)
    {
        string[0] = '0';
    }
    string[num_str] = '\0';
    *len = num_str;
    free(buffer);
    return string;
}

static int load_u8same(void *bytes, int *index, int size, char **col, int *row)
{
    int success = TRUE;
    u16 num_same = 0;
    success = bytes_load(bytes, index, size, (void *)&num_same, U16);
    if (!success)
    {
        return FALSE;
    }
    u8 factor = 0;
    success = bytes_load(bytes, index, size, (void *)&factor, U8);
    u8 value = 0;
    success = bytes_load(bytes, index, size, (void *)&value, U8);
    int len;
    char *buffer = u64_to_string((u64)value, factor, &len);
    for (int i = 0; i < num_same; ++i)
    {
        col[*row] = mem_alloc(sizeof(char) * (len + 1));
        memcpy(col[*row], buffer, sizeof(char) * (len + 1));
        ++*row;
    }
    free(buffer);
    return TRUE;
}

static int load_u16same(void *bytes, int *index, int size, char **col, int *row)
{
    int success = TRUE;
    u16 num_same = 0;
    success = bytes_load(bytes, index, size, (void *)&num_same, U16);
    if (!success)
    {
        return FALSE;
    }
    u8 factor = 0;
    success = bytes_load(bytes, index, size, (void *)&factor, U8);
    u16 value = 0;
    success = bytes_load(bytes, index, size, (void *)&value, U16);
    int len;
    char *buffer = u64_to_string((u64)value, factor, &len);
    for (int i = 0; i < num_same; ++i)
    {
        col[*row] = mem_alloc(sizeof(char) * (len + 1));
        memcpy(col[*row], buffer, sizeof(char) * (len + 1));
        ++*row;
    }
    free(buffer);
    return TRUE;
}

static int load_u32same(void *bytes, int *index, int size, char **col, int *row)
{
    int success = TRUE;
    u16 num_same = 0;
    success = bytes_load(bytes, index, size, (void *)&num_same, U16);
    if (!success)
    {
        return FALSE;
    }
    u8 factor = 0;
    success = bytes_load(bytes, index, size, (void *)&factor, U8);
    u32 value = 0;
    success = bytes_load(bytes, index, size, (void *)&value, U32);
    int len;
    char *buffer = u64_to_string((u64)value, factor, &len);
    for (int i = 0; i < num_same; ++i)
    {
        col[*row] = mem_alloc(sizeof(char) * (len + 1));
        memcpy(col[*row], buffer, sizeof(char) * (len + 1));
        ++*row;
    }
    free(buffer);
    return TRUE;
}

static int load_u64same(void *bytes, int *index, int size, char **col, int *row)
{
    int success = TRUE;
    u16 num_same = 0;
    success = bytes_load(bytes, index, size, (void *)&num_same, U16);
    if (!success)
    {
        return FALSE;
    }
    u8 factor = 0;
    success = bytes_load(bytes, index, size, (void *)&factor, U8);
    u64 value = 0;
    success = bytes_load(bytes, index, size, (void *)&value, U64);
    int len;
    char *buffer = u64_to_string((u64)value, factor, &len);
    for (int i = 0; i < num_same; ++i)
    {
        col[*row] = mem_alloc(sizeof(char) * (len + 1));
        memcpy(col[*row], buffer, sizeof(char) * (len + 1));
        ++*row;
    }
    free(buffer);
    return TRUE;
}

int (*LOAD_FUNCS[])(void *, int *, int, char **, int *) = {
    load_strsame,
    load_strdiff,
    load_u8same,
    load_u16same,
    load_u32same,
    load_u64same};

#define NUM_LOAD_FUNCS (sizeof(LOAD_FUNCS) / sizeof(LOAD_FUNCS[0]))

static void bytes_parsec(void *bytes, int *index, int size, int num_cols, int num_rows, char ****cols)
{
    *cols = mem_alloc(sizeof(char **) * num_cols);
    for (int i = 0; i < num_cols; ++i)
    {
        (*cols)[i] = mem_alloc(sizeof(char *) * num_rows);
    }
    for (int col = 0; col < num_cols; ++col)
    {
        int row = 0;
        while (row < num_rows)
        {
            int success = TRUE;
            u8 indchar = 0;
            success = bytes_load(bytes, index, size, (void *)&indchar, U8);
            if (!success)
            {
                break;
            }
            LOAD_FUNCS[indchar](bytes, index, size, (*cols)[col], &row);
        }
    }
}

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
    char ***cols = NULL;
    bytes_parsec(bytes, &index, size, num_cols, num_rows, &cols);
    struct frame *df = mem_alloc(sizeof(struct frame));
    df->num_cols = num_cols;
    df->num_rows = num_rows;
    df->headers = headers;
    df->cols = cols;
    return df;
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
            *size += BYTES_INC_SIZE;
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
            *size += BYTES_INC_SIZE;
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
            *size += BYTES_INC_SIZE;
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
            *size += BYTES_INC_SIZE;
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
            *size += BYTES_INC_SIZE;
        }
        *((char *)(*bytes + *index)) = *(char *)data;
        *index += sizeof(char);
    }
    break;
    default:
        break;
    }
}

static void bytes_dump_string(char const *string, void **bytes, int *index, int *size)
{
    int length = strlen(string);
    for (int i = 0; i < length; ++i)
    {
        bytes_dump(bytes, index, size, (void *)&string[i], CHAR);
    }
    bytes_dump(bytes, index, size, (void *)&string[length], CHAR);
}

static void frame_bin_bytesh(struct frame *df, void **bytes, int *index, int *size)
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

static int string_same(char const *string1, char const *string2)
{
    int index = 0;
    while (string1[index] != '\0' && string2[index] != '\0')
    {
        if (string1[index] != string2[index])
        {
            return FALSE;
        }
        ++index;
    }
    if (string1[index] == string2[index])
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static int num_strsame(struct frame *df, int row, int col)
{
    int count = 1;
    while (row + count < df->num_rows)
    {
        if (string_same(df->cols[col][row], df->cols[col][row + count]))
        {
            ++count;
        }
        else
        {
            break;
        }
    }
    return count;
}

static float comp_strsame(struct frame *df, int row, int col)
{
    int num_same = num_strsame(df, row, col);
    if (num_same > UINT16_MAX)
    {
        num_same = UINT16_MAX;
    }
    int high_size = num_same * strlen(df->cols[col][row]);
    int low_size = sizeof(u16) + strlen(df->cols[col][row]);
    return (((float)low_size) / ((float)high_size));
}

static float comp_strdiff(struct frame *df, int row, int col)
{
    return 1;
}

static int isdigit(char c)
{
    if (c >= '0' && c <= '9')
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static int uint_possible(struct frame *df, int row, int col)
{
    int str_len = strlen(df->cols[col][row]);
    int possible = TRUE;
    int found_dec = FALSE;
    for (int i = 0; i < str_len; ++i)
    {
        if (!isdigit(df->cols[col][row][i]))
        {
            if (df->cols[col][row][i] == '.')
            {
                if (found_dec)
                {
                    possible = FALSE;
                    break;
                }
                else
                {
                    found_dec = TRUE;
                }
            }
            else
            {
                possible = FALSE;
                break;
            }
        }
    }
    return possible;
}

static int stou64_hasdec(char const *string)
{
    int len = strlen(string);
    for (int i = 0; i < len; ++i)
    {
        if (string[i] == '.')
        {
            return TRUE;
        }
    }
    return FALSE;
}

static int compute_u64_convfactor(char const *string, u8 *factor)
{
    *factor = 0;
    int len = strlen(string);
    for (int i = len - 1; i >= 0; --i)
    {
        if (string[i] == '.')
        {
            break;
        }
        else
        {
            if (*factor == UINT8_MAX)
            {
                return FALSE;
            }
            else
            {
                ++*factor;
            }
        }
    }
    return TRUE;
}

static int string_to_u64(char const *string, u64 *u_int, u8 *factor)
{
    int len = strlen(string);
    if (stou64_hasdec(string))
    {
        int success = compute_u64_convfactor(string, factor);
        if (!success)
        {
            return FALSE;
        }
    }
    else
    {
        *factor = 0;
    }
    *u_int = 0;
    u64 mul = 1;
    for (int i = len - 1; i >= 0; --i)
    {
        if (string[i] != '.')
        {
            if (*u_int > UINT64_MAX / 100)
            {
                return FALSE;
            }
            else
            {
                *u_int += (string[i] - '0') * mul;
                mul *= 10;
            }
        }
    }
    return TRUE;
}

static int u8_possible(struct frame *df, int row, int col)
{
    if (uint_possible(df, row, col))
    {
        u64 value;
        u8 factor;
        int success = string_to_u64(df->cols[col][row], &value, &factor);
        if (!success)
        {
            return FALSE;
        }
        else
        {
            if (value <= UINT8_MAX)
            {
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
    }
    else
    {
        return FALSE;
    }
}

static int u16_possible(struct frame *df, int row, int col)
{
    if (uint_possible(df, row, col))
    {
        u64 value;
        u8 factor;
        int success = string_to_u64(df->cols[col][row], &value, &factor);
        if (!success)
        {
            return FALSE;
        }
        else
        {
            if (value <= UINT16_MAX)
            {
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
    }
    else
    {
        return FALSE;
    }
}

static int u32_possible(struct frame *df, int row, int col)
{
    if (uint_possible(df, row, col))
    {
        u64 value;
        u8 factor;
        int success = string_to_u64(df->cols[col][row], &value, &factor);
        if (!success)
        {
            return FALSE;
        }
        else
        {
            if (value <= UINT32_MAX)
            {
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
    }
    else
    {
        return FALSE;
    }
}

static int u64_possible(struct frame *df, int row, int col)
{
    if (uint_possible(df, row, col))
    {
        u64 value;
        u8 factor;
        int success = string_to_u64(df->cols[col][row], &value, &factor);
        if (!success)
        {
            return FALSE;
        }
        else
        {
            return TRUE;
        }
    }
    else
    {
        return FALSE;
    }
}

static float comp_u8same(struct frame *df, int row, int col)
{
    if (u8_possible(df, row, col))
    {
        int num_same = num_strsame(df, row, col);
        if (num_same > UINT16_MAX)
        {
            num_same = UINT16_MAX;
        }
        int high_size = num_same * strlen(df->cols[col][row]);
        int low_size = sizeof(u16) + sizeof(u8) + sizeof(u8);
        return (((float)low_size) / ((float)high_size));
    }
    else
    {
        return FLT_MAX;
    }
}

static float comp_u8diff(struct frame *df, int row, int col) {}
static float comp_u8inc(struct frame *df, int row, int col) {}
static float comp_u8dec(struct frame *df, int row, int col) {}

static float comp_u16same(struct frame *df, int row, int col)
{
    if (u16_possible(df, row, col))
    {
        int num_same = num_strsame(df, row, col);
        if (num_same > UINT16_MAX)
        {
            num_same = UINT16_MAX;
        }
        int high_size = num_same * strlen(df->cols[col][row]);
        int low_size = sizeof(u16) + sizeof(u8) + sizeof(u16);
        return (((float)low_size) / ((float)high_size));
    }
    else
    {
        return FLT_MAX;
    }
}

static float comp_u16diff(struct frame *df, int row, int col) {}
static float comp_u16inc(struct frame *df, int row, int col) {}
static float comp_u16dec(struct frame *df, int row, int col) {}

static float comp_u32same(struct frame *df, int row, int col)
{
    if (u32_possible(df, row, col))
    {
        int num_same = num_strsame(df, row, col);
        if (num_same > UINT16_MAX)
        {
            num_same = UINT16_MAX;
        }
        int high_size = num_same * strlen(df->cols[col][row]);
        int low_size = sizeof(u16) + sizeof(u8) + sizeof(u32);
        return (((float)low_size) / ((float)high_size));
    }
    else
    {
        return FLT_MAX;
    }
}

static float comp_u32diff(struct frame *df, int row, int col) {}
static float comp_u32inc(struct frame *df, int row, int col) {}
static float comp_u32dec(struct frame *df, int row, int col) {}

static float comp_u64same(struct frame *df, int row, int col)
{
    if (u64_possible(df, row, col))
    {
        int num_same = num_strsame(df, row, col);
        if (num_same > UINT16_MAX)
        {
            num_same = UINT16_MAX;
        }
        int high_size = num_same * strlen(df->cols[col][row]);
        int low_size = sizeof(u16) + sizeof(u8) + sizeof(u64);
        return (((float)low_size) / ((float)high_size));
    }
    else
    {
        return FLT_MAX;
    }
}

static float comp_u64diff(struct frame *df, int row, int col) {}
static float comp_u64inc(struct frame *df, int row, int col) {}
static float comp_u64dec(struct frame *df, int row, int col) {}

static void dump_strsame(struct frame *df, void **bytes, int *index, int *size, int *row, int col)
{
    int num_same = num_strsame(df, *row, col);
    if (num_same > UINT16_MAX)
    {
        num_same = UINT16_MAX;
    }
    u8 indchr = (u8)STRING_SAMES;
    bytes_dump(bytes, index, size, (void *)&indchr, U8);
    u16 nsame = (u16)num_same;
    bytes_dump(bytes, index, size, (void *)&nsame, U16);
    bytes_dump_string(df->cols[col][*row], bytes, index, size);
    *row += num_same;
}

static void dump_strdiff(struct frame *df, void **bytes, int *index, int *size, int *row, int col)
{
    u8 indchr = (u8)STRING_DIFFS;
    bytes_dump(bytes, index, size, (void *)&indchr, U8);
    u16 num_diff;
    if (df->num_rows - *row > UINT16_MAX)
    {
        num_diff = UINT16_MAX;
    }
    else
    {
        num_diff = (u16)(df->num_rows - *row);
    }
    bytes_dump(bytes, index, size, (void *)&num_diff, U16);
    for (int i = 0; i < num_diff; ++i)
    {
        bytes_dump_string(df->cols[col][*row], bytes, index, size);
        ++*row;
    }
}

static void dump_u8same(struct frame *df, void **bytes, int *index, int *size, int *row, int col)
{
    int num_same = num_strsame(df, *row, col);
    if (num_same > UINT16_MAX)
    {
        num_same = UINT16_MAX;
    }
    u8 indchr = (u8)U8_SAMES;
    bytes_dump(bytes, index, size, (void *)&indchr, U8);
    u16 nsame = (u16)num_same;
    bytes_dump(bytes, index, size, (void *)&nsame, U16);
    u8 factor;
    u64 u_int;
    (void)string_to_u64(df->cols[col][*row], &u_int, &factor);
    bytes_dump(bytes, index, size, (void *)&factor, U8);
    u8 value = (u8)u_int;
    bytes_dump(bytes, index, size, (void *)&value, U8);
    *row += num_same;
}

static void dump_u8diff(struct frame *df, void **bytes, int *index, int *size, int *row, int col) {}
static void dump_u8inc(struct frame *df, void **bytes, int *index, int *size, int *row, int col) {}
static void dump_u8dec(struct frame *df, void **bytes, int *index, int *size, int *row, int col) {}

static void dump_u16same(struct frame *df, void **bytes, int *index, int *size, int *row, int col)
{
    int num_same = num_strsame(df, *row, col);
    if (num_same > UINT16_MAX)
    {
        num_same = UINT16_MAX;
    }
    u8 indchr = (u8)U16_SAMES;
    bytes_dump(bytes, index, size, (void *)&indchr, U8);
    u16 nsame = (u16)num_same;
    bytes_dump(bytes, index, size, (void *)&nsame, U16);
    u8 factor;
    u64 u_int;
    (void)string_to_u64(df->cols[col][*row], &u_int, &factor);
    bytes_dump(bytes, index, size, (void *)&factor, U8);
    u16 value = (u16)u_int;
    bytes_dump(bytes, index, size, (void *)&value, U16);
    *row += num_same;
}

static void dump_u16diff(struct frame *df, void **bytes, int *index, int *size, int *row, int col) {}
static void dump_u16inc(struct frame *df, void **bytes, int *index, int *size, int *row, int col) {}
static void dump_u16dec(struct frame *df, void **bytes, int *index, int *size, int *row, int col) {}

static void dump_u32same(struct frame *df, void **bytes, int *index, int *size, int *row, int col)
{
    int num_same = num_strsame(df, *row, col);
    if (num_same > UINT16_MAX)
    {
        num_same = UINT16_MAX;
    }
    u8 indchr = (u8)U32_SAMES;
    bytes_dump(bytes, index, size, (void *)&indchr, U8);
    u16 nsame = (u16)num_same;
    bytes_dump(bytes, index, size, (void *)&nsame, U16);
    u8 factor;
    u64 u_int;
    (void)string_to_u64(df->cols[col][*row], &u_int, &factor);
    bytes_dump(bytes, index, size, (void *)&factor, U8);
    u32 value = (u32)u_int;
    bytes_dump(bytes, index, size, (void *)&value, U32);
    *row += num_same;
}

static void dump_u32diff(struct frame *df, void **bytes, int *index, int *size, int *row, int col) {}
static void dump_u32inc(struct frame *df, void **bytes, int *index, int *size, int *row, int col) {}
static void dump_u32dec(struct frame *df, void **bytes, int *index, int *size, int *row, int col) {}

static void dump_u64same(struct frame *df, void **bytes, int *index, int *size, int *row, int col)
{
    int num_same = num_strsame(df, *row, col);
    if (num_same > UINT16_MAX)
    {
        num_same = UINT16_MAX;
    }
    u8 indchr = (u8)U64_SAMES;
    bytes_dump(bytes, index, size, (void *)&indchr, U8);
    u16 nsame = (u16)num_same;
    bytes_dump(bytes, index, size, (void *)&nsame, U16);
    u8 factor;
    u64 u_int;
    (void)string_to_u64(df->cols[col][*row], &u_int, &factor);
    bytes_dump(bytes, index, size, (void *)&factor, U8);
    u64 value = (u64)u_int;
    bytes_dump(bytes, index, size, (void *)&value, U64);
    *row += num_same;
}

static void dump_u64diff(struct frame *df, void **bytes, int *index, int *size, int *row, int col) {}
static void dump_u64inc(struct frame *df, void **bytes, int *index, int *size, int *row, int col) {}
static void dump_u64dec(struct frame *df, void **bytes, int *index, int *size, int *row, int col) {}

float (*COMP_FUNCS[])(struct frame *, int, int) = {
    comp_strsame,
    comp_strdiff,
    comp_u8same,
    comp_u8diff,
    comp_u8inc,
    comp_u8dec,
    comp_u16same,
    comp_u16diff,
    comp_u16inc,
    comp_u16dec,
    comp_u32same,
    comp_u32diff,
    comp_u32inc,
    comp_u32dec,
    comp_u64same,
    comp_u64diff,
    comp_u64inc,
    comp_u64dec};

void (*DUMP_FUNCS[])(struct frame *, void **, int *, int *, int *, int) = {
    dump_strsame,
    dump_strdiff,
    dump_u8same,
    dump_u8diff,
    dump_u8inc,
    dump_u8dec,
    dump_u16same,
    dump_u16diff,
    dump_u16inc,
    dump_u16dec,
    dump_u32same,
    dump_u32diff,
    dump_u32inc,
    dump_u32dec,
    dump_u64same,
    dump_u64diff,
    dump_u64inc,
    dump_u64dec};

#define NUM_CDFUNCS (sizeof(COMP_FUNCS) / sizeof(COMP_FUNCS[0]))

static void frame_bin_bytescl(struct frame *df, void **bytes, int *index, int *size, int col)
{
    int row = 0;
    while (row < df->num_rows)
    {
        float max_comp = comp_strsame(df, row, col);
        int max_index = 0;
        for (int i = 1; i < 3; ++i)
        {
            float comp = COMP_FUNCS[i](df, row, col);
            if (comp < max_comp)
            {
                max_comp = comp;
                max_index = i;
            }
        }
        DUMP_FUNCS[max_index](df, bytes, index, size, &row, col);
    }
}

static void frame_bin_bytesc(struct frame *df, void **bytes, int *index, int *size)
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
    frame_bin_bytesh(df, &bytes, index, size);
    frame_bin_bytesc(df, &bytes, index, size);
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