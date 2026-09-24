#ifndef HDF_IO64_H
#define HDF_IO64_H

#include <stdio.h>
#include <sys/types.h>

#ifdef __PSP2__
#include <psp2/io/fcntl.h>

typedef int hdf_fd;
#define HDF_FD_INVALID (-1)

static inline hdf_fd hdf_open_readwrite(const char *path)
{
    return sceIoOpen(path, SCE_O_RDWR, 0777);
}

static inline hdf_fd hdf_open_readonly(const char *path)
{
    return sceIoOpen(path, SCE_O_RDONLY, 0777);
}

static inline int hdf_is_open(hdf_fd fd)
{
    return fd >= 0;
}

static inline void hdf_close(hdf_fd fd)
{
    if (fd >= 0)
        sceIoClose(fd);
}

static inline long long hdf_file_size64(hdf_fd fd)
{
    SceOff pos = sceIoLseek(fd, 0, SCE_SEEK_END);
    if (pos < 0)
        return -1;
    return (long long)pos;
}

static inline int hdf_file_seek64(hdf_fd fd, long long offset)
{
    return sceIoLseek(fd, (SceOff)offset, SCE_SEEK_SET) < 0 ? -1 : 0;
}

static inline int hdf_file_read(hdf_fd fd, void *buf, int len)
{
    int done = 0;
    while (done < len) {
        int got = sceIoRead(fd, (char *)buf + done, (SceSize)(len - done));
        if (got <= 0)
            break;
        done += got;
    }
    return done;
}

static inline int hdf_file_write(hdf_fd fd, const void *buf, int len)
{
    int done = 0;
    while (done < len) {
        int put = sceIoWrite(fd, (const char *)buf + done, (SceSize)(len - done));
        if (put <= 0)
            break;
        done += put;
    }
    return done;
}

#else

typedef FILE *hdf_fd;
#define HDF_FD_INVALID NULL

static inline hdf_fd hdf_open_readwrite(const char *path)
{
    return fopen(path, "r+b");
}

static inline hdf_fd hdf_open_readonly(const char *path)
{
    return fopen(path, "rb");
}

static inline int hdf_is_open(hdf_fd fd)
{
    return fd != NULL;
}

static inline void hdf_close(hdf_fd fd)
{
    if (fd != NULL)
        fclose(fd);
}

static inline long long hdf_file_size64(hdf_fd fd)
{
#ifdef _WIN32
    if (_fseeki64(fd, 0, SEEK_END) != 0)
        return -1;
    return (long long)_ftelli64(fd);
#else
    if (fseeko(fd, 0, SEEK_END) != 0)
        return -1;
    return (long long)ftello(fd);
#endif
}

static inline int hdf_file_seek64(hdf_fd fd, long long offset)
{
#ifdef _WIN32
    return _fseeki64(fd, (__int64)offset, SEEK_SET) != 0 ? -1 : 0;
#else
    return fseeko(fd, (off_t)offset, SEEK_SET) != 0 ? -1 : 0;
#endif
}

static inline int hdf_file_read(hdf_fd fd, void *buf, int len)
{
    return (int)fread(buf, 1, (size_t)len, fd);
}

static inline int hdf_file_write(hdf_fd fd, const void *buf, int len)
{
    return (int)fwrite(buf, 1, (size_t)len, fd);
}

#endif

#endif
