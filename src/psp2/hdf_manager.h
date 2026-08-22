/*
 * HDF Manager helper for UAE4ALL2-HD-VITA
 *
 * Self-contained helpers used by the Vita frontend to validate, inspect and
 * back up hard disk image files (.hdf) before/while they are mounted.
 *
 * It only reads the file header (boot block / RDB) to extract information;
 * it never loads the whole image into RAM. The actual mounting/unmounting is
 * still done by the existing UAE core (add_filesys_unit / kill_filesys_unit).
 */

#ifndef HDF_MANAGER_H
#define HDF_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

/* AmigaDOS disk types */
#define HDF_DOSTYPE_OFS 0x444f5300u /* DOS\0 */
#define HDF_DOSTYPE_FFS 0x444f5301u /* DOS\1 */

typedef struct {
    char path[512];            /* full path to the image file */
    unsigned long size;        /* size in bytes */
    unsigned long total_blocks;/* size / blocksize */
    int sectors_per_track;     /* detected / default geometry */
    int surfaces;
    int reserved;
    int blocksize;
    int cylinders;
    unsigned int dostype;      /* 0x444F53xx or 0 if unknown */
    int is_rdb;                /* starts with RDSK magic */
    int is_readonly;           /* file cannot be opened for writing */
    char filesystem[16];       /* "FFS", "OFS", "RDB", "Unknown" */
    int valid;                 /* 1 when the file looks like a usable HDF */
    char error[256];           /* reason when valid == 0 */
} HdfInfo;

/* Inspect an .hdf file and fill *info.
 * Returns 1 on success (info->valid set accordingly), 0 if the file could not
 * even be opened/read (info->error filled). */
int hdf_analyze(const char *path, HdfInfo *info);

/* Copy an .hdf file to dest_dir (created if missing).
 * Returns 0 on success, a negative error code on failure and fills err.
 * Refuses to overwrite an existing file. */
int hdf_backup(const char *path, const char *dest_dir, char *err, size_t errsz);

/* Create a blank (zero-filled) raw hard disk image of the given size.
 * Refuses to overwrite an existing file. Returns 0 on success, a negative
 * error code on failure and fills err. */
int hdf_create_blank(const char *path, unsigned long megabytes, char *err, size_t errsz);

#ifdef __cplusplus
}
#endif

#endif /* HDF_MANAGER_H */
