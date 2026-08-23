#include "sysconfig.h"
#include "sysdeps.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "hdf_manager.h"

#define HDF_DEFAULT_SECTORS 32
#define HDF_DEFAULT_RESERVED 2
#define HDF_DEFAULT_BLOCKSIZE 512
#define HDF_MAX_BLOCKSIZE 4096

static void hdf_set_error(char *err, size_t errsz, const char *fmt, ...)
{
    if (!err || errsz == 0)
        return;

    err[0] = '\0';

    char tmp[256];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(tmp, sizeof(tmp), fmt, ap);
    va_end(ap);

    snprintf(err, errsz, "%s", tmp);
}

static int hdf_read_block(FILE *f, unsigned long block, int blocksize, unsigned char *buf)
{
    if (fseek(f, (long)((unsigned long long)block * (unsigned long)blocksize), SEEK_SET) != 0)
        return 0;
    return (int)fread(buf, 1, (size_t)blocksize, f) == blocksize;
}

static unsigned int hdf_get_u32(const unsigned char *b)
{
    return ((unsigned int)b[0] << 24) | ((unsigned int)b[1] << 16) |
           ((unsigned int)b[2] << 8) | (unsigned int)b[3];
}

static void hdf_strip_geometry_prefix(const char *src, char *dst, size_t dstsz)
{
    const char *p = src;
    int colons = 0;

    if (src) {
        for (int i = 0; src[i] != '\0' && colons < 4; i++) {
            if (src[i] == ':') {
                colons++;
                p = &src[i + 1];
            }
        }
    }
    snprintf(dst, dstsz, "%s", p ? p : (src ? src : ""));
}

int hdf_analyze(const char *path, HdfInfo *info)
{
    FILE *f;
    unsigned char hdr[HDF_MAX_BLOCKSIZE];
    unsigned long size;
    char clean[512];

    if (!path || !info)
        return 0;

    memset(info, 0, sizeof(*info));
    hdf_strip_geometry_prefix(path, clean, sizeof(clean));
    snprintf(info->path, sizeof(info->path), "%s", clean);
    info->blocksize = HDF_DEFAULT_BLOCKSIZE;
    info->sectors_per_track = HDF_DEFAULT_SECTORS;
    info->reserved = HDF_DEFAULT_RESERVED;
    info->surfaces = 1;

    f = fopen(clean, "rb");
    if (f == NULL) {
        hdf_set_error(info->error, sizeof(info->error), "Unable to open HDF file: %s", path);
        return 0;
    }

    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        hdf_set_error(info->error, sizeof(info->error), "Unable to read HDF file: %s", path);
        return 0;
    }
    size = (unsigned long)ftell(f);
    if (size == 0) {
        fclose(f);
        hdf_set_error(info->error, sizeof(info->error), "Invalid HDF size: the file is empty");
        return 0;
    }
    if (size < 512) {
        fclose(f);
        hdf_set_error(info->error, sizeof(info->error), "Invalid HDF size: too small (%lu bytes)", size);
        return 0;
    }
    if (size % HDF_DEFAULT_BLOCKSIZE != 0) {
        fclose(f);
        hdf_set_error(info->error, sizeof(info->error),
                      "Invalid HDF size: %lu bytes is not a multiple of %d", size, HDF_DEFAULT_BLOCKSIZE);
        return 0;
    }
    if (size >= 0x80000000UL) {
        fclose(f);
        hdf_set_error(info->error, sizeof(info->error),
                      "HDF too large: maximum supported size on PS Vita is 2 GB (%lu bytes)", size);
        return 0;
    }

    info->size = size;
    info->total_blocks = size / HDF_DEFAULT_BLOCKSIZE;

    {
        FILE *rw = fopen(path, "r+b");
        if (rw != NULL) {
            fclose(rw);
            info->is_readonly = 0;
        } else {
            info->is_readonly = 1;
        }
    }

    if (!hdf_read_block(f, 0, HDF_DEFAULT_BLOCKSIZE, hdr)) {
        fclose(f);
        hdf_set_error(info->error, sizeof(info->error), "Read error while inspecting HDF");
        return 0;
    }

    if (hdr[0] == 'R' && hdr[1] == 'D' && hdr[2] == 'S' && hdr[3] == 'K') {
        info->is_rdb = 1;
        snprintf(info->filesystem, sizeof(info->filesystem), "RDB");
        info->dostype = 0;
        info->valid = 1;
        fclose(f);
        return 1;
    }

    if (info->total_blocks > 0) {
        unsigned long mid = info->total_blocks / 2;
        unsigned char root_buf[HDF_DEFAULT_BLOCKSIZE];
        unsigned int t, st;

        if (hdf_read_block(f, mid, HDF_DEFAULT_BLOCKSIZE, root_buf)) {
            t = hdf_get_u32(root_buf);
            st = hdf_get_u32(root_buf + 508);
            if (t == 2 && st == 1) {
                info->reserved = 0;
            } else if (mid + 1 < info->total_blocks &&
                       hdf_read_block(f, mid + 1, HDF_DEFAULT_BLOCKSIZE, root_buf)) {
                t = hdf_get_u32(root_buf);
                st = hdf_get_u32(root_buf + 508);
                if (t == 2 && st == 1)
                    info->reserved = 2;
            }
        }
    }

    {
        unsigned long boot_off = (unsigned long)info->reserved * HDF_DEFAULT_BLOCKSIZE;
        unsigned char boot[HDF_DEFAULT_BLOCKSIZE];
        if (hdf_read_block(f, info->reserved, HDF_DEFAULT_BLOCKSIZE, boot)) {
            if (boot[0] == 'D' && boot[1] == 'O' && boot[2] == 'S' && boot[3] <= 5) {
                info->dostype = hdf_get_u32(boot);
            }
        }
    }

    if (info->dostype == HDF_DOSTYPE_FFS)
        snprintf(info->filesystem, sizeof(info->filesystem), "FFS");
    else if (info->dostype == HDF_DOSTYPE_OFS)
        snprintf(info->filesystem, sizeof(info->filesystem), "OFS");
    else if (info->dostype != 0)
        snprintf(info->filesystem, sizeof(info->filesystem), "DOS\\%d", (int)(info->dostype & 0xFF));
    else
        snprintf(info->filesystem, sizeof(info->filesystem), "Unknown");

    if (info->size >= 1073741824UL && info->size < 2147483648UL)
        info->surfaces = 2;
    info->cylinders = (int)((info->total_blocks / info->sectors_per_track) / info->surfaces);

    info->valid = 1;
    fclose(f);
    return 1;
}

