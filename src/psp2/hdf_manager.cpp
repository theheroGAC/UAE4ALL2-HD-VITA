#include "sysconfig.h"
#include "sysdeps.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef __PSP2__
#include <psp2/io/devctl.h>
#endif

#include "hdf_manager.h"
#include "hdf_io64.h"

#define HDF_DEFAULT_SECTORS 32
#define HDF_DEFAULT_RESERVED 2
#define HDF_DEFAULT_BLOCKSIZE 512
#define HDF_MAX_BLOCKSIZE 4096
#define HDF_MAX_SIZE_MB 8192ULL

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

static int hdf_read_block(hdf_fd f, unsigned long long block, int blocksize, unsigned char *buf)
{
    if (hdf_file_seek64(f, (long long)(block * (unsigned long long)blocksize)) != 0)
        return 0;
    return hdf_file_read(f, buf, blocksize) == blocksize;
}

static long long hdf_free_space_mb(const char *path)
{
#ifdef __PSP2__
    char dev[8];
    const char *colon;
    SceIoDevInfo info;
    size_t len;

    if (!path)
        return -1;
    colon = strchr(path, ':');
    if (!colon)
        return -1;
    len = (size_t)(colon - path);
    if (len == 0 || len >= sizeof(dev) - 1)
        return -1;
    memcpy(dev, path, len);
    dev[len] = ':';
    dev[len + 1] = '\0';

    memset(&info, 0, sizeof(info));
    if (sceIoDevctl(dev, 0x3001, NULL, 0, &info, sizeof(info)) < 0)
        return -1;
    return (long long)((unsigned long long)info.free_size / (1024ULL * 1024ULL));
#else
    (void)path;
    return -1;
#endif
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
    hdf_fd f;
    unsigned char hdr[HDF_MAX_BLOCKSIZE];
    unsigned long long size;
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

    f = hdf_open_readonly(clean);
    if (!hdf_is_open(f)) {
        hdf_set_error(info->error, sizeof(info->error), "Unable to open HDF file: %s", path);
        return 0;
    }

    {
        long long fsize = hdf_file_size64(f);
        if (fsize <= 0) {
            hdf_close(f);
            hdf_set_error(info->error, sizeof(info->error), "Invalid HDF size: the file is empty or unreadable");
            return 0;
        }
        size = (unsigned long long)fsize;
    }
    if (size < 512) {
        hdf_close(f);
        hdf_set_error(info->error, sizeof(info->error), "Invalid HDF size: too small (%llu bytes)", size);
        return 0;
    }
    if (size % HDF_DEFAULT_BLOCKSIZE != 0) {
        hdf_close(f);
        hdf_set_error(info->error, sizeof(info->error),
                      "Invalid HDF size: %llu bytes is not a multiple of %d", size, HDF_DEFAULT_BLOCKSIZE);
        return 0;
    }
    if (size > HDF_MAX_SIZE_MB * 1024ULL * 1024ULL) {
        hdf_close(f);
        hdf_set_error(info->error, sizeof(info->error),
                      "HDF too large: maximum supported size on PS Vita is %llu MB (%llu bytes)",
                      HDF_MAX_SIZE_MB, size);
        return 0;
    }

    info->size = size;
    info->total_blocks = size / HDF_DEFAULT_BLOCKSIZE;

    {
        hdf_fd rw = hdf_open_readwrite(clean);
        if (hdf_is_open(rw)) {
            hdf_close(rw);
            info->is_readonly = 0;
        } else {
            info->is_readonly = 1;
        }
    }

    if (!hdf_read_block(f, 0, HDF_DEFAULT_BLOCKSIZE, hdr)) {
        hdf_close(f);
        hdf_set_error(info->error, sizeof(info->error), "Read error while inspecting HDF");
        return 0;
    }

    if (hdr[0] == 'R' && hdr[1] == 'D' && hdr[2] == 'S' && hdr[3] == 'K') {
        info->is_rdb = 1;
        snprintf(info->filesystem, sizeof(info->filesystem), "RDB");
        info->dostype = 0;
        info->valid = 1;
        hdf_close(f);
        return 1;
    }

    if (info->total_blocks > 0) {
        unsigned long long mid = info->total_blocks / 2;
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

    if (info->size >= 1073741824ULL && info->size < 2147483648ULL)
        info->surfaces = 2;
    else if (info->size >= 2147483648ULL && info->size <= 4294967296ULL)
        info->surfaces = 4;
    else if (info->size > 4294967296ULL && info->size < 8589934592ULL)
        info->surfaces = 8;
    else if (info->size >= 8589934592ULL)
        info->surfaces = 16;
    info->cylinders = (int)((info->total_blocks / (unsigned long long)info->sectors_per_track) / (unsigned long long)info->surfaces);

    info->valid = 1;
    hdf_close(f);
    return 1;
}

int hdf_is_bootable(const char *path)
{
    hdf_fd f;
    unsigned char hdr[HDF_DEFAULT_BLOCKSIZE];
    char clean[512];
    int bootable = 0;

    if (!path || path[0] == '\0')
        return 0;

    hdf_strip_geometry_prefix(path, clean, sizeof(clean));
    f = hdf_open_readonly(clean);
    if (!hdf_is_open(f))
        return 0;

    if (hdf_read_block(f, 0, HDF_DEFAULT_BLOCKSIZE, hdr)) {
        if (hdr[0] == 'R' && hdr[1] == 'D' && hdr[2] == 'S' && hdr[3] == 'K')
            bootable = 1;
        else if (hdr[0] == 'D' && hdr[1] == 'O' && hdr[2] == 'S' && hdr[3] <= 5)
            bootable = 1;
    }

    hdf_close(f);
    return bootable;
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

    if (!path || path[0] == '\0' || megabytes == 0 || megabytes > (unsigned long)HDF_MAX_SIZE_MB) {
        hdf_set_error(err, errsz, "Invalid size or path (maximum %llu MB)", HDF_MAX_SIZE_MB);
        return -1;
    }

    if (stat(path, &st) == 0) {
        hdf_set_error(err, errsz, "File already exists: %s", path);
        return -2;
    }

    total = (unsigned long long)megabytes * 1024ULL * 1024ULL;
    {
        long long free_mb = hdf_free_space_mb(path);
        if (free_mb >= 0 && (unsigned long long)free_mb < (unsigned long long)megabytes) {
            hdf_set_error(err, errsz,
                          "Not enough free space: %lld MB available, %lu MB required", free_mb, megabytes);
            return -1;
        }
    }

    dst = fopen(path, "wb");
    if (dst == NULL) {
        hdf_set_error(err, errsz, "Unable to create HDF file (free space on ux0?)");
        return -1;
    }
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
