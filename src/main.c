#include "frame.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

enum process_option
{
    COMP,
    DECOMP,
    INVALID
};

static enum process_option get_option(char const *string)
{
    int str_len = strlen(string);
    if (str_len == 2)
    {
        if (string[0] == '-' && string[1] == 'c')
        {
            return COMP;
        }
    }
    else if (str_len == 3)
    {
        if (string[0] == '-' && string[1] == 'd' && string[2] == 'c')
        {
            return DECOMP;
        }
    }
    return INVALID;
}

int main(int argc, char **argv)
{
    char const *str_option = argv[1];
    enum process_option option = get_option(str_option);
    char const *out_path = argv[2];
    char const *buf_path = argv[3];
    switch (option)
    {
    case COMP:
    {
        struct frame *df = frame_read_csv(buf_path);
        frame_write_bin(df, out_path);
        frame_free(df);
    }
    break;
    case DECOMP:
    {
        struct frame *df = frame_read_bin(out_path);
        frame_write_csv(df, buf_path);
        frame_free(df);
    }
    break;
    case INVALID:
    {
        printf("Invalid option given\n");
    }
    break;
    default:
        break;
    }
    return 0;
}