int hdf_backup(const char *path, const char *dest_dir, char *err, size_t errsz)
{
    FILE *src, *dst;
    struct stat st;
    char dest[512];
    char clean[512];
    unsigned char buf[262144];
    size_t n;
    const char *name;

    if (!path || !dest_dir || path[0] == '\0') {
        hdf_set_error(err, errsz, "Invalid HDF path");
        return -1;
    }

    hdf_strip_geometry_prefix(path, clean, sizeof(clean));

    if (stat(clean, &st) != 0 || !S_ISREG(st.st_mode)) {
        hdf_set_error(err, errsz, "Unable to open HDF: %s", clean);
        return -1;
    }

    if (mkdir(dest_dir, 0777) != 0 && errno != EEXIST) {
        hdf_set_error(err, errsz, "Unable to create backup directory: %s", dest_dir);
        return -1;
    }

    name = strrchr(clean, '/');
    if (!name)
        name = strrchr(clean, '\\');
    name = name ? name + 1 : clean;

    snprintf(dest, sizeof(dest), "%s/%s", dest_dir, name);

    if (stat(dest, &st) == 0) {
        hdf_set_error(err, errsz, "Backup already exists: %s", dest);
        return -2;
    }

    src = fopen(clean, "rb");
    if (src == NULL) {
        hdf_set_error(err, errsz, "Unable to open HDF for backup: %s", clean);
        return -1;
    }
    dst = fopen(dest, "wb");
    if (dst == NULL) {
        fclose(src);
        hdf_set_error(err, errsz, "Unable to create backup file: %s", dest);
        return -1;
    }

    while ((n = fread(buf, 1, sizeof(buf), src)) > 0) {
        if (fwrite(buf, 1, n, dst) != n) {
            fclose(src);
            fclose(dst);
            remove(dest);
            hdf_set_error(err, errsz, "Write error during backup (not enough free space?)");
            return -1;
        }
    }

    if (ferror(src)) {
        fclose(src);
        fclose(dst);
        remove(dest);
        hdf_set_error(err, errsz, "Read error during backup");
        return -1;
    }

    fclose(src);
    if (fflush(dst) != 0 || fclose(dst) != 0) {
        remove(dest);
        hdf_set_error(err, errsz, "Write error during backup");
        return -1;
    }

    return 0;
}

extern "C" void vita_gui_draw_progress(const char *title, const char *subtitle, float fraction, const char *item_name);

int hdf_create_blank(const char *path, unsigned long megabytes, char *err, size_t errsz)
{
    struct stat st;
    FILE *dst;
    unsigned char *buf;
    unsigned long long total;
    unsigned long long written = 0;
    int last_percent = -1;

    if (!path || path[0] == '\0' || megabytes == 0 || megabytes > 65536) {
        hdf_set_error(err, errsz, "Invalid size or path");
        return -1;
    }

    if (stat(path, &st) == 0) {
        hdf_set_error(err, errsz, "File already exists: %s", path);
        return -2;
    }

    dst = fopen(path, "wb");
    if (dst == NULL) {
        hdf_set_error(err, errsz, "Unable to create HDF file (free space on ux0?)");
        return -1;
    }

    total = (unsigned long long)megabytes * 1024ULL * 1024ULL;
    buf = (unsigned char *)malloc(1024 * 1024);
    if (!buf) {
        fclose(dst);
        remove(path);
        hdf_set_error(err, errsz, "Out of memory");
        return -1;
    }
    memset(buf, 0, 1024 * 1024);

    while (written < total) {
        size_t chunk = 1024 * 1024;
        if (total - written < chunk)
            chunk = (size_t)(total - written);
        if (fwrite(buf, 1, chunk, dst) != chunk) {
            free(buf);
            fclose(dst);
            remove(path);
            hdf_set_error(err, errsz, "Write error (not enough free space on ux0?)");
            return -1;
        }
        written += chunk;
        {
            int percent = (int)(written * 100 / total);
            if (percent != last_percent) {
                last_percent = percent;
                {
                    char sub[64];
                    snprintf(sub, sizeof(sub), "%llu MB / %lu MB (%d%%)",
                             (unsigned long long)(written / (1024ULL * 1024ULL)),
                             megabytes, percent);
                    vita_gui_draw_progress("Creating Blank HDF", sub,
                        (float)percent / 100.0f, path);
                }
            }
        }
    }

    free(buf);
    if (fflush(dst) != 0 || fclose(dst) != 0) {
        remove(path);
        hdf_set_error(err, errsz, "Write error during final flush");
        return -1;
    }

    return 0;
}
