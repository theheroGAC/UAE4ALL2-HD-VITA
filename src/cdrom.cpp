#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

#include "sysconfig.h"
#include "sysdeps.h"
#include "cdrom.h"

#define CDROM_MAX_TRACKS 100
#define CDROM_MAX_FILES 100
#define CDROM_AUDIO_FRAMES_PER_SECTOR 588
#define CDROM_AUDIO_PHASE_SHIFT 16
#define CDROM_AUDIO_PHASE_ONE (1u << CDROM_AUDIO_PHASE_SHIFT)

struct cdrom_file_entry {
    char path[512];
    int sector_size;
};

struct cdrom_track_entry {
    int number;
    int audio;
    int sector_size;
    int file_index;
    int index0_valid;
    int index1_valid;
    int index0_frames;
    int index1_frames;
    int pregap_frames;
    int postgap_frames;
    uae_u32 start_lba;
    uae_u32 end_lba;
    long file_offset;
};

char current_cd_image[256] = "";
int cdrom_is_inserted = 0;

static FILE *cd_file = NULL;
static struct cdrom_file_entry cd_files[CDROM_MAX_FILES];
static struct cdrom_track_entry cd_tracks[CDROM_MAX_TRACKS];
static int cd_file_count = 0;
static int cd_track_count = 0;
static uae_u32 cd_total_sectors = 0;
static int cd_audio_playing = 0;
static int cd_audio_paused = 0;
static uae_u32 cd_audio_start_lba = 0;
static uae_u32 cd_audio_end_lba = 0;
static uae_u32 cd_audio_phase = 0;
static int cd_audio_sector_lba = -1;
static uae_u8 cd_audio_sector[2352];

static int has_extension(const char *path, const char *extension)
{
    const char *dot = strrchr(path, '.');
    if (!dot) return 0;
    while (*dot && *extension) {
        if (tolower((unsigned char)*dot) != tolower((unsigned char)*extension))
            return 0;
        dot++;
        extension++;
    }
    return *dot == '\0' && *extension == '\0';
}

static void get_directory(const char *path, char *directory, size_t directory_size)
{
    const char *slash;
    size_t length;

    if (!directory || directory_size == 0) return;
    slash = strrchr(path, '/');
#ifdef _WIN32
    {
        const char *backslash = strrchr(path, '\\');
        if (!slash || (backslash && backslash > slash)) slash = backslash;
    }
#endif
    if (!slash) {
        strncpy(directory, ".", directory_size - 1);
        directory[directory_size - 1] = '\0';
        return;
    }
    length = (size_t)(slash - path);
    if (length >= directory_size) length = directory_size - 1;
    memcpy(directory, path, length);
    directory[length] = '\0';
}

static void trim_line(char *line)
{
    size_t length;
    char *start = line;
    while (*start && isspace((unsigned char)*start)) start++;
    if (start != line) memmove(line, start, strlen(start) + 1);
    length = strlen(line);
    while (length > 0 && isspace((unsigned char)line[length - 1]))
        line[--length] = '\0';
}

static int parse_msf(const char *text)
{
    int minutes = 0;
    int seconds = 0;
    int frames = 0;
    if (sscanf(text, "%d:%d:%d", &minutes, &seconds, &frames) != 3)
        return -1;
    if (seconds < 0 || seconds >= 60 || frames < 0 || frames >= 75)
        return -1;
    return (minutes * 60 + seconds) * 75 + frames;
}

static int parse_file_name(const char *line, char *name, size_t name_size)
{
    const char *p = line;
    const char *end;
    size_t length;

    while (*p && !isspace((unsigned char)*p)) p++;
    while (*p && isspace((unsigned char)*p)) p++;
    if (!*p) return 0;
    if (*p == '"') {
        p++;
        end = strchr(p, '"');
        if (!end) return 0;
    } else {
        end = p;
        while (*end && !isspace((unsigned char)*end)) end++;
    }
    length = (size_t)(end - p);
    if (length >= name_size) length = name_size - 1;
    memcpy(name, p, length);
    name[length] = '\0';
    return length > 0;
}

