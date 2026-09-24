 /*
  * UAE - The Un*x Amiga Emulator
  *
  * Unix file system handler for AmigaDOS
  *
  * Copyright 1997 Bernd Schmidt
  */

#include "hdf_io64.h"

#define FILESYS_VIRTUAL 0
#define FILESYS_HARDFILE 1
#define FILESYS_HARDDRIVE 3

struct hardfiledata {
    unsigned long long size;
    unsigned long long offset;
    int bootpri;
    int nrcyls;
    int secspertrack;
    int surfaces;
    int reservedblocks;
    int blocksize;
    uae_u32 dostype;
    hdf_fd fd;
};
#ifdef WIN32
int truncate (const char *name, long int len);
#endif

struct uaedev_mount_info;

extern struct hardfiledata *get_hardfile_data (int nr);
