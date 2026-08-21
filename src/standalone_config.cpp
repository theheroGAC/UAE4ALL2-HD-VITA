#include "standalone_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "sysconfig.h"
#include "sysdeps.h"
#include "config.h"
#include "options.h"
#include "gui.h"
#include "menu.h"
#include "menu_config.h"
#include "zfile.h"

extern SDL_Surface *prSDLScreen;
extern char romfile[256];
extern int kickstart;
extern char uae4all_image_file0[256];
extern char changed_df[4][256];
extern char prefs_df[4][256];
extern int real_changed_df[4];
extern int mainMenu_drives;
extern int mainMenu_chipMemory;
extern int mainMenu_chipset;
extern int mainMenu_CPU_model;

bool FileExists(const char *path)
{
    if (!path || path[0] == '\0') return false;
    FILE *f = fopen(path, "rb");
    if (f) {
        fclose(f);
        return true;
    }
    return false;
}

void ShowErrorAndExit(const char *line1, const char *line2, const char *line3, const char *line4)
{
    // Inizializza visualizzazione testo SDL
    init_text(0);

    // Disegna lo sfondo e la finestra di dialogo
    text_draw_background();
    text_draw_window(2, 4, 42, 14, "--- AMIGA LAUNCHER ERROR ---");

    if (line1) write_text(4, 7, (char *)line1);
    if (line2) write_text(4, 9, (char *)line2);
    if (line3) write_text(4, 11, (char *)line3);
    if (line4) write_text(4, 13, (char *)line4);

    text_flip();

    // Attesa non bloccante di 5 secondi (5000 ms), gestendo eventuali eventi di quit
    Uint32 start_ticks = SDL_GetTicks();
    SDL_Event ev;
    while (SDL_GetTicks() - start_ticks < 5000) {
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) {
                quit_text();
                SDL_Quit();
                exit(0);
            }
        }
        SDL_Delay(20);
    }

    quit_text();
    SDL_Quit();
    exit(0);
}

bool Standalone_CheckBootFiles(void)
{
    // 1. Controllo Kickstart 3.x
    if (!FileExists(PATH_KICK3)) {
        ShowErrorAndExit(
            "Missing kick3.rom",
            "Copy your Kickstart ROM to:",
            GAME_PATH,
            "The application will exit in 5 seconds."
        );
        return false;
    }

    // 2. Controllo Disco 1
    if (!FileExists(PATH_DISK1)) {
        ShowErrorAndExit(
            "Missing disk1.adf",
            "Copy your disk image to:",
            GAME_PATH,
            "The application will exit in 5 seconds."
        );
        return false;
    }

    // 3. Controllo Disco 2
    if (!FileExists(PATH_DISK2)) {
        ShowErrorAndExit(
            "Missing disk2.adf",
            "Copy your disk image to:",
            GAME_PATH,
            "The application will exit in 5 seconds."
        );
        return false;
    }

    return true;
}

void Standalone_ConfigureEmulator(void)
{
    // Imposta Kickstart 3.1
    kickstart = 3; // Indice 3 in UAE4All2 = Kickstart 3.1
    snprintf(romfile, sizeof(romfile), "%s", PATH_KICK3);
    uae4all_init_rom(romfile);

    // Configurazione hardware Amiga standard compatibile con KS 3.x
    mainMenu_drives = 1; // 1 Floppy drive fisico (DF0)
    mainMenu_chipMemory = 2; // 2MB Chip RAM
    mainMenu_CPU_model = 1;  // 68020
    
    // Inserimento Disk 1 in DF0
    strncpy(uae4all_image_file0, PATH_DISK1, 255);
    uae4all_image_file0[255] = '\0';

    strncpy(prefs_df[0], PATH_DISK1, 255);
    prefs_df[0][255] = '\0';

    strncpy(changed_df[0], PATH_DISK1, 255);
    changed_df[0][255] = '\0';

    real_changed_df[0] = 1;
}
