#ifdef __PSP2__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <sys/stat.h>
#include <psp2/io/dirent.h>
#include <psp2/io/stat.h>
#include <psp2/io/fcntl.h>
#include <archive.h>
#include <archive_entry.h>

#include "whdload_manager.h"

static int path_exists(const char *path, int *is_dir)
{
    SceIoStat st;
    if (sceIoGetstat(path, &st) < 0)
        return 0;
    if (is_dir)
        *is_dir = SCE_S_ISDIR(st.st_mode) ? 1 : 0;
    return 1;
}

static void ensure_directory(const char *path)
{
    char buffer[512];
    size_t len;
    strncpy(buffer, path, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    len = strlen(buffer);
    while (len > 0 && (buffer[len - 1] == '/' || buffer[len - 1] == '\\')) {
        buffer[len - 1] = '\0';
        len--;
    }
    for (char *p = buffer; *p; p++) {
        if (*p == '\\') *p = '/';
    }
    for (char *p = buffer; *p; p++) {
        if (*p == '/' && p != buffer + 3 && p != buffer + 4) {
            *p = '\0';
            sceIoMkdir(buffer, 0777);
            *p = '/';
        }
    }
    sceIoMkdir(buffer, 0777);
}

static int make_safe_relative(const char *source, char *destination, size_t destination_size)
{
    if (!source || !destination || destination_size == 0 || strchr(source, ':'))
        return 0;

    char temp[512];
    strncpy(temp, source, sizeof(temp) - 1);
    temp[sizeof(temp) - 1] = '\0';
    for (char *p = temp; *p; p++) {
        if (*p == '\\') *p = '/';
    }

    size_t out = 0;
    const char *part = temp;
    while (*part == '/') part++;

    while (*part) {
        while (*part == '/') part++;
        if (!*part) break;
        const char *end = strchr(part, '/');
        size_t length = end ? (size_t)(end - part) : strlen(part);
        if (length == 0 || (length == 1 && part[0] == '.')) {
            part = end ? end + 1 : part + length;
            continue;
        }
        if (length == 2 && part[0] == '.' && part[1] == '.')
            return 0;
        if (out != 0) {
            if (out + 1 >= destination_size) return 0;
            destination[out++] = '/';
        }
        if (out + length >= destination_size) return 0;
        memcpy(destination + out, part, length);
        out += length;
        part = end ? end + 1 : part + length;
    }
    if (out == 0) return 0;
    destination[out] = '\0';
    return 1;
}

static void make_folder_name(const char *archive_path, char *folder, size_t folder_size)
{
    const char *name = strrchr(archive_path, '/');
    name = name ? name + 1 : archive_path;
    strncpy(folder, name, folder_size - 1);
    folder[folder_size - 1] = '\0';
    char *dot = strrchr(folder, '.');
    if (dot) *dot = '\0';
    for (size_t i = 0; folder[i]; i++) {
        unsigned char c = (unsigned char)folder[i];
        if (!isalnum(c) && c != '_' && c != '-' && c != ' ' && c != '.')
            folder[i] = '_';
    }
    if (folder[0] == '\0')
        strncpy(folder, "game", folder_size - 1);
    folder[folder_size - 1] = '\0';
}

static void make_unique_folder(const char *base, char *folder, size_t folder_size)
{
    char candidate[512];
    int number = 1;
    strncpy(folder, base, folder_size - 1);
    folder[folder_size - 1] = '\0';
    snprintf(candidate, sizeof(candidate), "%s/%s", VITA_WHDLOAD_ROOT, folder);
    int is_dir = 0;
    while (path_exists(candidate, &is_dir)) {
        number++;
        snprintf(folder, folder_size, "%s_%d", base, number);
        snprintf(candidate, sizeof(candidate), "%s/%s", VITA_WHDLOAD_ROOT, folder);
    }
}

#define LHA_DICSIZ (1 << 13)
#define LHA_THRESHOLD 3
#define LHA_NC 510
#define LHA_NP 14
#define LHA_NT 19
#define LHA_PBIT 4
#define LHA_TBIT 5
#define LHA_NPT (LHA_NT > LHA_NP ? LHA_NT : LHA_NP)

#define LHA_IN_BUF_SIZE (64 * 1024)

typedef struct {
    SceUID in_fd;
    SceUID out_fd;
    uint32_t comp_size;
    uint32_t orig_size;
    uint64_t bitbuf;
    int bitcount;
    uint8_t *dtext;
    int dpos;

    uint8_t in_buf[LHA_IN_BUF_SIZE];
    SceOff in_pos;
    SceOff in_len;

    uint16_t left[2 * LHA_NC + 1];
    uint16_t right[2 * LHA_NC + 1];
    uint8_t pt_len[LHA_NPT];
    uint16_t pt_table[256];
    uint8_t c_len[LHA_NC];
    uint16_t c_table[4096];
    uint16_t blocksize;
} LhaNativeDecoder;

static int lha_read1(LhaNativeDecoder *dec)
{
    if (dec->comp_size == 0)
        return -1;
    if (dec->in_pos >= dec->in_len) {
        int want = (dec->comp_size < LHA_IN_BUF_SIZE) ? (int)dec->comp_size : LHA_IN_BUF_SIZE;
        int got = (int)sceIoRead(dec->in_fd, dec->in_buf, want);
        if (got <= 0)
            return -1;
        dec->in_pos = 0;
        dec->in_len = got;
    }
    int b = dec->in_buf[dec->in_pos++];
    dec->comp_size--;
    return b;
}

static void lha_fillbuf(LhaNativeDecoder *dec, int n)
{
    while (dec->bitcount < n) {
        int c = lha_read1(dec);
        dec->bitbuf = (dec->bitbuf << 8) | (uint64_t)(c >= 0 ? (c & 0xff) : 0);
        dec->bitcount += 8;
    }
}

static uint32_t lha_getbits(LhaNativeDecoder *dec, int n)
{
    lha_fillbuf(dec, n);
    uint32_t x = (uint32_t)((dec->bitbuf >> (dec->bitcount - n)) & ((1U << n) - 1));
    dec->bitcount -= n;
    return x;
}

static uint32_t lha_peekbits(LhaNativeDecoder *dec, int n)
{
    lha_fillbuf(dec, n);
    return (uint32_t)((dec->bitbuf >> (dec->bitcount - n)) & ((1U << n) - 1));
}

static void lha_dropbits(LhaNativeDecoder *dec, int n)
{
    dec->bitcount -= n;
}

static void lha_make_table(int nchar, uint8_t *bitlen, int tablebits, uint16_t *table, uint16_t *left, uint16_t *right)
{
    uint16_t count[17] = {0};
    uint16_t start[17] = {0};
    int i;
    for (i = 0; i < nchar; i++) {
        if (bitlen[i] <= 16) count[bitlen[i]]++;
    }
    for (i = 1; i <= 16; i++) {
        start[i] = start[i - 1] + (count[i - 1] << (16 - (i - 1)));
    }
    int tablesize = 1 << tablebits;
    for (i = 0; i < tablesize; i++) table[i] = 0;
    int avail = nchar;
    for (i = 0; i < nchar; i++) {
        int len = bitlen[i];
        if (len == 0) continue;
        uint16_t k = start[len];
        start[len] += 1 << (16 - len);
        if (len <= tablebits) {
            int step = 1 << (tablebits - len);
            int idx = k >> (16 - tablebits);
            for (int j = 0; j < step; j++) table[idx + j] = i;
        } else {
            int idx = k >> (16 - tablebits);
            uint16_t *p = &table[idx];
            for (int j = tablebits; j < len; j++) {
                if (*p == 0) { left[avail] = right[avail] = 0; *p = avail++; }
                if (k & (1 << (15 - j))) p = &right[*p];
                else p = &left[*p];
            }
            *p = i;
        }
    }
}

static void lha_read_pt_len(LhaNativeDecoder *dec, int nn, int nbit, int i_special)
{
    int n = lha_getbits(dec, nbit);
    if (n == 0) {
        int c = lha_getbits(dec, nbit);
        for (int i = 0; i < nn; i++) dec->pt_len[i] = 0;
        for (int i = 0; i < 256; i++) dec->pt_table[i] = c;
    } else {
        int i = 0;
        while (i < n) {
            int c = lha_peekbits(dec, 3);
            if (c == 7) {
                lha_fillbuf(dec, 20);
                uint64_t mask = (uint64_t)1 << (dec->bitcount - 4);
                while (mask & dec->bitbuf) { mask >>= 1; c++; }
            }
            lha_dropbits(dec, (c < 7) ? 3 : c - 3);
            dec->pt_len[i++] = c;
            if (i == i_special) {
                int c2 = lha_getbits(dec, 2);
                while (--c2 >= 0) dec->pt_len[i++] = 0;
            }
        }
        while (i < nn) dec->pt_len[i++] = 0;
        lha_make_table(nn, dec->pt_len, 8, dec->pt_table, dec->left, dec->right);
    }
}

static void lha_read_c_len(LhaNativeDecoder *dec)
{
    int n = lha_getbits(dec, 9);
    if (n == 0) {
        int c = lha_getbits(dec, 9);
        for (int i = 0; i < LHA_NC; i++) dec->c_len[i] = 0;
        for (int i = 0; i < 4096; i++) dec->c_table[i] = c;
    } else {
        int i = 0;
        while (i < n) {
            int c = dec->pt_table[lha_peekbits(dec, 8)];
            if (c >= LHA_NT) {
                lha_fillbuf(dec, 20);
                uint64_t mask = (uint64_t)1 << (dec->bitcount - 9);
                do {
                    if (dec->bitbuf & mask) c = dec->right[c];
                    else c = dec->left[c];
                    mask >>= 1;
                } while (c >= LHA_NT);
            }
            lha_dropbits(dec, dec->pt_len[c]);
            if (c <= 2) {
                if (c == 0) c = 1;
                else if (c == 1) c = lha_getbits(dec, 4) + 3;
                else c = lha_getbits(dec, 9) + 20;
                while (--c >= 0) dec->c_len[i++] = 0;
            } else {
                dec->c_len[i++] = c - 2;
            }
        }
        while (i < LHA_NC) dec->c_len[i++] = 0;
        lha_make_table(LHA_NC, dec->c_len, 12, dec->c_table, dec->left, dec->right);
    }
}

static int lha_decode_c(LhaNativeDecoder *dec)
{
    if (dec->blocksize == 0) {
        dec->blocksize = lha_getbits(dec, 16);
        lha_read_pt_len(dec, LHA_NT, LHA_TBIT, 3);
        lha_read_c_len(dec);
        lha_read_pt_len(dec, LHA_NP, LHA_PBIT, -1);
    }
    dec->blocksize--;
    int j = dec->c_table[lha_peekbits(dec, 12)];
    if (j >= LHA_NC) {
        lha_fillbuf(dec, 24);
        uint64_t mask = (uint64_t)1 << (dec->bitcount - 13);
        do {
            if (dec->bitbuf & mask) j = dec->right[j];
            else j = dec->left[j];
            mask >>= 1;
        } while (j >= LHA_NC);
    }
    lha_dropbits(dec, dec->c_len[j]);
    return j;
}

static int lha_decode_p(LhaNativeDecoder *dec)
{
    int j = dec->pt_table[lha_peekbits(dec, 8)];
    if (j >= LHA_NP) {
        lha_fillbuf(dec, 20);
        uint64_t mask = (uint64_t)1 << (dec->bitcount - 9);
        do {
            if (dec->bitbuf & mask) j = dec->right[j];
            else j = dec->left[j];
            mask >>= 1;
        } while (j >= LHA_NP);
    }
    lha_dropbits(dec, dec->pt_len[j]);
    if (j != 0) j = (1 << (j - 1)) + lha_getbits(dec, j - 1);
    return j;
}

static int lha_flush_output(SceUID out_fd, char *buffer, int *buffer_pos)
{
    if (*buffer_pos <= 0)
        return 1;
    int written = sceIoWrite(out_fd, buffer, *buffer_pos);
    if (written != *buffer_pos)
        return 0;
    *buffer_pos = 0;
    return 1;
}

static int lha_decode_file(SceUID in_fd, SceUID out_fd, uint32_t comp_size, uint32_t orig_size)
{
    LhaNativeDecoder *dec = (LhaNativeDecoder *)calloc(1, sizeof(LhaNativeDecoder));
    if (!dec) return 0;
    dec->in_fd = in_fd;
    dec->out_fd = out_fd;
    dec->comp_size = comp_size;
    dec->orig_size = orig_size;
    dec->dtext = (uint8_t *)malloc(LHA_DICSIZ);
    if (!dec->dtext) { free(dec); return 0; }
    memset(dec->dtext, ' ', LHA_DICSIZ);

    const int out_capacity = 64 * 1024;
    char *out_buf = (char *)malloc(out_capacity);
    if (!out_buf) { free(dec->dtext); free(dec); return 0; }
    int out_buf_pos = 0;
    int success = 1;
    uint32_t count = 0;

    while (success && count < orig_size) {
        int c = lha_decode_c(dec);
        if (c < 256) {
            out_buf[out_buf_pos++] = (char)c;
            if (out_buf_pos >= out_capacity)
                success = lha_flush_output(out_fd, out_buf, &out_buf_pos);
            dec->dtext[dec->dpos++] = (uint8_t)c;
            if (dec->dpos >= LHA_DICSIZ) dec->dpos = 0;
            count++;
        } else {
            int match_len = c - 256 + LHA_THRESHOLD;
            int offset = lha_decode_p(dec);
            int match_pos = (dec->dpos - offset - 1) & (LHA_DICSIZ - 1);
            for (int k = 0; success && k < match_len && count < orig_size; k++) {
                uint8_t ch = dec->dtext[match_pos++];
                if (match_pos >= LHA_DICSIZ) match_pos = 0;
                out_buf[out_buf_pos++] = (char)ch;
                if (out_buf_pos >= out_capacity)
                    success = lha_flush_output(out_fd, out_buf, &out_buf_pos);
                dec->dtext[dec->dpos++] = ch;
                if (dec->dpos >= LHA_DICSIZ) dec->dpos = 0;
                count++;
            }
        }
    }

    if (success && !lha_flush_output(out_fd, out_buf, &out_buf_pos))
        success = 0;
    free(out_buf);
    free(dec->dtext);
    free(dec);
    return success && count == orig_size;
}

extern "C" void vita_gui_draw_progress(const char *title, const char *subtitle, float fraction, const char *item_name);

typedef struct {
    char method[6];
    uint32_t comp_size;
    uint32_t orig_size;
    uint8_t level;
    char name[512];
    char dirname[512];
    uint32_t ext_total;
} LhaHeaderInfo;

static int lha_read_header(SceUID in_fd, LhaHeaderInfo *h)
{
    uint8_t hsize = 0;
    uint8_t hchk = 0;
    uint32_t time = 0;
    uint16_t crc = 0;
    uint8_t attr = 0;

    if (sceIoRead(in_fd, &hsize, 1) != 1 || hsize == 0)
        return 0;
    if (sceIoRead(in_fd, &hchk, 1) != 1)
        return 0;
    memset(h, 0, sizeof(*h));
    if (sceIoRead(in_fd, h->method, 5) != 5)
        return 0;
    h->method[5] = '\0';
    if (sceIoRead(in_fd, &h->comp_size, 4) != 4 ||
        sceIoRead(in_fd, &h->orig_size, 4) != 4 ||
        sceIoRead(in_fd, &time, 4) != 4 ||
        sceIoRead(in_fd, &attr, 1) != 1 ||
        sceIoRead(in_fd, &h->level, 1) != 1)
        return 0;

    if (h->level == 0 || h->level == 1) {
        uint8_t nlen = 0;
        if (sceIoRead(in_fd, &nlen, 1) != 1)
            return 0;
        if (sceIoRead(in_fd, h->name, nlen) != nlen)
            return 0;
        h->name[nlen] = '\0';
        if (sceIoRead(in_fd, &crc, 2) != 2)
            return 0;
        if (h->level == 1) {
            uint8_t os = 0;
            if (sceIoRead(in_fd, &os, 1) != 1)
                return 0;
            uint16_t ext_size = 0;
            while (sceIoRead(in_fd, &ext_size, 2) == 2 && ext_size > 0) {
                h->ext_total += ext_size;
                uint8_t ext_type = 0;
                if (sceIoRead(in_fd, &ext_type, 1) != 1)
                    return 0;
                if (ext_type == 0x02 && ext_size > 3) {
                    int dlen = ext_size - 3;
                    int read_len = dlen > 510 ? 510 : dlen;
                    if (sceIoRead(in_fd, h->dirname, read_len) != read_len)
                        return 0;
                    h->dirname[read_len] = '\0';
                    for (int i = 0; i < read_len; i++)
                        if (h->dirname[i] == (char)0xFF) h->dirname[i] = '/';
                    if (dlen > read_len)
                        sceIoLseek(in_fd, dlen - read_len, SCE_SEEK_CUR);
                } else if (ext_type == 0x01 && ext_size > 3) {
                    int flen = ext_size - 3;
                    int read_len = flen > 510 ? 510 : flen;
                    if (sceIoRead(in_fd, h->name, read_len) != read_len)
                        return 0;
                    h->name[read_len] = '\0';
                    if (flen > read_len)
                        sceIoLseek(in_fd, flen - read_len, SCE_SEEK_CUR);
                } else if (ext_size > 3) {
                    sceIoLseek(in_fd, ext_size - 3, SCE_SEEK_CUR);
                }
            }
        }
    }
    return 1;
}

static int count_native_lha_files(const char *archive_path)
{
    SceUID in_fd = sceIoOpen(archive_path, SCE_O_RDONLY, 0);
    if (in_fd < 0) return 0;

    int total = 0;
    LhaHeaderInfo h;
    while (lha_read_header(in_fd, &h)) {
        uint32_t actual_comp = (h.level == 1 && h.comp_size >= h.ext_total) ? h.comp_size - h.ext_total : h.comp_size;
        if (strcmp(h.method, "-lhd-") != 0) {
            total++;
        }
        sceIoLseek(in_fd, actual_comp, SCE_SEEK_CUR);
    }

    sceIoClose(in_fd);
    return total;
}

static int extract_native_lha(const char *archive_path, const char *destination_root)
{
    int total_files = count_native_lha_files(archive_path);
    if (total_files <= 0) total_files = 1;

    SceUID in_fd = sceIoOpen(archive_path, SCE_O_RDONLY, 0);
    if (in_fd < 0) return 0;

    int file_count = 0;
    LhaHeaderInfo h;
    while (lha_read_header(in_fd, &h)) {
        char raw_path[1024];
        if (h.dirname[0]) {
            snprintf(raw_path, sizeof(raw_path), "%s%s", h.dirname, h.name);
        } else {
            snprintf(raw_path, sizeof(raw_path), "%s", h.name);
        }

        char relative[512];
        if (!make_safe_relative(raw_path, relative, sizeof(relative))) {
            uint32_t skip_bytes = (h.level == 1 && h.comp_size >= h.ext_total) ? (h.comp_size - h.ext_total) : h.comp_size;
            sceIoLseek(in_fd, skip_bytes, SCE_SEEK_CUR);
            continue;
        }

        char output_path[512];
        snprintf(output_path, sizeof(output_path), "%s/%s", destination_root, relative);
        size_t rel_len = strlen(relative);
        if (rel_len > 0 && relative[rel_len - 1] == '/') {
            ensure_directory(output_path);
            continue;
        }
        if (strcmp(h.method, "-lhd-") == 0) {
            ensure_directory(output_path);
            continue;
        }

        char *slash = strrchr(output_path, '/');
        if (slash) {
            *slash = '\0';
            ensure_directory(output_path);
            *slash = '/';
        }

        uint32_t actual_comp = (h.level == 1 && h.comp_size >= h.ext_total) ? h.comp_size - h.ext_total : h.comp_size;

        float fraction = (float)file_count / (float)total_files;
        char sub[64];
        snprintf(sub, sizeof(sub), "Extracting %d / %d (%d%%)", file_count + 1, total_files, (int)(fraction * 100));
        vita_gui_draw_progress("Extracting WHDLoad Game...", sub, fraction, raw_path);

        SceOff data_start = sceIoLseek(in_fd, 0, SCE_SEEK_CUR);
        if (strcmp(h.method, "-lh0-") == 0) {
            SceUID out_fd = sceIoOpen(output_path, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
            if (out_fd >= 0) {
                char buf[4096];
                uint32_t remaining = h.orig_size;
                while (remaining > 0) {
                    uint32_t to_read = remaining < sizeof(buf) ? remaining : sizeof(buf);
                    int r = sceIoRead(in_fd, buf, to_read);
                    if (r <= 0) break;
                    sceIoWrite(out_fd, buf, r);
                    remaining -= r;
                }
                int copy_ok = remaining == 0;
                sceIoClose(out_fd);
                if (copy_ok) file_count++;
            }
        } else if (strcmp(h.method, "-lh5-") == 0 || strcmp(h.method, "-lh4-") == 0) {
            SceUID out_fd = sceIoOpen(output_path, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
            if (out_fd >= 0) {
                int decode_ok = lha_decode_file(in_fd, out_fd, actual_comp, h.orig_size);
                sceIoClose(out_fd);
                if (decode_ok) file_count++;
            }
        }

        sceIoLseek(in_fd, data_start + actual_comp, SCE_SEEK_SET);
    }

    sceIoClose(in_fd);
    return file_count;
}

struct ArchiveFileContext {
    SceUID fd;
    char buffer[64 * 1024];
};

static int custom_archive_open(struct archive *a, void *client_data)
{
    return ARCHIVE_OK;
}

static la_ssize_t custom_archive_read(struct archive *a, void *client_data, const void **buff)
{
    ArchiveFileContext *ctx = (ArchiveFileContext *)client_data;
    if (!ctx || ctx->fd < 0) return -1;
    *buff = ctx->buffer;
    int bytes = sceIoRead(ctx->fd, ctx->buffer, sizeof(ctx->buffer));
    if (bytes < 0) return -1;
    return (la_ssize_t)bytes;
}

static int custom_archive_close(struct archive *a, void *client_data)
{
    ArchiveFileContext *ctx = (ArchiveFileContext *)client_data;
    if (ctx) {
        if (ctx->fd >= 0) {
            sceIoClose(ctx->fd);
            ctx->fd = -1;
        }
        free(ctx);
    }
    return ARCHIVE_OK;
}

static int write_entry(struct archive *input, const char *path)
{
    SceUID fd = sceIoOpen(path, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
    if (fd < 0)
        return 0;

    char *buffer = (char *)malloc(64 * 1024);
    if (!buffer) {
        sceIoClose(fd);
        return 0;
    }

    la_ssize_t bytes;
    int success = 1;
    while ((bytes = archive_read_data(input, buffer, 64 * 1024)) > 0) {
        if (sceIoWrite(fd, buffer, bytes) != bytes) {
            success = 0;
            break;
        }
    }
    if (bytes < 0)
        success = 0;

    free(buffer);
    sceIoClose(fd);
    return success;
}

static int find_slave_recursive(const char *root, const char *relative, char *result, size_t result_size, int depth)
{
    if (depth > 8)
        return 0;

    char directory_path[512];
    if (relative && relative[0])
        snprintf(directory_path, sizeof(directory_path), "%s/%s", root, relative);
    else
        snprintf(directory_path, sizeof(directory_path), "%s", root);

    SceUID directory = sceIoDopen(directory_path);
    if (directory < 0)
        return 0;

    SceIoDirent entry;
    int found = 0;
    while (!found && sceIoDread(directory, &entry) > 0) {
        if (entry.d_name[0] == '.' || strcmp(entry.d_name, "S") == 0)
            continue;

        char child_relative[384];
        if (relative && relative[0])
            snprintf(child_relative, sizeof(child_relative), "%s/%s", relative, entry.d_name);
        else
            snprintf(child_relative, sizeof(child_relative), "%s", entry.d_name);

        if (SCE_S_ISDIR(entry.d_stat.st_mode)) {
            found = find_slave_recursive(root, child_relative, result, result_size, depth + 1);
        } else {
            const char *dot = strrchr(entry.d_name, '.');
            if (dot && strcasecmp(dot, ".slave") == 0) {
                strncpy(result, child_relative, result_size - 1);
                result[result_size - 1] = '\0';
                found = 1;
            }
        }
    }
    sceIoDclose(directory);
    return found;
}

static int copy_text_file(const char *source, const char *destination)
{
    FILE *input = fopen(source, "rb");
    if (!input)
        return 0;
    FILE *output = fopen(destination, "wb");
    if (!output) {
        fclose(input);
        return 0;
    }
    char buffer[4096];
    size_t bytes;
    int success = 1;
    while ((bytes = fread(buffer, 1, sizeof(buffer), input)) > 0) {
        if (fwrite(buffer, 1, bytes, output) != bytes) {
            success = 0;
            break;
        }
    }
    fclose(input);
    fclose(output);
    return success;
}

static int read_text_file(const char *path, char *buffer, size_t buffer_size)
{
    FILE *input = fopen(path, "rb");
    if (!input)
        return 0;
    size_t length = fread(buffer, 1, buffer_size - 1, input);
    fclose(input);
    buffer[length] = '\0';
    return 1;
}

static int write_text_file(const char *path, const char *text)
{
    FILE *output = fopen(path, "wb");
    if (!output)
        return 0;
    size_t length = strlen(text);
    int success = fwrite(text, 1, length, output) == length;
    fclose(output);
    return success;
}

static char *find_endcli(char *text)
{
    for (char *p = text; *p; p++) {
        if ((p == text || p[-1] == '\n') &&
            (p[0] == 'E' || p[0] == 'e') &&
            (p[1] == 'N' || p[1] == 'n') &&
            (p[2] == 'D' || p[2] == 'd') &&
            (p[3] == 'C' || p[3] == 'c') &&
            (p[4] == 'L' || p[4] == 'l') &&
            (p[5] == 'I' || p[5] == 'i'))
            return p;
    }
    return NULL;
}

static char s_last_error[256] = "";

const char *vita_whdload_get_last_error(void)
{
    return s_last_error;
}

const char *vita_whdload_root(void)
{
    return VITA_WHDLOAD_ROOT;
}

int vita_whdload_install_lha(const char *archive_path, char *installed_path, size_t installed_path_size)
{
    s_last_error[0] = '\0';
    if (!archive_path || !installed_path || installed_path_size == 0) {
        snprintf(s_last_error, sizeof(s_last_error), "Invalid parameters");
        return 0;
    }

    installed_path[0] = '\0';
    ensure_directory(VITA_WHDLOAD_ROOT);

    char base[128];
    char folder[128];
    make_folder_name(archive_path, base, sizeof(base));
    strncpy(folder, base, sizeof(folder) - 1);
    folder[sizeof(folder) - 1] = '\0';

    char destination_root[512];
    snprintf(destination_root, sizeof(destination_root), "%s/%s", VITA_WHDLOAD_ROOT, folder);
    ensure_directory(destination_root);

    int native_count = extract_native_lha(archive_path, destination_root);
    if (native_count > 0) {
        strncpy(installed_path, destination_root, installed_path_size - 1);
        installed_path[installed_path_size - 1] = '\0';
        return 1;
    }

    struct archive *input = archive_read_new();
    struct archive_entry *entry = NULL;
    if (!input) {
        snprintf(s_last_error, sizeof(s_last_error), "archive_read_new failed");
        return 0;
    }

    ArchiveFileContext *ctx = (ArchiveFileContext *)malloc(sizeof(ArchiveFileContext));
    if (!ctx) {
        snprintf(s_last_error, sizeof(s_last_error), "malloc failed");
        archive_read_free(input);
        return 0;
    }

    ctx->fd = sceIoOpen(archive_path, SCE_O_RDONLY, 0);
    if (ctx->fd < 0) {
        snprintf(s_last_error, sizeof(s_last_error), "Cannot open %s (err %d)", archive_path, (int)ctx->fd);
        free(ctx);
        archive_read_free(input);
        return 0;
    }

    archive_read_support_filter_all(input);
    archive_read_support_format_lha(input);
    archive_read_support_format_zip(input);
    archive_read_support_format_7zip(input);
    archive_read_support_format_all(input);
    archive_read_set_open_callback(input, custom_archive_open);
    archive_read_set_read_callback(input, custom_archive_read);
    archive_read_set_close_callback(input, custom_archive_close);
    archive_read_set_callback_data(input, ctx);

    int open_res = archive_read_open1(input);
    if (open_res != ARCHIVE_OK && open_res != ARCHIVE_WARN) {
        snprintf(s_last_error, sizeof(s_last_error), "open1 err %d: %s", open_res, archive_error_string(input));
        archive_read_free(input);
        return 0;
    }

    int extracted = 0;
    int r;
    while ((r = archive_read_next_header(input, &entry)) == ARCHIVE_OK || r == ARCHIVE_WARN) {
        const char *entry_name = archive_entry_pathname(entry);
        char relative[384];
        if (!make_safe_relative(entry_name, relative, sizeof(relative))) {
            archive_read_data_skip(input);
            continue;
        }

        char output_path[512];
        snprintf(output_path, sizeof(output_path), "%s/%s", destination_root, relative);
        mode_t file_type = archive_entry_filetype(entry);
        size_t rel_len = strlen(relative);
        if (file_type == AE_IFDIR || (rel_len > 0 && relative[rel_len - 1] == '/')) {
            ensure_directory(output_path);
            continue;
        }

        char *slash = strrchr(output_path, '/');
        if (slash) {
            *slash = '\0';
            ensure_directory(output_path);
            *slash = '/';
        }
        if (write_entry(input, output_path)) {
            extracted++;
        } else {
            archive_read_data_skip(input);
        }
    }

    if (r != ARCHIVE_EOF && r != ARCHIVE_OK && r != ARCHIVE_WARN && extracted == 0) {
        snprintf(s_last_error, sizeof(s_last_error), "read_header %d: %s", r, archive_error_string(input));
    }

    archive_read_free(input);
    if (extracted == 0) {
        if (s_last_error[0] == '\0') {
            snprintf(s_last_error, sizeof(s_last_error), "0 files extracted from archive");
        }
        return 0;
    }

    strncpy(installed_path, destination_root, installed_path_size - 1);
    installed_path[installed_path_size - 1] = '\0';
    return 1;
}

static void deploy_file_if_missing(const char *src, const char *dst)
{
    SceIoStat st;
    if (sceIoGetstat(dst, &st) >= 0 && st.st_size > 0) return;
    char dst_copy[512];
    strncpy(dst_copy, dst, sizeof(dst_copy) - 1);
    dst_copy[sizeof(dst_copy) - 1] = '\0';
    char *slash = strrchr(dst_copy, '/');
    if (slash) {
        *slash = '\0';
        ensure_directory(dst_copy);
        *slash = '/';
    }
    copy_text_file(src, dst);
}

static void deploy_kickstart_file(const char *destination_name, const char *const *source_names)
{
    char destination[512];
    snprintf(destination, sizeof(destination), "%s/Devs/Kickstarts/%s", VITA_WHDLOAD_ROOT, destination_name);
    for (int i = 0; source_names[i]; i++) {
        char source[512];
        snprintf(source, sizeof(source), "ux0:/data/uae4all/kickstarts/%s", source_names[i]);
        SceIoStat st;
        if (sceIoGetstat(source, &st) >= 0 && st.st_size > 0) {
            deploy_file_if_missing(source, destination);
            return;
        }
    }
}

static void deploy_kickstart_aliases(void)
{
    static const char *kick12[] = { "kick12.rom", "kick33180.A500", "amiga-os-120.rom", NULL };
    static const char *kick13[] = { "kick13.rom", "kick34005.A500", "amiga-os-130.rom", NULL };
    static const char *kick20[] = { "kick20.rom", "kick37175.A500", "amiga-os-204.rom", NULL };
    static const char *kick31[] = { "kick31.rom", "kick40068.A1200", "amiga-os-310-a1200.rom", NULL };
    static const char *kick205[] = { "kick37350.A600", "kick205.rom", "amiga-os-205-a600.rom", NULL };
    static const char *cd32[] = { "kick40060.CD32", "amiga-os-310-cd32.rom", NULL };
    static const struct {
        const char *name;
        const char *const *sources;
    } files[] = {
        { "kick12.rom", kick12 }, { "kick33180.A500", kick12 },
        { "kick34005.A500", kick13 }, { "kick13.rom", kick13 },
        { "kick20.rom", kick20 }, { "kick37175.A500", kick20 },
        { "kick31.rom", kick31 }, { "kick40068.A1200", kick31 },
        { "kick37350.A600", kick205 }, { "kick205.rom", kick205 },
        { "kick40060.CD32", cd32 }, { "amiga-os-310-cd32.rom", cd32 },
        { NULL, NULL }
    };
    ensure_directory(VITA_WHDLOAD_ROOT "/Devs/Kickstarts");
    for (int i = 0; files[i].name; i++)
        deploy_kickstart_file(files[i].name, files[i].sources);
    static const char *kick_rtb[] = { "kick34005.A500.RTB", NULL };
    deploy_kickstart_file("kick34005.A500.RTB", kick_rtb);
}

static void deploy_whdload_base(void)
{
    ensure_directory(VITA_WHDLOAD_ROOT);
    deploy_kickstart_aliases();
    deploy_file_if_missing("app0:/data/whdload_base/C/WHDLoad", VITA_WHDLOAD_ROOT "/C/WHDLoad");
    deploy_file_if_missing("app0:/data/whdload_base/C/WHDLoadCD32", VITA_WHDLOAD_ROOT "/C/WHDLoadCD32");
    deploy_file_if_missing("app0:/data/whdload_base/C/DIC", VITA_WHDLOAD_ROOT "/C/DIC");
    deploy_file_if_missing("app0:/data/whdload_base/C/Patcher", VITA_WHDLOAD_ROOT "/C/Patcher");
    deploy_file_if_missing("app0:/data/whdload_base/C/RawDIC", VITA_WHDLOAD_ROOT "/C/RawDIC");
    deploy_file_if_missing("app0:/data/whdload_base/C/WArc", VITA_WHDLOAD_ROOT "/C/WArc");
    deploy_file_if_missing("app0:/data/whdload_base/C/WHDLoad.VFS", VITA_WHDLOAD_ROOT "/C/WHDLoad.VFS");
    deploy_file_if_missing("app0:/data/whdload_base/S/WHDLoad.prefs", VITA_WHDLOAD_ROOT "/S/WHDLoad.prefs");
    deploy_file_if_missing("app0:/data/whdload_base/S/WHDLoad-Startup", VITA_WHDLOAD_ROOT "/S/WHDLoad-Startup");
    deploy_file_if_missing("app0:/data/whdload_base/S/WHDLoad-Cleanup", VITA_WHDLOAD_ROOT "/S/WHDLoad-Cleanup");
}

int vita_whdload_prepare_launch(const char *game_name)
{
    if (!game_name || game_name[0] == '\0' || strchr(game_name, '/') || strchr(game_name, '\\') || strchr(game_name, ':'))
        return 0;

    deploy_whdload_base();

    char game_root[512];
    char slave_relative[384];
    snprintf(game_root, sizeof(game_root), "%s/%s", VITA_WHDLOAD_ROOT, game_name);
    int is_dir = 0;
    if (!path_exists(game_root, &is_dir) || !is_dir)
        return 0;
    if (!find_slave_recursive(game_root, "", slave_relative, sizeof(slave_relative), 0))
        return 0;

    char startup_dir[512];
    char startup_path[512];
    char backup_path[512];
    snprintf(startup_dir, sizeof(startup_dir), "%s/S", VITA_WHDLOAD_ROOT);
    snprintf(startup_path, sizeof(startup_path), "%s/Startup-Sequence", startup_dir);
    snprintf(backup_path, sizeof(backup_path), "%s/Startup-Sequence.uae4all", startup_dir);
    ensure_directory(startup_dir);

    int startup_exists = path_exists(startup_path, NULL);
    char *original_startup = (char *)calloc(1, 65536);
    if (!original_startup)
        return 0;

    int has_real_workbench = 0;
    if (path_exists(backup_path, NULL)) {
        read_text_file(backup_path, original_startup, 65536);
    } else if (startup_exists) {
        read_text_file(startup_path, original_startup, 65536);
    }

    if (original_startup[0] && strstr(original_startup, "SetPatch") != NULL) {
        has_real_workbench = 1;
    }

    char amiga_dir[512];
    char slave_file[128];
    const char *last_slash = strrchr(slave_relative, '/');
    if (last_slash) {
        char sub_dir[384];
        size_t dlen = (size_t)(last_slash - slave_relative);
        strncpy(sub_dir, slave_relative, dlen);
        sub_dir[dlen] = '\0';
        snprintf(amiga_dir, sizeof(amiga_dir), "DH0:%s/%s", game_name, sub_dir);
        strncpy(slave_file, last_slash + 1, sizeof(slave_file) - 1);
        slave_file[sizeof(slave_file) - 1] = '\0';
    } else {
        snprintf(amiga_dir, sizeof(amiga_dir), "DH0:%s", game_name);
        strncpy(slave_file, slave_relative, sizeof(slave_file) - 1);
        slave_file[sizeof(slave_file) - 1] = '\0';
    }

    char launch_line[640];
    snprintf(launch_line, sizeof(launch_line), "CD \"%s\"\nC:WHDLoad \"%s\" PRELOAD\n", amiga_dir, slave_file);

    char *updated_startup = (char *)calloc(1, 66000);
    if (!updated_startup) {
        free(original_startup);
        return 0;
    }

    if (has_real_workbench) {
        char *endcli = find_endcli(original_startup);
        if (endcli) {
            size_t prefix_length = (size_t)(endcli - original_startup);
            memcpy(updated_startup, original_startup, prefix_length);
            memcpy(updated_startup + prefix_length, launch_line, strlen(launch_line));
            strcpy(updated_startup + prefix_length + strlen(launch_line), endcli);
        } else {
            strcpy(updated_startup, original_startup);
            strcat(updated_startup, launch_line);
        }
    } else {
        snprintf(updated_startup, 66000, "CD \"%s\"\nC:WHDLoad \"%s\" PRELOAD\n", amiga_dir, slave_file);
    }

    write_text_file(startup_path, updated_startup);
    free(updated_startup);
    free(original_startup);
    return 1;
}

int vita_whdload_list(char names[][128], int max_names)
{
    if (!names || max_names <= 0)
        return 0;

    ensure_directory(VITA_WHDLOAD_ROOT);
    SceUID directory = sceIoDopen(VITA_WHDLOAD_ROOT);
    if (directory < 0)
        return 0;

    int count = 0;
    SceIoDirent entry;
    while (count < max_names && sceIoDread(directory, &entry) > 0) {
        if (entry.d_name[0] == '.')
            continue;
        if (!SCE_S_ISDIR(entry.d_stat.st_mode))
            continue;
        if (strcasecmp(entry.d_name, "S") == 0 ||
            strcasecmp(entry.d_name, "C") == 0 ||
            strcasecmp(entry.d_name, "Libs") == 0 ||
            strcasecmp(entry.d_name, "Devs") == 0 ||
            strcasecmp(entry.d_name, "L") == 0 ||
            strcasecmp(entry.d_name, "Fonts") == 0 ||
            strcasecmp(entry.d_name, "Locale") == 0)
            continue;
        strncpy(names[count], entry.d_name, 127);
        names[count][127] = '\0';
        count++;
    }
    sceIoDclose(directory);
    return count;
}

#endif
