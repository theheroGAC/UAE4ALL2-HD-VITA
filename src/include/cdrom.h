#ifndef CDROM_H
#define CDROM_H

#include <stdio.h>
#include "sysconfig.h"
#include "sysdeps.h"

extern char current_cd_image[256];
extern int cdrom_is_inserted;

int cdrom_open_image(const char *path);
void cdrom_close_image(void);
int cdrom_read_sector(uae_u32 lba, uae_u8 *buffer);
uae_u32 cdrom_get_capacity(void);

#endif
