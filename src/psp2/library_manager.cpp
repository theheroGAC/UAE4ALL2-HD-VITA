#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#ifdef __PSP2__
#include <psp2/types.h>
#include <psp2/io/dirent.h>
#include <psp2/io/stat.h>
#endif

#include "library_manager.h"

#define VITA_LIBRARY_MAGIC      "UAE4ALL-LIBRARY"
#define VITA_LIBRARY_VERSION    4
#define VITA_LIBRARY_MAX_DEPTH  5
#define VITA_LIBRARY_ZIP_TAIL   (64 * 1024)
#define VITA_LIBRARY_MAX_COUNT  999999

static VitaLibraryEntry s_lib_entries[VITA_LIBRARY_MAX];
static int s_lib_count = 0;
static int s_lib_loaded = 0;
static int s_lib_stale = 1;
static char s_lib_roots[VITA_LIBRARY_MAX_ROOTS][VITA_LIBRARY_PATH_LEN];
static unsigned long s_lib_sig_mtime[VITA_LIBRARY_MAX_ROOTS];
static int s_lib_sig_ready = 0;
static int s_lib_root_count = 0;

int vita_library_kind_from_ext(const char *filename)
{
    const char *ext;

    if (!filename)
        return VITA_LIB_KIND_UNKNOWN;

    ext = strrchr(filename, '.');
    if (!ext)
        return VITA_LIB_KIND_UNKNOWN;

    if (!strcasecmp(ext, ".adf") || !strcasecmp(ext, ".adz") || !strcasecmp(ext, ".dms") ||
        !strcasecmp(ext, ".ipf") || !strcasecmp(ext, ".fdi"))
        return VITA_LIB_KIND_FLOPPY;
    if (!strcasecmp(ext, ".hdf") || !strcasecmp(ext, ".hda") || !strcasecmp(ext, ".vhd"))
        return VITA_LIB_KIND_HDF;
    if (!strcasecmp(ext, ".lha") || !strcasecmp(ext, ".lzh"))
        return VITA_LIB_KIND_LHA;
    if (!strcasecmp(ext, ".chd") || !strcasecmp(ext, ".cue") || !strcasecmp(ext, ".iso") ||
        !strcasecmp(ext, ".m3u") || !strcasecmp(ext, ".bin"))
        return VITA_LIB_KIND_CD;

    return VITA_LIB_KIND_UNKNOWN;
}

int vita_library_kind_label_for(int kind, const char **label)
{
    const char *name;

    switch (kind) {
    case VITA_LIB_KIND_FLOPPY:  name = "FLOPPY";  break;
    case VITA_LIB_KIND_HDF:     name = "HARD DISK"; break;
    case VITA_LIB_KIND_WHDLOAD: name = "WHDLOAD"; break;
    case VITA_LIB_KIND_LHA:     name = "LHA";     break;
    case VITA_LIB_KIND_CD:      name = "CD32";    break;
    default:                    name = "UNKNOWN"; break;
    }

    if (label)
        *label = name;
    return 0;
}

static unsigned int lib_le16(const unsigned char *p)
{
    return (unsigned int)p[0] | ((unsigned int)p[1] << 8);
}

static unsigned long lib_le32(const unsigned char *p)
{
    return (unsigned long)((unsigned long)p[0] | ((unsigned long)p[1] << 8) |
                           ((unsigned long)p[2] << 16) | ((unsigned long)p[3] << 24));
}

unsigned long vita_library_zip_cd_offset(const unsigned char *tail, size_t tail_len)
{
    size_t i;

    if (!tail || tail_len < 22)
        return 0;

    i = tail_len - 22;
    for (;;) {
        if (tail[i] == 'P' && tail[i + 1] == 'K' && tail[i + 2] == 5 && tail[i + 3] == 6)
            return lib_le32(tail + i + 16);
        if (i == 0)
            break;
        i--;
    }

    return 0;
}

