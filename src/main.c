#include "frame.h"
#include <stdlib.h>

int main(int argc, char **argv)
{
    char const *out_path = argv[1];
    char const *buf_path = argv[2];
    char const *params = argv[3];
    struct frame *df = frame_read_csv(buf_path);
    frame_write_bin(df, out_path);
    frame_free(df);
    df = frame_read_bin(out_path);
    frame_write_csv(df, out_path);
    frame_free(df);
    return 0;
}
