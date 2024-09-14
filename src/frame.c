#include "frame.h"

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