int vita_library_zip_kind(const unsigned char *cd, size_t cd_len)
{
    size_t off = 0;

    if (!cd)
        return VITA_LIB_KIND_UNKNOWN;

    while (off + 46 <= cd_len) {
        unsigned int name_len, extra_len, comment_len, copy, i;
        char name[300];
        int kind;
        int is_dir = 0;

        if (cd[off] != 'P' || cd[off + 1] != 'K' || cd[off + 2] != 1 || cd[off + 3] != 2)
            break;

        name_len = lib_le16(cd + off + 28);
        extra_len = lib_le16(cd + off + 30);
        comment_len = lib_le16(cd + off + 32);

        if (off + 46 + name_len > cd_len)
            break;

        copy = name_len < sizeof(name) - 1 ? name_len : (unsigned int)(sizeof(name) - 1);
        for (i = 0; i < copy; i++)
            name[i] = (char)cd[off + 46 + i];
        name[copy] = '\0';

        if (copy > 0 && name[copy - 1] == '/')
            is_dir = 1;

        if (!is_dir) {
            kind = vita_library_kind_from_ext(name);
            if (kind == VITA_LIB_KIND_FLOPPY || kind == VITA_LIB_KIND_CD ||
                kind == VITA_LIB_KIND_LHA)
                return kind;
        }

        off += 46 + name_len + extra_len + comment_len;
    }

    return VITA_LIB_KIND_UNKNOWN;
}

static char lib_kind_char(int kind)
{
    switch (kind) {
    case VITA_LIB_KIND_FLOPPY:  return 'F';
    case VITA_LIB_KIND_HDF:     return 'H';
    case VITA_LIB_KIND_WHDLOAD: return 'W';
    case VITA_LIB_KIND_LHA:     return 'L';
    case VITA_LIB_KIND_CD:      return 'C';
    default:                    return '?';
    }
}

static int lib_char_kind(char c)
{
    switch (c) {
    case 'F': return VITA_LIB_KIND_FLOPPY;
    case 'H': return VITA_LIB_KIND_HDF;
    case 'W': return VITA_LIB_KIND_WHDLOAD;
    case 'L': return VITA_LIB_KIND_LHA;
    case 'C': return VITA_LIB_KIND_CD;
    default:  return VITA_LIB_KIND_UNKNOWN;
    }
}

int vita_library_format_entry(const VitaLibraryEntry *entry, char *out, size_t out_size)
{
    if (!entry || !out || out_size == 0)
        return 0;

    return snprintf(out, out_size, "%c\t%llu\t%s\n", lib_kind_char(entry->kind),
                    entry->size, entry->path) > 0;
}

int vita_library_parse_entry(const char *line, VitaLibraryEntry *entry)
{
    unsigned long long size = 0;
    size_t len;
    const char *p;
    int kind;

    if (!line || !entry)
        return 0;

    kind = lib_char_kind(line[0]);
    if (kind == VITA_LIB_KIND_UNKNOWN)
        return 0;
    if (line[1] != '\t' && line[1] != ' ')
        return 0;

    p = line + 2;
    if (*p < '0' || *p > '9')
        return 0;
    while (*p >= '0' && *p <= '9') {
        size = size * 10ULL + (unsigned long long)(*p - '0');
        p++;
    }
    if (*p != '\t' && *p != ' ')
        return 0;
    p++;

    len = strcspn(p, "\r\n");
    if (len == 0 || len >= VITA_LIBRARY_PATH_LEN)
        return 0;

    memcpy(entry->path, p, len);
    entry->path[len] = '\0';
    entry->size = size;
    entry->kind = kind;
    return 1;
}

int vita_library_dedup_entries(VitaLibraryEntry *entries, int count)
{
    int out = 0;

    if (!entries || count <= 0)
        return 0;

    for (int i = 0; i < count; i++) {
        if (out > 0 && strcasecmp(entries[out - 1].path, entries[i].path) == 0)
            continue;
        if (out != i)
            entries[out] = entries[i];
        out++;
    }

    return out;
}

