#ifndef CDROM_H
#define CDROM_H

#include "sysconfig.h"
#include "sysdeps.h"

typedef struct {
    int number;
    int audio;
    uae_u32 start_lba;
    uae_u32 end_lba;
} CdromTrackInfo;

extern char current_cd_image[256];
extern int cdrom_is_inserted;

int cdrom_open_image(const char *path);
void cdrom_close_image(void);
int cdrom_read_sector(uae_u32 lba, uae_u8 *buffer);
int cdrom_read_raw_sector(uae_u32 lba, uae_u8 *buffer);
uae_u32 cdrom_get_capacity(void);
int cdrom_get_track_count(void);
int cdrom_get_track_info(int index, CdromTrackInfo *info);
int cdrom_get_subcode(uae_u32 lba, uae_u8 *buffer);
int cdrom_is_audio_lba(uae_u32 lba);
void cdrom_audio_start(uae_u32 start_lba, uae_u32 end_lba);
void cdrom_audio_pause(int paused);
void cdrom_audio_stop(void);
int cdrom_audio_is_playing(void);
void cdrom_mix_audio(uae_s16 *samples, int frames, int channels, int output_rate);
void cdrom_audio_get_state(uae_u32 *start_lba, uae_u32 *end_lba, uae_u32 *phase, int *playing, int *paused);
void cdrom_audio_set_state(uae_u32 start_lba, uae_u32 end_lba, uae_u32 phase, int playing, int paused);

#endif
