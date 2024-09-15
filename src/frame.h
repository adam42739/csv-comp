#pragma once

struct frame
{
    int num_cols;
    int num_rows;
    char **headers;
    char ***rows;
};

struct frame frame_alloc(int num_rows, int num_cols);

void frame_free(struct frame df);

struct frame frame_read_csv(char const *path);

void frame_write_csv(struct frame df, char const *path);
