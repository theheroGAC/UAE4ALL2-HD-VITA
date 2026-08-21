#ifndef DISK_SOUND_H
#define DISK_SOUND_H

#include "sysconfig.h"
#include "sysdeps.h"

void disk_sound_init(void);
void disk_sound_reset(void);
void disk_sound_set_volume(int percent);
void disk_sound_shutdown(void);
void disk_sound_floppy_step(void);
void disk_sound_floppy_motor(void);
void disk_sound_floppy_read(void);
void disk_sound_floppy_write(void);
void disk_sound_hard_read(void);
void disk_sound_hard_write(void);
void disk_sound_mix(uae_s16 *samples, int frames, int channels, int output_rate);

#endif