static const char *lib_ext_of(const char *path)
{
    const char *ext = path ? strrchr(path, '.') : NULL;

    if (!ext || ext[1] == '\0' || strchr(ext, '/'))
        return NULL;
    return ext;
}

static int lib_has_path(const VitaLibraryEntry *entries, int count, const char *path)
{
    for (int i = 0; i < count; i++) {
        if (strcasecmp(entries[i].path, path) == 0)
            return 1;
    }
    return 0;
}

int vita_library_drop_sidecar_bins(VitaLibraryEntry *entries, int count)
{
    int out = 0;

    if (!entries || count <= 0)
        return 0;

    for (int i = 0; i < count; i++) {
        const char *ext = lib_ext_of(entries[i].path);
        int drop = 0;

        if (ext && !strcasecmp(ext, ".bin")) {
            static const char *companions[] = { ".m3u", ".cue", ".iso", NULL };
            size_t base_len = (size_t)(ext - entries[i].path);

            for (int c = 0; companions[c] && !drop; c++) {
                char candidate[VITA_LIBRARY_PATH_LEN];

                if (base_len + strlen(companions[c]) >= sizeof(candidate))
                    continue;
                memcpy(candidate, entries[i].path, base_len);
                snprintf(candidate + base_len, sizeof(candidate) - base_len, "%s", companions[c]);

                for (int j = 0; j < count; j++) {
                    if (j != i && strcasecmp(entries[j].path, candidate) == 0) {
                        drop = 1;
                        break;
                    }
                }
            }
        }

        if (drop)
            continue;
        if (out != i)
            entries[out] = entries[i];
        out++;
    }

    return out;
}

int vita_library_parse_root_line(const char *line, char *out, size_t out_size)
{
    size_t start, len;

    if (!line || !out || out_size == 0)
        return 0;

    out[0] = '\0';

    while (*line == ' ' || *line == '\t' || *line == '\r' || *line == '\n')
        line++;
    if (*line == '\0' || *line == '#' || *line == ';')
        return 0;

    start = 0;
    len = strcspn(line, "\r\n");
    while (len > 0 && (line[len - 1] == ' ' || line[len - 1] == '\t'))
        len--;
    while (start < len && (line[start] == ' ' || line[start] == '\t'))
        start++;
    len -= start;

    while (len > 1 && line[start + len - 1] == '/')
        len--;

    if (len == 0 || len >= out_size)
        return 0;
    if (strchr(line + start, ':') == NULL)
        return 0;

    memcpy(out, line + start, len);
    out[len] = '\0';
    return 1;
}

int vita_library_root_count(void)
{
    return s_lib_root_count;
}

const char *vita_library_root(int index)
{
    if (index < 0 || index >= s_lib_root_count)
        return NULL;
    return s_lib_roots[index];
}

int vita_library_is_stale(void)
{
    return s_lib_stale;
}

int vita_library_count(void)
{
    return s_lib_count;
}

const VitaLibraryEntry *vita_library_entry(int index)
{
    if (index < 0 || index >= s_lib_count)
        return NULL;
    return &s_lib_entries[index];
}

void vita_library_reset(void)
{
    s_lib_count = 0;
    s_lib_loaded = 0;
    s_lib_stale = 1;
}

static void lib_add_root(const char *path)
{
    size_t len;

    if (!path || path[0] == '\0' || s_lib_root_count >= VITA_LIBRARY_MAX_ROOTS)
        return;

    len = strlen(path);
    if (len == 0 || len >= VITA_LIBRARY_PATH_LEN)
        return;

    for (int i = 0; i < s_lib_root_count; i++) {
        if (strcasecmp(s_lib_roots[i], path) == 0)
            return;
    }

    memcpy(s_lib_roots[s_lib_root_count], path, len + 1);
    s_lib_root_count++;
}

#ifdef __PSP2__

static int lib_is_root_path(const char *path)
{
    for (int i = 0; i < s_lib_root_count; i++) {
        if (strcasecmp(s_lib_roots[i], path) == 0)
            return 1;
    }
    return 0;
}

