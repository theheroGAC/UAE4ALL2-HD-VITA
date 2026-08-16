#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sysconfig.h"
#include "sysdeps.h"
#include "cdrom.h"

char current_cd_image[256] = "";
int cdrom_is_inserted = 0;
static FILE *cd_file = NULL;
static uae_u32 cd_total_sectors = 0;

int cdrom_open_image(const char *path)
{
    cdrom_close_image();
    if (!path || strlen(path) == 0) {
        return 0;
    }
    cd_file = fopen(path, "rb");
    if (!cd_file) {
        return 0;
    }
    strncpy(current_cd_image, path, 255);
    current_cd_image[255] = 0;
    fseek(cd_file, 0, SEEK_END);
    long size = ftell(cd_file);
    fseek(cd_file, 0, SEEK_SET);
    cd_total_sectors = (uae_u32)(size / 2048);
    cdrom_is_inserted = 1;
    return 1;
}

void cdrom_close_image(void)
{
    if (cd_file) {
        fclose(cd_file);
        cd_file = NULL;
    }
    current_cd_image[0] = 0;
    cdrom_is_inserted = 0;
    cd_total_sectors = 0;
}

int cdrom_read_sector(uae_u32 lba, uae_u8 *buffer)
{
    if (!cd_file || lba >= cd_total_sectors || !buffer) {
        return 0;
    }
    fseek(cd_file, (long)lba * 2048, SEEK_SET);
    size_t read_bytes = fread(buffer, 1, 2048, cd_file);
    return (read_bytes == 2048) ? 1 : 0;
}

uae_u32 cdrom_get_capacity(void)
{
    return cd_total_sectors;
}
