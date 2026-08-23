#ifndef HDF_MANAGER_H
#define HDF_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#define HDF_DOSTYPE_OFS 0x444f5300u
#define HDF_DOSTYPE_FFS 0x444f5301u

typedef struct {
    char path[512];
    unsigned long size;
    unsigned long total_blocks;
    int sectors_per_track;
    int surfaces;
    int reserved;
    int blocksize;
    int cylinders;
    unsigned int dostype;
    int is_rdb;           
    int is_readonly;      
    char filesystem[16];  
    int valid;            
    char error[256];      
} HdfInfo;

int hdf_analyze(const char *path, HdfInfo *info);

int hdf_backup(const char *path, const char *dest_dir, char *err, size_t errsz);

int hdf_create_blank(const char *path, unsigned long megabytes, char *err, size_t errsz);

#ifdef __cplusplus
}
#endif

#endif