static int add_cue_file(const char *directory, const char *name)
{
    char resolved[512];
    int i;

    if (cd_file_count >= CDROM_MAX_FILES || !name || !name[0]) return -1;
    if (name[0] == '/' || (strlen(name) > 1 && name[1] == ':'))
        snprintf(resolved, sizeof(resolved), "%s", name);
    else
        snprintf(resolved, sizeof(resolved), "%s/%s", directory, name);

    for (i = 0; i < cd_file_count; i++) {
        if (strcmp(cd_files[i].path, resolved) == 0) return i;
    }
    strncpy(cd_files[cd_file_count].path, resolved, sizeof(cd_files[cd_file_count].path) - 1);
    cd_files[cd_file_count].path[sizeof(cd_files[cd_file_count].path) - 1] = '\0';
    cd_files[cd_file_count].sector_size = 2352;
    return cd_file_count++;
}

static int cue_track_sector_size(const char *line, int *audio)
{
    char mode[32];
    int number;
    if (sscanf(line, "TRACK %d %31s", &number, mode) != 2) return 0;
    *audio = strcasecmp(mode, "AUDIO") == 0;
    if (*audio) return 2352;
    if (strcasecmp(mode, "MODE1/2048") == 0 || strcasecmp(mode, "MODE2/2048") == 0)
        return 2048;
    if (strcasecmp(mode, "MODE1/2352") == 0 || strcasecmp(mode, "MODE2/2352") == 0 ||
        strcasecmp(mode, "MODE2/2336") == 0)
        return 2352;
    return 2352;
}

static int open_cue(const char *cue_path)
{
    FILE *cue;
    char line[512];
    char directory[512];
    char name[512];
    int current_file = -1;
    int current_track = -1;
    int i;

    cue = fopen(cue_path, "rb");
    if (!cue) return 0;
    get_directory(cue_path, directory, sizeof(directory));
    cd_file_count = 0;
    cd_track_count = 0;

    while (fgets(line, sizeof(line), cue)) {
        trim_line(line);
        if (strncasecmp(line, "REM", 3) == 0 || line[0] == '\0') continue;
        if (strncasecmp(line, "FILE", 4) == 0 && isspace((unsigned char)line[4])) {
            if (!parse_file_name(line, name, sizeof(name))) continue;
            current_file = add_cue_file(directory, name);
        } else if (strncasecmp(line, "TRACK", 5) == 0 && isspace((unsigned char)line[5])) {
            int audio;
            int sector_size = cue_track_sector_size(line, &audio);
            if (current_file < 0 || cd_track_count >= CDROM_MAX_TRACKS) continue;
            memset(&cd_tracks[cd_track_count], 0, sizeof(cd_tracks[cd_track_count]));
            cd_tracks[cd_track_count].number = 0;
            sscanf(line, "TRACK %d", &cd_tracks[cd_track_count].number);
            cd_tracks[cd_track_count].audio = audio;
            cd_tracks[cd_track_count].sector_size = sector_size;
            cd_tracks[cd_track_count].file_index = current_file;
            cd_files[current_file].sector_size = sector_size;
            current_track = cd_track_count++;
        } else if (current_track >= 0 && strncasecmp(line, "INDEX", 5) == 0) {
            int index;
            const char *time = line + 5;
            while (*time && isspace((unsigned char)*time)) time++;
            if (sscanf(time, "%d", &index) != 1) continue;
            while (*time && !isspace((unsigned char)*time)) time++;
            while (*time && isspace((unsigned char)*time)) time++;
            int frames = parse_msf(time);
            if (frames < 0) continue;
            if (index == 0) {
                cd_tracks[current_track].index0_valid = 1;
                cd_tracks[current_track].index0_frames = frames;
            } else if (index == 1) {
                cd_tracks[current_track].index1_valid = 1;
                cd_tracks[current_track].index1_frames = frames;
            }
        } else if (current_track >= 0 && strncasecmp(line, "PREGAP", 6) == 0) {
            const char *time = line + 6;
            while (*time && isspace((unsigned char)*time)) time++;
            cd_tracks[current_track].pregap_frames = parse_msf(time);
        } else if (current_track >= 0 && strncasecmp(line, "POSTGAP", 7) == 0) {
            const char *time = line + 7;
            while (*time && isspace((unsigned char)*time)) time++;
            cd_tracks[current_track].postgap_frames = parse_msf(time);
        }
    }
    fclose(cue);

    if (cd_track_count <= 0) return 0;
    for (i = 0; i < cd_track_count; i++) {
        struct cdrom_track_entry *track = &cd_tracks[i];
        int frames = track->index1_valid ? track->index1_frames : track->index0_frames;
        if (!track->index1_valid && !track->index0_valid) frames = 0;
        track->file_offset = (long)frames * track->sector_size;
        if (i == 0) {
            track->start_lba = 0;
        } else if (track->file_index == cd_tracks[i - 1].file_index) {
            int previous_frames = cd_tracks[i - 1].index1_valid ? cd_tracks[i - 1].index1_frames : cd_tracks[i - 1].index0_frames;
            int delta = frames - previous_frames;
            if (delta < 0) delta = 0;
            track->start_lba = cd_tracks[i - 1].start_lba + (uae_u32)delta;
        } else {
            track->start_lba = cd_tracks[i - 1].end_lba;
        }
        if (track->pregap_frames > 0)
            track->start_lba += (uae_u32)track->pregap_frames;
        if (i + 1 < cd_track_count && cd_tracks[i + 1].file_index == track->file_index) {
            int next_frames = cd_tracks[i + 1].index1_valid ? cd_tracks[i + 1].index1_frames : cd_tracks[i + 1].index0_frames;
            int length = next_frames - frames;
            if (length < 0) length = 0;
            track->end_lba = track->start_lba + (uae_u32)length;
        } else {
            FILE *data = fopen(cd_files[track->file_index].path, "rb");
            long size = 0;
            if (data) {
                fseek(data, 0, SEEK_END);
                size = ftell(data);
                fclose(data);
            }
            if (size < track->file_offset)
                track->end_lba = track->start_lba;
            else
                track->end_lba = track->start_lba + (uae_u32)((size - track->file_offset) / track->sector_size);
        }
        if (track->end_lba < track->start_lba)
            track->end_lba = track->start_lba;
    }

    cd_file = fopen(cd_files[cd_tracks[0].file_index].path, "rb");
    return cd_file != NULL;
}