static int lib_is_excluded_dir(const char *name)
{
    static const char *excluded[] = {
        "saves", "whdload", "covers", "thumbs", "conf", "kickstarts", "tmp", "data", NULL
    };

    for (int i = 0; excluded[i]; i++) {
        if (!strcasecmp(name, excluded[i]))
            return 1;
    }
    return 0;
}

static int lib_build_roots(void)
{
    FILE *f;
    char line[VITA_LIBRARY_PATH_LEN + 16];

    s_lib_root_count = 0;

    f = fopen(VITA_LIBRARY_ROOTS_FILE, "rb");
    if (f) {
        while (fgets(line, sizeof(line), f)) {
            char root[VITA_LIBRARY_PATH_LEN];
            if (vita_library_parse_root_line(line, root, sizeof(root)))
                lib_add_root(root);
        }
        fclose(f);
    }

    if (s_lib_root_count == 0) {
        lib_add_root(VITA_LIBRARY_ROMS_DIR);
        lib_add_root(VITA_LIBRARY_HOME_DIR);
    }

    return s_lib_root_count;
}

static unsigned long lib_datetime_key(const SceDateTime *dt)
{
    return ((((unsigned long)dt->year * 13UL + dt->month) * 32UL + dt->day) * 24UL + dt->hour) * 3600UL
         + (unsigned long)dt->minute * 60UL + dt->second;
}

static void lib_root_signature(const char *root, unsigned long *mtime, unsigned long *count)
{
    SceIoStat st;
    SceUID dfd;
    SceIoDirent entry;
    unsigned long n = 0;

    *mtime = 0;
    *count = 0;

    if (sceIoGetstat(root, &st) < 0)
        return;

    *mtime = lib_datetime_key(&st.st_mtime);

    dfd = sceIoDopen(root);
    if (dfd < 0)
        return;

    while (n < VITA_LIBRARY_MAX_COUNT && sceIoDread(dfd, &entry) > 0)
        n++;
    sceIoDclose(dfd);

    *count = n;
}

static int lib_compare_entries(const void *a, const void *b)
{
    const VitaLibraryEntry *ea = (const VitaLibraryEntry *)a;
    const VitaLibraryEntry *eb = (const VitaLibraryEntry *)b;

    return strcasecmp(ea->path, eb->path);
}

static void lib_store(const char *path, unsigned long long size, int kind)
{
    size_t len;

    if (s_lib_count >= VITA_LIBRARY_MAX)
        return;

    len = strlen(path);
    if (len == 0 || len >= VITA_LIBRARY_PATH_LEN)
        return;

    memcpy(s_lib_entries[s_lib_count].path, path, len + 1);
    s_lib_entries[s_lib_count].size = size;
    s_lib_entries[s_lib_count].kind = kind;
    s_lib_count++;
}

static int lib_zip_probe(const char *path)
{
    unsigned char tail[VITA_LIBRARY_ZIP_TAIL];
    unsigned char cd[VITA_LIBRARY_ZIP_TAIL];
    FILE *f;
    long fsz;
    size_t tail_len, cd_len;
    unsigned long cd_off;

    f = fopen(path, "rb");
    if (!f)
        return VITA_LIB_KIND_UNKNOWN;

    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return VITA_LIB_KIND_UNKNOWN;
    }
    fsz = ftell(f);
    if (fsz < 22 || fsz > 0x40000000L) {
        fclose(f);
        return VITA_LIB_KIND_UNKNOWN;
    }

    tail_len = (size_t)fsz < sizeof(tail) ? (size_t)fsz : sizeof(tail);
    if (fseek(f, fsz - (long)tail_len, SEEK_SET) != 0) {
        fclose(f);
        return VITA_LIB_KIND_UNKNOWN;
    }
    tail_len = fread(tail, 1, tail_len, f);

    cd_off = vita_library_zip_cd_offset(tail, tail_len);
    if (cd_off == 0 || cd_off >= (unsigned long)fsz) {
        fclose(f);
        return VITA_LIB_KIND_UNKNOWN;
    }
    if (fseek(f, (long)cd_off, SEEK_SET) != 0) {
        fclose(f);
        return VITA_LIB_KIND_UNKNOWN;
    }
    cd_len = fread(cd, 1, sizeof(cd), f);

    fclose(f);
    return vita_library_zip_kind(cd, cd_len);
}

