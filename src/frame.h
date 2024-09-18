#pragma once

enum csv_cdtype
{
    CSVCDT_PKTINT,
    CSVCDT_MPLINT,
    CSVCDT_STRING
};

struct frame
{
    int num_cols;
    int num_rows;
    char **headers;
    char ***cols;
};

struct frame *frame_read_csv(char const *path);

int frame_write_csv(struct frame *df, char const *path);

void frame_free(struct frame *df);

struct frame *frame_read_bin(char const *path);

int frame_write_bin(struct frame *df, char const *path);
