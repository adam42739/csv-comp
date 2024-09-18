#include "frame-new.h"
#include <stdlib.h>

int main(int argc, char **argv)
{
    char const *out_path = argv[1];
    char const *buf_path = argv[2];
    char const *params = argv[3];
    struct frame *df = frame_read_csv(buf_path);
    return 0;
}
