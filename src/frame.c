#include "frame.h"

typedef __int64_t u64;
typedef __int32_t u32;
typedef __int16_t u16;
typedef __int8_t u8;

struct frame frame_alloc(int num_rows, int num_cols)
{
    char **headers = mem_alloc(sizeof(char *) * num_cols);
    void **cols = mem_alloc(sizeof(void *) * num_rows);
    struct frame df = {
        .num_cols = num_cols,
        .num_rows = num_rows,
        .headers = headers,
        .cols = cols};
    return df;
}

void frame_free(struct frame df)
{
    free(df.headers);
    free(df.cols);
}

enum byte_dtype
{
    U8,
    U16,
    U32,
    U64,
    STRING,
    CSV
};

static void get_bytes(void *bytes, int *index, int size, void *data, enum byte_dtype dtype)
{
    switch (dtype)
    {
    case U8:
        if (size >= *index + sizeof(u8))
        {
            *(u8 *)data = *(u8 *)((u8 *)bytes + *index);
            *index += sizeof(u8);
        }
        else
        {
            *(u8 *)data = 0;
        }
        break;
    case U16:
        if (size >= *index + sizeof(u16))
        {
            *(u16 *)data = *(u16 *)((u8 *)bytes + *index);
            *index += sizeof(u16);
        }
        else
        {
            *(u16 *)data = 0;
        }
        break;
    case U32:
        if (size >= *index + sizeof(u32))
        {
            *(u32 *)data = *(u32 *)((u8 *)bytes + *index);
            *index += sizeof(u32);
        }
        else
        {
            *(u32 *)data = 0;
        }
        break;
    case U64:
        if (size >= *index + sizeof(u64))
        {
            *(u64 *)data = *(u64 *)((u8 *)bytes + *index);
            *index += sizeof(u64);
        }
        else
        {
            *(u64 *)data = 0;
        }
        break;
    case STRING:
    {
        int str_len = 0;
        while (size > *index + str_len)
        {
            if (*(char *)((u8 *)bytes + *index + str_len) == '\0')
            {
                break;
            }
            ++str_len;
        }
        char *string = mem_alloc(sizeof(char) * (str_len + 1));
        int str_index = 0;
        while (str_index < str_len)
        {
            string[str_index] = *(char *)((u8 *)bytes + *index);
            ++*index;
        }
        string[str_len] = '\0';
        ++*index;
        *(char **)data = string;
    }
    break;
    case CSV:
    {
        int str_len = 0;
        while (size > *index + str_len)
        {
            char nchar = *(char *)((u8 *)bytes + *index + str_len);
            if (nchar == ',' || nchar == '\n')
            {
                break;
            }
            ++str_len;
        }
        char *string = mem_alloc(sizeof(char) * (str_len + 1));
        int str_index = 0;
        while (str_index < str_len)
        {
            string[str_index] = *(char *)((u8 *)bytes + *index);
            ++str_index;
            ++*index;
        }
        string[str_len] = '\0';
        ++*index;
        *(char **)data = string;
    }
    break;
    default:
        break;
    }
}

static int count_comma(void *bytes, int size)
{
    int count = 0;
    int index = 0;
    while (index < size)
    {
        if (*((char *)bytes + index) == '\n')
        {
            break;
        }
        else if (*((char *)bytes + index) == ',')
        {
            ++count;
        }
        ++index;
    }
    return count;
}

static int count_bn(void *bytes, int size)
{
    int count = 0;
    int index = 0;
    while (index < size)
    {
        if (*((char *)bytes + index) == '\n')
        {
            ++count;
        }
        ++index;
    }
    return count;
}

static void get_headers_csv(struct frame *df, void *bytes, int *index, int size)
{
    for (int i = 0; i < df->num_cols; ++i)
    {
        get_bytes(bytes, index, size, &df->headers[i], CSV);
    }
}

static void get_line_csv(struct frame *df, void *bytes, int *index, int size)
{
    for (int i = 0; i < df->num_cols; ++i)
    {
        get_bytes(bytes, index, size, &df->headers[i], CSV);
    }
}

struct frame frame_read_csv(const char *path)
{
    void *bytes = NULL;
    int size = 0;
    fileio_read(path, &bytes, &size, FILEIO_READ_ASCII);
    int index = 0;
    int num_rows = 0;
    int num_cols = 0;
    num_rows = count_bn(bytes, size) - 2;
    num_cols = count_comma(bytes, size);
    struct frame df = frame_alloc(num_rows, num_cols);
    get_headers_csv(&df, bytes, &index, size);
    for (int i = 0; i < num_cols; ++i)
    {
        }
    return df;
}

struct frame frame_write_csv(const char *path)
{
}