static int open_plain_image(const char *path)
{
    cd_file_count = 1;
    cd_track_count = 1;
    strncpy(cd_files[0].path, path, sizeof(cd_files[0].path) - 1);
    cd_files[0].path[sizeof(cd_files[0].path) - 1] = '\0';
    cd_files[0].sector_size = has_extension(path, ".iso") ? 2048 : 2352;
    memset(&cd_tracks[0], 0, sizeof(cd_tracks[0]));
    cd_tracks[0].number = 1;
    cd_tracks[0].audio = 0;
    cd_tracks[0].sector_size = cd_files[0].sector_size;
    cd_tracks[0].file_index = 0;
    cd_tracks[0].start_lba = 0;
    cd_tracks[0].file_offset = 0;
    cd_file = fopen(path, "rb");
    if (!cd_file) return 0;
    fseek(cd_file, 0, SEEK_END);
    cd_tracks[0].end_lba = (uae_u32)(ftell(cd_file) / cd_tracks[0].sector_size);
    return 1;
}

static struct cdrom_track_entry *find_track(uae_u32 lba)
{
    int i;
    for (i = 0; i < cd_track_count; i++) {
        if (lba >= cd_tracks[i].start_lba && lba < cd_tracks[i].end_lba)
            return &cd_tracks[i];
    }
    return NULL;
}

static int read_track_raw(struct cdrom_track_entry *track, uae_u32 lba, uae_u8 *buffer)
{
    FILE *file;
    long offset;
    size_t read_bytes;

    if (!track || !buffer || lba < track->start_lba || lba >= track->end_lba)
        return 0;
    file = cd_file;
    if (!file) return 0;
    if (track->file_index != 0 || cd_file_count > 1) {
        file = fopen(cd_files[track->file_index].path, "rb");
        if (!file) return 0;
    }
    offset = track->file_offset + (long)(lba - track->start_lba) * track->sector_size;
    if (fseek(file, offset, SEEK_SET) != 0) {
        if (file != cd_file) fclose(file);
        return 0;
    }
    if (track->sector_size == 2352) {
        read_bytes = fread(buffer, 1, 2352, file);
    } else {
        memset(buffer, 0, 2352);
        read_bytes = fread(buffer + 16, 1, 2048, file);
    }
    if (file != cd_file) fclose(file);
    return track->sector_size == 2352 ? read_bytes == 2352 : read_bytes == 2048;
}

