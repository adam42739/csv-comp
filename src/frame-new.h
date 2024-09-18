#pragma once

enum csv_cdtype
{
    CSVCDT_INT,
    CSVCDT_FLOAT,
    CSVCDF_STRING
};

enum csv_cdotype
{
    CSVCDOT_PKTINT,
    CSVCDOT_MPLINT,
    CSVCDOT_STRING
};

struct frame
{
    int num_cols;
    int num_rows;
    char **headers;
    char ***cols;
};

struct frame *frame_read_csv(char const *path);

void frame_write_csv(struct frame *df, char const *path);
