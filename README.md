# csv-comp

## Introduction

csv-comp is an algorithm for compressing CSV files designed specifically for stock options data. CSV comp looks for patterns in the columns of a table to maximize file compression.

## Performance

For the 9 stock options located in [files](/files/), compression was performed using [Apache Parquet](https://en.wikipedia.org/wiki/Apache_Parquet), [Apache ORC](https://en.wikipedia.org/wiki/Apache_ORC), and [feather](https://github.com/wesm/feather). The compression ratio is computed by dividing the bytes of the compressed file by the bytes of the CSV file.

|CSV (baseline)|Parquet|ORC|feather|csv-comp|
|-|-|-|-|-|
|100%|57.8%|111.1%|60.5%|37.1%|

## Usage

### Compress a file

```shell
>>> ./csv_comp -c [compressed output path] [CSV path]
```

### Decompress a file

```shell
>>> ./csv_comp -dc [compressed path] [output CSV path]
```