static uae_u8 to_bcd(int value)
{
    return (uae_u8)(((value / 10) << 4) | (value % 10));
}

int cdrom_open_image(const char *path)
{
    int i;

    cdrom_close_image();
    if (!path || path[0] == '\0') return 0;
    if (has_extension(path, ".cue")) {
        if (!open_cue(path)) {
            cdrom_close_image();
            return 0;
        }
    } else if (!open_plain_image(path)) {
        cdrom_close_image();
        return 0;
    }

    cd_total_sectors = 0;
    for (i = 0; i < cd_track_count; i++) {
        if (cd_tracks[i].end_lba > cd_total_sectors)
            cd_total_sectors = cd_tracks[i].end_lba;
    }
    strncpy(current_cd_image, path, sizeof(current_cd_image) - 1);
    current_cd_image[sizeof(current_cd_image) - 1] = '\0';
    cdrom_is_inserted = cd_total_sectors > 0;
    return cdrom_is_inserted;
}

void cdrom_close_image(void)
{
    if (cd_file) {
        fclose(cd_file);
        cd_file = NULL;
    }
    current_cd_image[0] = '\0';
    cdrom_is_inserted = 0;
    cd_file_count = 0;
    cd_track_count = 0;
    cd_total_sectors = 0;
    cdrom_audio_stop();
}

int cdrom_read_raw_sector(uae_u32 lba, uae_u8 *buffer)
{
    return read_track_raw(find_track(lba), lba, buffer);
}

int cdrom_read_sector(uae_u32 lba, uae_u8 *buffer)
{
    struct cdrom_track_entry *track = find_track(lba);
    if (!track || track->audio) return 0;
    if (!buffer) return 0;
    if (track->sector_size == 2048) {
        FILE *file = cd_file;
        long offset = track->file_offset + (long)(lba - track->start_lba) * 2048;
        size_t bytes;
        if (track->file_index != 0 || cd_file_count > 1) {
            file = fopen(cd_files[track->file_index].path, "rb");
            if (!file) return 0;
        }
        if (fseek(file, offset, SEEK_SET) != 0) {
            if (file != cd_file) fclose(file);
            return 0;
        }
        bytes = fread(buffer, 1, 2048, file);
        if (file != cd_file) fclose(file);
        return bytes == 2048;
    }
    if (!read_track_raw(track, lba, cd_audio_sector)) return 0;
    memcpy(buffer, cd_audio_sector + 16, 2048);
    return 1;
}

uae_u32 cdrom_get_capacity(void)
{
    return cd_total_sectors;
}

int cdrom_get_track_count(void)
{
    return cd_track_count;
}

int cdrom_get_track_info(int index, CdromTrackInfo *info)
{
    if (!info || index < 0 || index >= cd_track_count) return 0;
    info->number = cd_tracks[index].number;
    info->audio = cd_tracks[index].audio;
    info->start_lba = cd_tracks[index].start_lba;
    info->end_lba = cd_tracks[index].end_lba;
    return 1;
}

int cdrom_get_subcode(uae_u32 lba, uae_u8 *buffer)
{
    struct cdrom_track_entry *track = find_track(lba);
    int absolute;
    int relative;
    int minutes;
    int seconds;
    int frames;
    if (!track || !buffer) return 0;
    memset(buffer, 0, 12);
    buffer[0] = (uae_u8)(track->audio ? 0x01 : 0x41);
    buffer[1] = to_bcd(track->number);
    buffer[2] = 1;
    relative = (int)(lba - track->start_lba);
    absolute = (int)lba + 150;
    minutes = relative / (60 * 75);
    seconds = (relative / 75) % 60;
    frames = relative % 75;
    buffer[3] = to_bcd(minutes);
    buffer[4] = to_bcd(seconds);
    buffer[5] = to_bcd(frames);
    minutes = absolute / (60 * 75);
    seconds = (absolute / 75) % 60;
    frames = absolute % 75;
    buffer[7] = to_bcd(minutes);
    buffer[8] = to_bcd(seconds);
    buffer[9] = to_bcd(frames);
    return 1;
}

int cdrom_is_audio_lba(uae_u32 lba)
{
    struct cdrom_track_entry *track = find_track(lba);
    return track && track->audio;
}

