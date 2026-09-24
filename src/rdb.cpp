#include <stdio.h>
#include <string.h>

#include "hdf_io64.h"
#include "rdb.h"

#define RDB_BLOCK_SIZE 512
#define RDB_NO_BLOCK 0xFFFFFFFFu

#define RDB_OFF_BLOCK_BYTES 16
#define RDB_OFF_PART_LIST 28
#define RDB_OFF_CYLINDERS 64
#define RDB_OFF_SECTORS 68
#define RDB_OFF_HEADS 72
#define RDB_OFF_CYL_BLOCKS 144

#define PART_OFF_NEXT 16
#define PART_OFF_FLAGS 20
#define PART_OFF_DRIVE_NAME 36
#define PART_OFF_ENV 128

#define ENV_OFF_TABLESIZE 0
#define ENV_OFF_SIZEBLOCK 4
#define ENV_OFF_NUMHEADS 12
#define ENV_OFF_SECSPERBLK 16
#define ENV_OFF_BLKSPERTRACK 20
#define ENV_OFF_RESERVED 24
#define ENV_OFF_LOWCYL 36
#define ENV_OFF_HIGHCYL 40
#define ENV_OFF_BOOTPRI 60
#define ENV_OFF_DOSTYPE 64

#define PART_FLAG_BOOTABLE 1u

static unsigned int rdb_get_u32(const unsigned char *b, int off)
{
    return ((unsigned int)b[off] << 24) | ((unsigned int)b[off + 1] << 16) |
           ((unsigned int)b[off + 2] << 8) | (unsigned int)b[off + 3];
}

static int rdb_read_block(hdf_fd fd, unsigned long long block, unsigned char *buf)
{
    if (hdf_file_seek64(fd, (long long)(block * (unsigned long long)RDB_BLOCK_SIZE)) != 0)
        return 0;
    return hdf_file_read(fd, buf, RDB_BLOCK_SIZE) == RDB_BLOCK_SIZE;
}

int rdb_parse(const char *path, RdbPartition *parts, int max_parts)
{
    hdf_fd fd;
    unsigned char blk[RDB_BLOCK_SIZE];
    unsigned int heads, sectors, part_list;
    unsigned long long cyl_blocks, file_blocks;
    int count = 0;
    int guard = 0;

    if (!path || !parts || max_parts <= 0)
        return -1;

    fd = hdf_open_readonly(path);
    if (!hdf_is_open(fd))
        return -1;

    if (!rdb_read_block(fd, 0, blk) ||
        blk[0] != 'R' || blk[1] != 'D' || blk[2] != 'S' || blk[3] != 'K') {
        hdf_close(fd);
        return 0;
    }

    if (rdb_get_u32(blk, RDB_OFF_BLOCK_BYTES) != RDB_BLOCK_SIZE) {
        hdf_close(fd);
        return 0;
    }

    sectors = rdb_get_u32(blk, RDB_OFF_SECTORS);
    heads = rdb_get_u32(blk, RDB_OFF_HEADS);
    cyl_blocks = rdb_get_u32(blk, RDB_OFF_CYL_BLOCKS);
    if (cyl_blocks == 0)
        cyl_blocks = (unsigned long long)heads * (unsigned long long)sectors;
    if (cyl_blocks == 0) {
        hdf_close(fd);
        return 0;
    }

    file_blocks = (unsigned long long)hdf_file_size64(fd) / (unsigned long long)RDB_BLOCK_SIZE;
    part_list = rdb_get_u32(blk, RDB_OFF_PART_LIST);

    while (part_list != RDB_NO_BLOCK && count < max_parts && guard++ < 64) {
        unsigned int flags, next, env_size, size_block, secs_per_blk;
        unsigned int low_cyl, high_cyl, dostype;
        int heads_p, blks_per_trk, reserved, bootpri, cyls;
        unsigned long long total, start;

        if (!rdb_read_block(fd, part_list, blk))
            break;
        if (blk[0] != 'P' || blk[1] != 'A' || blk[2] != 'R' || blk[3] != 'T')
            break;

        next = rdb_get_u32(blk, PART_OFF_NEXT);
        flags = rdb_get_u32(blk, PART_OFF_FLAGS);

        env_size = rdb_get_u32(blk, PART_OFF_ENV + ENV_OFF_TABLESIZE);
        size_block = rdb_get_u32(blk, PART_OFF_ENV + ENV_OFF_SIZEBLOCK);
        heads_p = (int)rdb_get_u32(blk, PART_OFF_ENV + ENV_OFF_NUMHEADS);
        secs_per_blk = rdb_get_u32(blk, PART_OFF_ENV + ENV_OFF_SECSPERBLK);
        blks_per_trk = (int)rdb_get_u32(blk, PART_OFF_ENV + ENV_OFF_BLKSPERTRACK);
        reserved = (int)rdb_get_u32(blk, PART_OFF_ENV + ENV_OFF_RESERVED);
        low_cyl = rdb_get_u32(blk, PART_OFF_ENV + ENV_OFF_LOWCYL);
        high_cyl = rdb_get_u32(blk, PART_OFF_ENV + ENV_OFF_HIGHCYL);
        bootpri = (int)rdb_get_u32(blk, PART_OFF_ENV + ENV_OFF_BOOTPRI);
        dostype = (env_size >= 17) ? rdb_get_u32(blk, PART_OFF_ENV + ENV_OFF_DOSTYPE) : 0;

        if (secs_per_blk == 0)
            secs_per_blk = 1;

        if (env_size >= 1 && high_cyl >= low_cyl &&
            size_block * 4u * secs_per_blk == RDB_BLOCK_SIZE) {
            cyls = (int)(high_cyl - low_cyl) + 1;
            start = (unsigned long long)low_cyl * cyl_blocks;
            total = (unsigned long long)cyls * cyl_blocks;
            if (start >= file_blocks)
                total = 0;
            else if (start + total > file_blocks)
                total = file_blocks - start;

            if (total > 0) {
                RdbPartition *p = &parts[count];
                int i, len = blk[PART_OFF_DRIVE_NAME];

                memset(p, 0, sizeof(*p));
                if (len > 31)
                    len = 31;
                for (i = 0; i < len; i++)
                    p->name[i] = (char)blk[PART_OFF_DRIVE_NAME + 1 + i];
                p->name[len] = '\0';
                p->dostype = dostype;
                p->bootable = (flags & PART_FLAG_BOOTABLE) ? 1 : 0;
                p->bootpri = (flags & PART_FLAG_BOOTABLE) ? bootpri : -1;
                p->heads = heads_p > 0 ? heads_p : (int)heads;
                p->sectors_per_track = blks_per_trk > 0 ? blks_per_trk : (int)sectors;
                p->reserved = reserved > 0 ? reserved : 0;
                p->blocksize = RDB_BLOCK_SIZE;
                p->cylinders = cyls;
                p->start_block = start;
                p->total_blocks = total;
                count++;
            }
        }

        part_list = next;
    }

    hdf_close(fd);
    return count;
}
