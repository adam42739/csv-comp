#include "misc.h"

struct frame
{
    int num_cols;
    int num_rows;
    char **headers;
    void **cols;
};

struct frame frame_alloc(int num_rows, int num_cols);

void frame_free(struct frame df);