static void lib_scan_dir(const char *dir, int depth, int max_depth)
{
    SceUID dfd;
    SceIoDirent entry;

    if (depth > max_depth || s_lib_count >= VITA_LIBRARY_MAX)
        return;

    dfd = sceIoDopen(dir);
    if (dfd < 0)
        return;

    while (s_lib_count < VITA_LIBRARY_MAX && sceIoDread(dfd, &entry) > 0) {
        char full[VITA_LIBRARY_PATH_LEN];
        int kind;

        if (entry.d_name[0] == '.')
            continue;

        snprintf(full, sizeof(full), "%s/%s", dir, entry.d_name);

        if (SCE_S_ISDIR(entry.d_stat.st_mode)) {
            if (depth < max_depth && !lib_is_excluded_dir(entry.d_name) && !lib_is_root_path(full))
                lib_scan_dir(full, depth + 1, max_depth);
            continue;
        }

        kind = vita_library_kind_from_ext(entry.d_name);
        if (kind == VITA_LIB_KIND_UNKNOWN) {
            const char *ext = strrchr(entry.d_name, '.');
            if (ext && !strcasecmp(ext, ".zip"))
                kind = lib_zip_probe(full);
        }
        if (kind == VITA_LIB_KIND_UNKNOWN)
            continue;

        lib_store(full, (unsigned long long)entry.d_stat.st_size, kind);
    }

    sceIoDclose(dfd);
}

static void lib_save_cache(void)
{
    FILE *f;
    int i;

    f = fopen(VITA_LIBRARY_FILE, "wb");
    if (!f)
        return;

    fprintf(f, "%s %d\n", VITA_LIBRARY_MAGIC, VITA_LIBRARY_VERSION);
    for (i = 0; i < s_lib_root_count; i++) {
        unsigned long mtime, count;
        lib_root_signature(s_lib_roots[i], &mtime, &count);
        fprintf(f, "R\t%lu\t%lu\t%s\n", mtime, count, s_lib_roots[i]);
    }
    for (i = 0; i < s_lib_count; i++) {
        char line[VITA_LIBRARY_PATH_LEN + 32];
        if (vita_library_format_entry(&s_lib_entries[i], line, sizeof(line)))
            fputs(line, f);
    }

    fclose(f);
}

