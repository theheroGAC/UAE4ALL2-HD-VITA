#ifndef UAE_LIBRARY_MANAGER_H
#define UAE_LIBRARY_MANAGER_H

#include <stddef.h>

#define VITA_LIBRARY_ROMS_DIR    "ux0:/data/uae4all/roms"
#define VITA_LIBRARY_HOME_DIR    "ux0:/data/uae4all"
#define VITA_LIBRARY_FILE        "ux0:/data/uae4all/library.txt"
#define VITA_LIBRARY_ROOTS_FILE  "ux0:/data/uae4all/library_roots.txt"
#define VITA_LIBRARY_MAX         2048
#define VITA_LIBRARY_MAX_ROOTS   8
#define VITA_LIBRARY_PATH_LEN    512

#define VITA_LIB_KIND_UNKNOWN   (-1)
#define VITA_LIB_KIND_FLOPPY    0
#define VITA_LIB_KIND_HDF       1
#define VITA_LIB_KIND_WHDLOAD   2
#define VITA_LIB_KIND_LHA       3
#define VITA_LIB_KIND_CD        4

typedef struct {
    char path[VITA_LIBRARY_PATH_LEN];
    unsigned long long size;
    int kind;
} VitaLibraryEntry;

int vita_library_scan(int force_rebuild);
int vita_library_preload(void);
int vita_library_count(void);
const VitaLibraryEntry *vita_library_entry(int index);
void vita_library_reset(void);

int vita_library_root_count(void);
const char *vita_library_root(int index);
int vita_library_parse_root_line(const char *line, char *out, size_t out_size);
int vita_library_is_stale(void);
int vita_library_check_stale(void);

int vita_library_kind_from_ext(const char *filename);
int vita_library_kind_label_for(int kind, const char **label);
unsigned long vita_library_zip_cd_offset(const unsigned char *tail, size_t tail_len);
int vita_library_zip_kind(const unsigned char *cd, size_t cd_len);

int vita_library_format_entry(const VitaLibraryEntry *entry, char *out, size_t out_size);
int vita_library_parse_entry(const char *line, VitaLibraryEntry *entry);
int vita_library_dedup_entries(VitaLibraryEntry *entries, int count);
int vita_library_drop_sidecar_bins(VitaLibraryEntry *entries, int count);

#endif
