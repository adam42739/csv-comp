#pragma once

enum csv_cdtype
{
    CSVCDT_INT,
    CSVCDT_FLOAT,
    CSVCDF_STRING
};

struct frame
{
    int num_cols;
    int num_rows;
    enum csv_cdtype *cdtypes;
    char **headers;
    void **cols;
};

struct frame *frame_read_csv(char const *path, enum csv_cdtype *cdtypes);

void frame_write_csv(struct frame *df, char const *path);