void cdrom_audio_start(uae_u32 start_lba, uae_u32 end_lba)
{
    struct cdrom_track_entry *track = find_track(start_lba);
    if (!track || !track->audio) {
        cdrom_audio_stop();
        return;
    }
    if (end_lba <= start_lba || end_lba > track->end_lba)
        end_lba = track->end_lba;
    cd_audio_start_lba = start_lba;
    cd_audio_end_lba = end_lba;
    cd_audio_phase = 0;
    cd_audio_sector_lba = -1;
    cd_audio_paused = 0;
    cd_audio_playing = 1;
}

void cdrom_audio_pause(int paused)
{
    if (cd_audio_playing) cd_audio_paused = paused != 0;
}

void cdrom_audio_stop(void)
{
    cd_audio_playing = 0;
    cd_audio_paused = 0;
    cd_audio_phase = 0;
    cd_audio_sector_lba = -1;
}

int cdrom_audio_is_playing(void)
{
    return cd_audio_playing && !cd_audio_paused;
}

void cdrom_audio_get_state(uae_u32 *start_lba, uae_u32 *end_lba, uae_u32 *phase, int *playing, int *paused)
{
    if (start_lba) *start_lba = cd_audio_start_lba;
    if (end_lba) *end_lba = cd_audio_end_lba;
    if (phase) *phase = cd_audio_phase;
    if (playing) *playing = cd_audio_playing;
    if (paused) *paused = cd_audio_paused;
}

void cdrom_audio_set_state(uae_u32 start_lba, uae_u32 end_lba, uae_u32 phase, int playing, int paused)
{
    cd_audio_start_lba = start_lba;
    cd_audio_end_lba = end_lba;
    cd_audio_phase = phase;
    cd_audio_sector_lba = -1;
    cd_audio_paused = paused != 0;
    cd_audio_playing = playing != 0 && cdrom_is_inserted;
}

void cdrom_mix_audio(uae_s16 *samples, int frames, int channels, int output_rate)
{
    uae_u32 increment;
    int i;
    if (!samples || frames <= 0 || channels <= 0 || output_rate <= 0 || !cd_audio_playing || cd_audio_paused)
        return;
    increment = (uae_u32)(((uae_u64)44100 * CDROM_AUDIO_PHASE_ONE) / (uae_u32)output_rate);
    if (increment == 0) increment = 1;
    for (i = 0; i < frames && cd_audio_playing; i++) {
        uae_u32 source_frame = cd_audio_phase >> CDROM_AUDIO_PHASE_SHIFT;
        uae_u32 lba = cd_audio_start_lba + source_frame / CDROM_AUDIO_FRAMES_PER_SECTOR;
        int frame = (int)(source_frame % CDROM_AUDIO_FRAMES_PER_SECTOR);
        int left;
        int right;
        int offset;
        if (lba >= cd_audio_end_lba) {
            cd_audio_playing = 0;
            break;
        }
        if (cd_audio_sector_lba != (int)lba) {
            if (!cdrom_read_raw_sector(lba, cd_audio_sector)) {
                cd_audio_playing = 0;
                break;
            }
            cd_audio_sector_lba = (int)lba;
        }
        offset = frame * 4;
        left = (short)(cd_audio_sector[offset] | (cd_audio_sector[offset + 1] << 8));
        right = (short)(cd_audio_sector[offset + 2] | (cd_audio_sector[offset + 3] << 8));
        if (channels == 1) {
            int value = ((left + right) / 2) / 2;
            int mixed = samples[i] + value;
            if (mixed > 32767) mixed = 32767;
            if (mixed < -32768) mixed = -32768;
            samples[i] = (uae_s16)mixed;
        } else {
            int mixed_left = samples[i * channels] + left / 2;
            int mixed_right = samples[i * channels + 1] + right / 2;
            if (mixed_left > 32767) mixed_left = 32767;
            if (mixed_left < -32768) mixed_left = -32768;
            if (mixed_right > 32767) mixed_right = 32767;
            if (mixed_right < -32768) mixed_right = -32768;
            samples[i * channels] = (uae_s16)mixed_left;
            samples[i * channels + 1] = (uae_s16)mixed_right;
        }
        cd_audio_phase += increment;
    }
}