static int lib_load_cache(void)
{
    FILE *f;
    char line[VITA_LIBRARY_PATH_LEN + 32];
    char cached_roots[VITA_LIBRARY_MAX_ROOTS][VITA_LIBRARY_PATH_LEN];
    unsigned long cached_mtime[VITA_LIBRARY_MAX_ROOTS];
    unsigned long cached_count[VITA_LIBRARY_MAX_ROOTS];
    int cached_root_count = 0;
    int loaded = 0;

    s_lib_count = 0;

    f = fopen(VITA_LIBRARY_FILE, "rb");
    if (!f)
        return 0;

    if (!fgets(line, sizeof(line), f) ||
        strncmp(line, VITA_LIBRARY_MAGIC, strlen(VITA_LIBRARY_MAGIC)) != 0 ||
        atoi(line + strlen(VITA_LIBRARY_MAGIC)) != VITA_LIBRARY_VERSION) {
        fclose(f);
        return 0;
    }

    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0')
            continue;

        if (line[0] == 'R' && (line[1] == '\t' || line[1] == ' ')) {
            char *p = line + 2;
            char *end;
            if (cached_root_count >= VITA_LIBRARY_MAX_ROOTS)
                continue;
            cached_mtime[cached_root_count] = (unsigned long)strtoul(p, &end, 10);
            if (end == p || (*end != '\t' && *end != ' '))
                continue;
            p = end + 1;
            cached_count[cached_root_count] = (unsigned long)strtoul(p, &end, 10);
            if (end == p || (*end != '\t' && *end != ' '))
                continue;
            snprintf(cached_roots[cached_root_count], VITA_LIBRARY_PATH_LEN, "%s", end + 1);
            if (cached_roots[cached_root_count][0] == '\0')
                continue;
            cached_root_count++;
            continue;
        }

        if (s_lib_count < VITA_LIBRARY_MAX) {
            VitaLibraryEntry entry;
            if (vita_library_parse_entry(line, &entry)) {
                s_lib_entries[s_lib_count] = entry;
                s_lib_count++;
                loaded = 1;
            }
        }
    }

    fclose(f);

    if (!loaded)
        return 0;

    if (s_lib_count > 1)
        qsort(s_lib_entries, s_lib_count, sizeof(VitaLibraryEntry), lib_compare_entries);
    s_lib_count = vita_library_dedup_entries(s_lib_entries, s_lib_count);
    s_lib_count = vita_library_drop_sidecar_bins(s_lib_entries, s_lib_count);

    if (cached_root_count != s_lib_root_count)
        return 0;

    for (int i = 0; i < s_lib_root_count; i++) {
        unsigned long mtime, count;

        if (strcasecmp(cached_roots[i], s_lib_roots[i]) != 0)
            return 0;

        lib_root_signature(s_lib_roots[i], &mtime, &count);
        if (mtime != cached_mtime[i] || count != cached_count[i])
            return 0;

        s_lib_sig_mtime[i] = mtime;
    }

    s_lib_sig_ready = 1;
    return 1;
}

static int lib_rebuild(void)
{
    s_lib_count = 0;

    lib_build_roots();
    for (int i = 0; i < s_lib_root_count; i++)
        lib_scan_dir(s_lib_roots[i], 0, VITA_LIBRARY_MAX_DEPTH);

    if (s_lib_count > 1)
        qsort(s_lib_entries, s_lib_count, sizeof(VitaLibraryEntry), lib_compare_entries);

    s_lib_count = vita_library_dedup_entries(s_lib_entries, s_lib_count);
    s_lib_count = vita_library_drop_sidecar_bins(s_lib_entries, s_lib_count);

    lib_save_cache();

    for (int i = 0; i < s_lib_root_count; i++) {
        unsigned long mtime, count;
        lib_root_signature(s_lib_roots[i], &mtime, &count);
        s_lib_sig_mtime[i] = mtime;
    }

    s_lib_sig_ready = 1;
    s_lib_stale = 0;
    return s_lib_count;
}

int vita_library_check_stale(void)
{
    if (!s_lib_sig_ready || s_lib_root_count == 0)
        return 0;

    for (int i = 0; i < s_lib_root_count; i++) {
        unsigned long mtime, count;

        lib_root_signature(s_lib_roots[i], &mtime, &count);
        if (mtime != s_lib_sig_mtime[i]) {
            s_lib_stale = 1;
            return 1;
        }
    }

    return 0;
}

int vita_library_preload(void)
{
    if (s_lib_loaded)
        return s_lib_count;

    s_lib_loaded = 1;
    s_lib_stale = 1;
    lib_build_roots();

    if (lib_load_cache()) {
        s_lib_stale = 0;
        return s_lib_count;
    }

    s_lib_count = 0;
    return 0;
}

int vita_library_scan(int force_rebuild)
{
    if (s_lib_loaded && !force_rebuild && !s_lib_stale)
        return s_lib_count;

    s_lib_loaded = 1;
    return lib_rebuild();
}

#else

int vita_library_preload(void)
{
    s_lib_count = 0;
    s_lib_loaded = 1;
    s_lib_stale = 0;
    return 0;
}

int vita_library_scan(int force_rebuild)
{
    (void)force_rebuild;
    s_lib_count = 0;
    s_lib_loaded = 1;
    s_lib_stale = 0;
    return 0;
}

int vita_library_check_stale(void)
{
    return 0;
}

#endif
