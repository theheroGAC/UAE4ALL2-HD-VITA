#include "disk_swap.h"
#include "standalone_config.h"
#include "osd.h"

#include <stdio.h>
#include <string.h>

#include "sysconfig.h"
#include "sysdeps.h"
#include "config.h"
#include "options.h"
#include "disk.h"
#include "gui.h"

extern char uae4all_image_file0[256];
extern char changed_df[4][256];
extern char prefs_df[4][256];
extern int real_changed_df[4];

static int g_current_disk = 1;

int GetCurrentDisk(void)
{
    return g_current_disk;
}

void SwapDisk(void)
{
    int target_disk = (g_current_disk == 1) ? 2 : 1;
    const char *target_path = (target_disk == 1) ? PATH_DISK1 : PATH_DISK2;

    // Controllo esistenza prima dello swap
    if (!FileExists(target_path)) {
        // Se il dischetto non esiste, manteniamo il disco corrente e mostriamo un OSD di allerta
        OSD_TriggerDiskSwap(g_current_disk, true);
        return;
    }

    // Eseguiamo l'inserimento nel drive DF0 (indice 0) con la funzione nativa di UAE4All2
    strncpy(changed_df[0], target_path, 255);
    changed_df[0][255] = '\0';

    strncpy(uae4all_image_file0, target_path, 255);
    uae4all_image_file0[255] = '\0';

    disk_insert(0, target_path);
    real_changed_df[0] = 1;

    g_current_disk = target_disk;

    // Attiva OSD per 2500ms
    OSD_TriggerDiskSwap(g_current_disk, false);
}
