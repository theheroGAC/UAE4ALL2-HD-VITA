#ifndef RDB_H
#define RDB_H

#define RDB_MAX_PARTITIONS 8

typedef struct {
    char name[32];
    unsigned int dostype;
    int bootable;
    int bootpri;
    int heads;
    int sectors_per_track;
    int reserved;
    int blocksize;
    int cylinders;
    unsigned long long start_block;
    unsigned long long total_blocks;
} RdbPartition;

int rdb_parse(const char *path, RdbPartition *parts, int max_parts);

#endif
