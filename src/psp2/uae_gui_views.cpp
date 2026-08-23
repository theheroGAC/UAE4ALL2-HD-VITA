#ifdef __PSP2__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sysconfig.h"
#include "sysdeps.h"
#include "config.h"
#include "options.h"
#include "uae.h"
#include "sound.h"
#include "disk.h"
#include "memory-uae.h"
#include "custom.h"
#include "xwin.h"
#include "drawing.h"
#include "menu.h"
#include "menu_config.h"
#include "savestate.h"
#include "gui.h"
#include "cfgfile.h"
#include "cdrom.h"
#include "whdload_manager.h"
#include "hdf_manager.h"
#include "midi_synth.h"
#include "cover_downloader.h"
#include <SDL_image.h>
#include <psp2/io/stat.h>

#include "uae_gui_vita.h"

extern char uae4all_image_file0[256];
extern char uae4all_image_file1[256];
extern char uae4all_image_file2[256];
extern char uae4all_image_file3[256];
extern char uae4all_hard_file0[256];
extern char uae4all_hard_file1[256];
extern char uae4all_hard_file2[256];
extern char uae4all_hard_file3[256];
extern char uae4all_hard_dir[256];
extern int uae4all_hard_file_ro[4];
extern int mainMenu_bootHD;
extern char currentDir[300];
extern char launchDir[300];
extern int mainMenu_drives;
extern int mainMenu_floppyspeed;
extern int mainMenu_CPU_model;
extern int mainMenu_chipset;
extern int mainMenu_chipMemory;
extern int mainMenu_slowMemory;
extern int mainMenu_fastMemory;
extern int kickstart;
extern int bReloadKickstart;
extern int uae4all_init_rom(const char *romfile);
extern const char *kickstarts_rom_names[];
extern const char *extended_rom_names[];
extern char romfile[256];
extern char extfile[256];
extern int mainMenu_shader;
extern int mainMenu_ntsc;
extern int mainMenu_showStatus;
extern int mainMenu_leftStickMouse;
extern int mainMenu_touchControls;
extern int mainMenu_autofire;
extern int mainMenu_pinballMode;
extern int moveY;
extern int mainMenu_soundStereo;
extern int mainMenu_soundStereoSep;
extern int mainMenu_diskSoundVolume;
extern void disk_sound_set_volume(int percent);
extern unsigned int sound_rate;
extern int saveMenu_n_savestate;
extern char *savestate_filename;
extern char *screenshot_filename;
extern int savestate_state;
extern int presetModeId;
extern int mainMenu_case;
extern int emulating;
extern int kickstart_warning;
extern volatile int vita_screenshot_request;

static void copy_drive_path(char *destination, const char *source)
{
    if (!destination) return;
    if (!source) {
        destination[0] = '\0';
        return;
    }
    strncpy(destination, source, 255);
    destination[255] = '\0';
}

static const char *get_filename_only(const char *path)
{
    if (!path || path[0] == '\0') return "Empty (Press X to insert)";
    const char *p = strrchr(path, '/');
    if (p) return p + 1;
    p = strrchr(path, '\\');
    if (p) return p + 1;
    return path;
}

static int vita_load_kickstart(const char *path)
{
    if (uae4all_init_rom(path) == -1) {
        kickstart_warning = 1;
        write_log("[VITA] Kickstart load failed: %s\n", path ? path : "");
        vita_show_message_box("Kickstart Missing", "The selected Kickstart ROM could not be opened.", "OK (X)");
        return 0;
    }
    kickstart_warning = 0;
    write_log("[VITA] Kickstart loaded: %s\n", path ? path : "");
    return 1;
}

static const char *vita_kickstart_aliases[KICKSTART_ROM_COUNT][8] = {
    { "kick12.rom", "kick33180.A500", "amiga-os-120.rom", "kick1.2.rom", NULL },
    { "kick13.rom", "kick34005.A500", "amiga-os-130.rom", "kick1.3.rom", NULL },
    { "kick20.rom", "kick37175.A500", "amiga-os-204.rom", "kick204.rom", "kick2.04.rom", NULL },
    { "kick31.rom", "kick40068.A1200", "amiga-os-310-a1200.rom", "kick3.1.rom", NULL },
    { "kickcustom.rom", "custom.rom", NULL },
    { "aros-amiga-m68k-rom.bin", "aros.rom", NULL },
    { "kick40060.CD32", "amiga-os-310-cd32.rom", "kick31.rom", NULL },
    { "kick31034.A1000", "amiga-os-110-ntsc.rom", NULL },
    { "kick32034.A1000", "amiga-os-110-pal.rom", NULL },
    { "kick33180.A500", "amiga-os-120.rom", "kick12.rom", "kick1.2.rom", NULL },
    { "kick37175.A500", "amiga-os-204.rom", "kick20.rom", "kick204.rom", NULL },
    { "kick37350.A600", "amiga-os-205-a600.rom", "kick205.rom", "kick2.05.rom", "kick600.rom", "kick20.rom", NULL },
    { "kick40063.A600", "amiga-os-310-a600.rom", "kick31.rom", "kick40068.A1200", NULL },
    { "kick39106.A1200", "amiga-os-300-a1200.rom", "kick30.rom", "kick3.0.rom", NULL },
    { "kick40068.A1200", "amiga-os-310-a1200.rom", "kick31.rom", "kick3.1.rom", NULL },
    { "kick39106.A4000", "amiga-os-300-a4000.rom", "kick30.rom", NULL },
    { "kick40068.A4000", "amiga-os-310-a4000.rom", "kick31.rom", NULL },
    { "kick13.rom", "amiga-os-130.rom", "kick34005.A500", "kick1.3.rom", NULL }
};

int vita_set_kickstart(int index, int load_rom)
{
    if (index < 0 || index >= KICKSTART_ROM_COUNT)
        return 0;

    int found = 0;
    romfile[0] = '\0';
    for (int i = 0; i < 8 && vita_kickstart_aliases[index][i]; i++) {
        char candidate[256];
        snprintf(candidate, sizeof(candidate), "%s/kickstarts/%s", launchDir, vita_kickstart_aliases[index][i]);
        FILE *file = fopen(candidate, "rb");
        if (file) {
            fclose(file);
            strncpy(romfile, candidate, sizeof(romfile) - 1);
            romfile[sizeof(romfile) - 1] = '\0';
            found = 1;
            break;
        }
    }
    if (!found)
        snprintf(romfile, sizeof(romfile), "%s/kickstarts/%s", launchDir, kickstarts_rom_names[index]);

    if (extended_rom_names[index][0] != '\0')
        snprintf(extfile, sizeof(extfile), "%s/kickstarts/%s", launchDir, extended_rom_names[index]);
    else
        extfile[0] = '\0';

    if (!found) {
        kickstart_warning = 1;
        write_log("[VITA] Kickstart unavailable: %s\n", romfile);
        return 0;
    }
    if (load_rom)
        return vita_load_kickstart(romfile);
    kickstart_warning = 0;
    return 1;
}

void vita_view_floppy(VitaInputState *input, int *selected_item)
{
    const int total_items = 6;
    if (*selected_item < 0) *selected_item = 0;
    if (*selected_item >= total_items) *selected_item = total_items - 1;

    if (input->pressed & SCE_CTRL_UP) {
        (*selected_item)--;
        if (*selected_item < 0) *selected_item = total_items - 1;
    }
    if (input->pressed & SCE_CTRL_DOWN) {
        (*selected_item)++;
        if (*selected_item >= total_items) *selected_item = 0;
    }

    if (input->pressed & SCE_CTRL_CROSS) {
        if (*selected_item >= 0 && *selected_item < 4) {
            if (mainMenu_drives < *selected_item + 1)
                mainMenu_drives = *selected_item + 1;

            char new_file[512];
            new_file[0] = '\0';
            int res = vita_gui_run_browser(new_file, currentDir, *selected_item);
            if (res == 1) {
                write_log("[VITA] floppy: selected DF%d path=%s\n", *selected_item, new_file);
                if (*selected_item == 0) copy_drive_path(uae4all_image_file0, new_file);
                if (*selected_item == 1) copy_drive_path(uae4all_image_file1, new_file);
                if (*selected_item == 2) copy_drive_path(uae4all_image_file2, new_file);
                if (*selected_item == 3) copy_drive_path(uae4all_image_file3, new_file);
                write_log("[VITA] floppy: updating emulator paths\n");
                gui_update();
                write_log("[VITA] floppy: emulator paths updated\n");
            } else if (res == 2) {
                if (*selected_item == 0) uae4all_image_file0[0] = '\0';
                if (*selected_item == 1) uae4all_image_file1[0] = '\0';
                if (*selected_item == 2) uae4all_image_file2[0] = '\0';
                if (*selected_item == 3) uae4all_image_file3[0] = '\0';
                gui_update();
            }
        } else if (*selected_item == 4) {
            if (mainMenu_floppyspeed == 100) mainMenu_floppyspeed = 200;
            else if (mainMenu_floppyspeed == 200) mainMenu_floppyspeed = 400;
            else if (mainMenu_floppyspeed == 400) mainMenu_floppyspeed = 800;
            else mainMenu_floppyspeed = 100;
        } else if (*selected_item == 5) {
            uae4all_image_file0[0] = '\0';
            uae4all_image_file1[0] = '\0';
            uae4all_image_file2[0] = '\0';
            uae4all_image_file3[0] = '\0';
            gui_update();
        }
    }

    if (input->pressed & SCE_CTRL_TRIANGLE) {
        if (*selected_item == 0) uae4all_image_file0[0] = '\0';
        if (*selected_item == 1) uae4all_image_file1[0] = '\0';
        if (*selected_item == 2) uae4all_image_file2[0] = '\0';
        if (*selected_item == 3) uae4all_image_file3[0] = '\0';
        gui_update();
    }

    if (input->pressed & SCE_CTRL_SQUARE) {
        mainMenu_case = MAIN_MENU_CASE_RESET;
    }

    float card_x = 20.0f;
    float card_w = VITA_SCREEN_W - 40.0f;
    float start_y = 90.0f;
    float slot_h = 56.0f;

    char *drive_files[4] = { uae4all_image_file0, uae4all_image_file1, uae4all_image_file2, uae4all_image_file3 };
    const char *drive_labels[4] = { "DF0: Internal Floppy Drive", "DF1: External Floppy Drive 1", "DF2: External Floppy Drive 2", "DF3: External Floppy Drive 3" };

    for (int i = 0; i < 4; i++) {
        float cy = start_y + (float)i * (slot_h + 8.0f);
        bool focused = (*selected_item == i);
        bool has_disk = (drive_files[i][0] != '\0');

        vita_draw_card(card_x, cy, card_w, slot_h, focused, false);

        char drive_tag[8];
        snprintf(drive_tag, sizeof(drive_tag), "DF%d", i);
        vita_draw_badge(card_x + 14.0f, cy + 18.0f, drive_tag, has_disk ? VITA_COLOR_AMIGA_RED : RGBA8(40, 50, 70, 255), VITA_COLOR_TEXT_WHITE);

        vita_draw_text(card_x + 64.0f, cy + 10.0f, VITA_COLOR_TEXT_MUTED, 0.85f, drive_labels[i]);

        char filename_buf[128];
        vita_truncate_text(get_filename_only(drive_files[i]), card_w - 200.0f, 1.00f, filename_buf, sizeof(filename_buf));
        unsigned int file_col = has_disk ? (focused ? VITA_COLOR_TEXT_WHITE : RGBA8(230, 240, 255, 255)) : VITA_COLOR_TEXT_DIM;
        vita_draw_text(card_x + 64.0f, cy + 28.0f, file_col, 1.00f, filename_buf);

        vita_draw_led(card_x + card_w - 110.0f, cy + 18.0f, has_disk ? "LOADED" : "EMPTY", has_disk, VITA_COLOR_AMIGA_ORANGE);
    }

    float spd_y = start_y + 4.0f * (slot_h + 8.0f);
    char spd_str[32];
    snprintf(spd_str, sizeof(spd_str), "%dx (%s)", mainMenu_floppyspeed / 100, (mainMenu_floppyspeed == 100) ? "1x Standard" : "Turbo");
    vita_draw_selector_item(card_x, spd_y, (card_w * 0.5f) - 6.0f, 44.0f, "Floppy Speed", spd_str, *selected_item == 4);

    vita_draw_button_item(card_x + (card_w * 0.5f) + 6.0f, spd_y, (card_w * 0.5f) - 6.0f, 44.0f, "Eject All Disks", NULL, "EJECT", *selected_item == 5, false);
}

static int s_hdf_mgr_slot = -1;
static int s_hdf_mgr_item = 0;
static int s_hdf_mgr_analyzed = 0;
static HdfInfo s_hdf_mgr_info;
static int s_hdf_mgr_creating = 0;
static int s_hdf_mgr_create_idx = 3;

static const int s_hdf_create_sizes[] = { 32, 64, 128, 256, 512, 1024, 2048 };
#define HDF_CREATE_SIZE_COUNT ((int)(sizeof(s_hdf_create_sizes) / sizeof(s_hdf_create_sizes[0])))

static void hdf_create_blank_into_slot(int slot)
{
    char path[512];
    int size_mb = s_hdf_create_sizes[s_hdf_mgr_create_idx];

    int n = 1;
    for (;;) {
        if (n == 1)
            snprintf(path, sizeof(path), "ux0:/data/uae4all/blank_%dMB.hdf", size_mb);
        else
            snprintf(path, sizeof(path), "ux0:/data/uae4all/blank_%dMB_%d.hdf", size_mb, n);
        FILE *test = fopen(path, "rb");
        if (!test)
            break;
        fclose(test);
        n++;
    }

    char err[256];
    err[0] = '\0';
    if (hdf_create_blank(path, (unsigned long)size_mb, err, sizeof(err)) != 0) {
        vita_show_message_box("HDF Creation Failed", err[0] ? err : "Unknown error", "OK (X)");
        return;
    }

    char *hdf_files[4] = { uae4all_hard_file0, uae4all_hard_file1, uae4all_hard_file2, uae4all_hard_file3 };
    strncpy(hdf_files[slot], path, 255);
    hdf_files[slot][255] = '\0';
    uae4all_hard_file_ro[slot] = 0;
    make_hard_file_cfg_line(hdf_files[slot]);
    mainMenu_whdload_game[0] = '\0';
    ApplyAutomaticGamePreset(1);
    gui_update();

    char msg[320];
    snprintf(msg, sizeof(msg),
        "Blank %d MB HDF created and mounted in slot %d.\nFormat it from Workbench (e.g. via HD Toolbox) before use.",
        size_mb, slot + 1);
    vita_show_message_box("Blank HDF Created", msg, "OK (X)");
}

void vita_view_hdf_manager(VitaInputState *input, int *selected_item)
{
    const int slot = s_hdf_mgr_slot;
    const int total_items = 5;

    char *hdf_files[4] = {
        uae4all_hard_file0, uae4all_hard_file1,
        uae4all_hard_file2, uae4all_hard_file3
    };

    if (slot < 0 || slot >= 4) {
        s_hdf_mgr_slot = -1;
        return;
    }

    if (!s_hdf_mgr_analyzed) {
        if (hdf_files[slot][0] == '\0') {
            memset(&s_hdf_mgr_info, 0, sizeof(s_hdf_mgr_info));
            s_hdf_mgr_info.valid = 0;
            strcpy(s_hdf_mgr_info.error, "No HDF mounted in this slot");
        } else {
            hdf_analyze(hdf_files[slot], &s_hdf_mgr_info);
        }
        s_hdf_mgr_analyzed = 1;
    }

    if (s_hdf_mgr_creating) {
        if (s_hdf_mgr_create_idx < 0) s_hdf_mgr_create_idx = 0;
        if (s_hdf_mgr_create_idx >= HDF_CREATE_SIZE_COUNT) s_hdf_mgr_create_idx = HDF_CREATE_SIZE_COUNT - 1;

        if (input->pressed & SCE_CTRL_UP) {
            s_hdf_mgr_create_idx -= 2;
            if (s_hdf_mgr_create_idx < 0) s_hdf_mgr_create_idx = (s_hdf_mgr_create_idx + HDF_CREATE_SIZE_COUNT + 1) % HDF_CREATE_SIZE_COUNT;
        }
        if (input->pressed & SCE_CTRL_DOWN) {
            s_hdf_mgr_create_idx += 2;
            if (s_hdf_mgr_create_idx >= HDF_CREATE_SIZE_COUNT) s_hdf_mgr_create_idx = (s_hdf_mgr_create_idx % 2 == 0) ? 0 : 1;
        }
        if (input->pressed & (SCE_CTRL_LEFT | SCE_CTRL_RIGHT)) {
            if (s_hdf_mgr_create_idx % 2 == 0) {
                if (s_hdf_mgr_create_idx + 1 < HDF_CREATE_SIZE_COUNT) s_hdf_mgr_create_idx++;
            } else {
                s_hdf_mgr_create_idx--;
            }
        }
        if (input->pressed & SCE_CTRL_CIRCLE) {
            s_hdf_mgr_creating = 0;
            return;
        }
        if (input->pressed & SCE_CTRL_CROSS) {
            s_hdf_mgr_creating = 0;
            hdf_create_blank_into_slot(slot);
            return;
        }

        float bx = 220.0f, by = 100.0f, bw = 520.0f, bh = 280.0f;
        vita_draw_rounded_rect(0.0f, 0.0f, (float)VITA_SCREEN_W, (float)VITA_SCREEN_H, 0.0f, VITA_COLOR_OVERLAY_BG);
        vita_draw_card_custom(bx, by, bw, bh, VITA_COLOR_HEADER, VITA_COLOR_FOCUS_BORDER);
        vita_draw_text_centered(bx + bw * 0.5f, by + 14.0f, VITA_COLOR_AMIGA_RED, 1.0f, "Create Blank HDF");
        vita_draw_text_centered(bx + bw * 0.5f, by + 38.0f, VITA_COLOR_TEXT_MUTED, 0.75f, "Select image capacity (format from Workbench later)");

        float col_w = (bw - 60.0f) * 0.5f;
        for (int i = 0; i < HDF_CREATE_SIZE_COUNT; i++) {
            int row = i / 2;
            int col = i % 2;
            float ix = bx + 22.0f + (float)col * (col_w + 16.0f);
            float iy = by + 60.0f + (float)row * 44.0f;
            char size_str[32];
            snprintf(size_str, sizeof(size_str), "%d MB", s_hdf_create_sizes[i]);
            vita_draw_button_item(ix, iy, col_w, 38.0f, size_str, NULL, NULL, s_hdf_mgr_create_idx == i, false);
        }
        vita_draw_text_centered(bx + bw * 0.5f, by + bh - 24.0f, VITA_COLOR_TEXT_MUTED, 0.75f, "X Create   O Cancel");
        return;
    }

    if (s_hdf_mgr_item < 0) s_hdf_mgr_item = 0;
    if (s_hdf_mgr_item >= total_items) s_hdf_mgr_item = total_items - 1;

    if (input->pressed & SCE_CTRL_UP) {
        s_hdf_mgr_item--;
        if (s_hdf_mgr_item < 0) s_hdf_mgr_item = total_items - 1;
    }
    if (input->pressed & SCE_CTRL_DOWN) {
        s_hdf_mgr_item++;
        if (s_hdf_mgr_item >= total_items) s_hdf_mgr_item = 0;
    }
    if (input->pressed & SCE_CTRL_CIRCLE) {
        s_hdf_mgr_slot = -1;
        return;
    }

    if (input->pressed & SCE_CTRL_CROSS) {
        if (s_hdf_mgr_item == 0) {
            if (hdf_files[slot][0] == '\0') {
                vita_show_message_box("HDF Manager", "Mount an HDF first.", "OK (X)");
            } else {
                uae4all_hard_file_ro[slot] = uae4all_hard_file_ro[slot] ? 0 : 1;
                reset_hdConf();
                bReloadKickstart = 1;
                gui_update();
            }
        } else if (s_hdf_mgr_item == 1) {
            if (hdf_files[slot][0] == '\0') {
                vita_show_message_box("HDF Manager", "Mount an HDF first.", "OK (X)");
            } else if (vita_show_confirm_box("Backup HDF",
                    "Create a backup copy in ux0:/data/uae4all/backups/?",
                    "Backup (X)", "Cancel (O)")) {
                char err[256];
                int rc = hdf_backup(hdf_files[slot], "ux0:/data/uae4all/backups", err, sizeof(err));
                if (rc == 0)
                    vita_show_message_box("Backup Complete", "The HDF was backed up successfully.", "OK (X)");
                else
                    vita_show_message_box("Backup Failed", err, "OK (X)");
            }
        } else if (s_hdf_mgr_item == 2) {
            if (hdf_files[slot][0] == '\0') {
                vita_show_message_box("HDF Manager", "Nothing mounted in this slot.", "OK (X)");
            } else if (vita_show_confirm_box("Unmount HDF",
                    "Remove this hard-disk image from the slot?",
                    "Unmount (X)", "Cancel (O)")) {
                hdf_files[slot][0] = 0;
                uae4all_hard_file_ro[slot] = 0;
                reset_hdConf();
                bReloadKickstart = 1;
                gui_update();
                s_hdf_mgr_slot = -1;
                return;
            }
        } else if (s_hdf_mgr_item == 3) {
            s_hdf_mgr_create_idx = 3;
            s_hdf_mgr_creating = 1;
        } else if (s_hdf_mgr_item == 4) {
            s_hdf_mgr_slot = -1;
            return;
        }
    }

    float card_x = 20.0f;
    float card_w = VITA_SCREEN_W - 40.0f;
    float start_y = 86.0f;

    char title[64];
    snprintf(title, sizeof(title), "HDF Manager - Slot %d", slot + 1);
    vita_draw_text(card_x, start_y, VITA_COLOR_TEXT_WHITE, 1.05f, title);
    vita_draw_text_right(VITA_SCREEN_W - 20.0f, start_y + 4.0f, VITA_COLOR_TEXT_MUTED, 0.75f, "X Select   O Back");

    float info_y = start_y + 22.0f;
    float info_h = 84.0f;
    vita_draw_card_custom(card_x, info_y, card_w, info_h, VITA_COLOR_CARD, VITA_COLOR_CARD_BORDER);

    const HdfInfo *inf = &s_hdf_mgr_info;
    char line[128];
    float ly = info_y + 10.0f;
    float lh = 19.0f;

    if (hdf_files[slot][0] == '\0') {
        snprintf(line, sizeof(line), "File: (none)");
        vita_draw_text(card_x + 16.0f, ly, VITA_COLOR_TEXT_WHITE, 0.82f, line);
        ly += lh;
        snprintf(line, sizeof(line), "Create a blank HDF below and it will be mounted in this slot.");
        vita_draw_text(card_x + 16.0f, ly, VITA_COLOR_TEXT_MUTED, 0.78f, line);
        ly += lh;
        snprintf(line, sizeof(line), "Status: Empty slot");
        vita_draw_text(card_x + 16.0f, ly, VITA_COLOR_AMIGA_ORANGE, 0.80f, line);
    } else {
        snprintf(line, sizeof(line), "File: %s", get_filename_only(hdf_files[slot]));
        vita_draw_text(card_x + 16.0f, ly, VITA_COLOR_TEXT_WHITE, 0.85f, line);
        ly += lh;

        if (inf->size >= 1048576UL)
            snprintf(line, sizeof(line), "Size: %.2f MB (%lu bytes)  |  Mounted: %s  |  Writable: %s",
                     (double)inf->size / 1048576.0, inf->size,
                     uae4all_hard_file_ro[slot] ? "Read-Only" : "Read/Write",
                     inf->is_readonly ? "No" : "Yes");
        else
            snprintf(line, sizeof(line), "Size: %lu bytes  |  Mounted: %s",
                     inf->size, uae4all_hard_file_ro[slot] ? "Read-Only" : "Read/Write");
        vita_draw_text(card_x + 16.0f, ly, VITA_COLOR_TEXT_MUTED, 0.76f, line);
        ly += lh;

        snprintf(line, sizeof(line), "Geometry: Sectors %d | Surfaces %d | Cylinders %d | Filesystem: %s",
                 inf->sectors_per_track, inf->surfaces, inf->cylinders, inf->filesystem);
        vita_draw_text(card_x + 16.0f, ly, VITA_COLOR_TEXT_MUTED, 0.76f, line);
        ly += lh;

        if (inf->valid)
            snprintf(line, sizeof(line), "Status: %s", inf->is_rdb ? "RDB image (partitions not bootable on this core)" : "Valid hard disk image");
        else
            snprintf(line, sizeof(line), "Status: %s", inf->error);
        vita_draw_text(card_x + 16.0f, ly, inf->valid ? VITA_COLOR_AMIGA_GREEN : VITA_COLOR_DANGER, 0.80f, line);
    }

    float ay = info_y + info_h + 8.0f;
    float item_h = 40.0f;

    vita_draw_switch_item(card_x, ay, card_w, item_h, "Read-Only Mode",
        uae4all_hard_file_ro[slot] != 0, s_hdf_mgr_item == 0);
    ay += item_h + 5.0f;

    vita_draw_button_item(card_x, ay, card_w, item_h, "Backup HDF",
        "Create a copy in ux0:/data/uae4all/backups/", "BACKUP", s_hdf_mgr_item == 1, false);
    ay += item_h + 5.0f;

    vita_draw_button_item(card_x, ay, card_w, item_h, "Unmount HDF",
        "Remove this image from the slot", "UNMOUNT", s_hdf_mgr_item == 2, false);
    ay += item_h + 5.0f;

    vita_draw_button_item(card_x, ay, card_w, item_h, "Create Blank HDF",
        "Make a new empty image and mount it in this slot", "CREATE", s_hdf_mgr_item == 3, false);
    ay += item_h + 5.0f;

    vita_draw_button_item(card_x, ay, card_w, item_h, "Back",
        "Return to hard disk slots", "BACK", s_hdf_mgr_item == 4, false);
}


void vita_view_hard_disk(VitaInputState *input, int *selected_item)
{
    if (s_hdf_mgr_slot >= 0) {
        vita_view_hdf_manager(input, selected_item);
        return;
    }

    const int total_items = 6;
    if (*selected_item < 0) *selected_item = 0;
    if (*selected_item >= total_items) *selected_item = total_items - 1;

    if (input->pressed & SCE_CTRL_UP) {
        (*selected_item)--;
        if (*selected_item < 0) *selected_item = total_items - 1;
    }
    if (input->pressed & SCE_CTRL_DOWN) {
        (*selected_item)++;
        if (*selected_item >= total_items) *selected_item = 0;
    }

    char *hdf_files[4] = {
        uae4all_hard_file0, uae4all_hard_file1,
        uae4all_hard_file2, uae4all_hard_file3
    };

    if (input->pressed & SCE_CTRL_CROSS) {
        if (*selected_item >= 0 && *selected_item < 4) {
            char new_file[512];
            new_file[0] = 0;
            int res = vita_gui_run_browser(new_file, currentDir, 4 + *selected_item);
            if (res == 1) {
                copy_drive_path(hdf_files[*selected_item], new_file);
                make_hard_file_cfg_line(hdf_files[*selected_item]);
                mainMenu_whdload_game[0] = '\0';
                ApplyAutomaticGamePreset(1);
                gui_update();
            } else if (res == 2) {
                hdf_files[*selected_item][0] = 0;
                reset_hdConf();
                bReloadKickstart = 1;
                gui_update();
            }
        } else if (*selected_item == 4) {
            mainMenu_bootHD = (mainMenu_bootHD + 1) % 3;
            reset_hdConf();
            bReloadKickstart = 1;
        } else if (*selected_item == 5) {
            for (int i = 0; i < 4; i++)
                hdf_files[i][0] = 0;
            mainMenu_bootHD = 0;
            reset_hdConf();
            bReloadKickstart = 1;
            gui_update();
        }
    }

    if ((input->pressed & SCE_CTRL_TRIANGLE) && *selected_item < 4) {
        hdf_files[*selected_item][0] = 0;
        reset_hdConf();
        bReloadKickstart = 1;
        gui_update();
    }

    if ((input->pressed & SCE_CTRL_SQUARE) && *selected_item >= 0 && *selected_item < 4) {
        s_hdf_mgr_slot = *selected_item;
        s_hdf_mgr_item = 0;
        s_hdf_mgr_analyzed = 0;
        s_hdf_mgr_creating = 0;
    }

    float card_x = 20.0f;
    float card_w = VITA_SCREEN_W - 40.0f;
    float start_y = 90.0f;
    float item_h = 56.0f;

    for (int i = 0; i < 4; i++) {
        float y = start_y + (float)i * (item_h + 8.0f);
        bool focused = (*selected_item == i);
        bool loaded = hdf_files[i][0] != 0;
        char slot_title[64];
        char file_name[128];
        snprintf(slot_title, sizeof(slot_title), "HDF%d: Hard Disk Slot %d", i + 1, i + 1);
        vita_truncate_text(get_filename_only(hdf_files[i]), card_w - 230.0f, 0.95f, file_name, sizeof(file_name));
        vita_draw_card(card_x, y, card_w, item_h, focused, loaded);
        char slot_tag[16];
        snprintf(slot_tag, sizeof(slot_tag), "HDF%d", i + 1);
        vita_draw_badge(card_x + 14.0f, y + 17.0f, slot_tag,
            loaded ? VITA_COLOR_AMIGA_RED : RGBA8(40, 50, 70, 255), VITA_COLOR_TEXT_WHITE);
        vita_draw_text(card_x + 78.0f, y + 10.0f, focused ? VITA_COLOR_TEXT_WHITE : VITA_COLOR_TEXT_MUTED,
            0.85f, slot_title);
        vita_draw_text(card_x + 78.0f, y + 29.0f,
            loaded ? VITA_COLOR_TEXT_WHITE : VITA_COLOR_TEXT_DIM, 0.95f, file_name);
        vita_draw_led(card_x + card_w - 110.0f, y + 18.0f, loaded ? "LOADED" : "EMPTY",
            loaded, VITA_COLOR_AMIGA_ORANGE);
    }

    const char *boot_names[3] = { "Off", "HD Directory", "HDF Files" };
    vita_draw_selector_item(card_x, start_y + 4.0f * (item_h + 8.0f), card_w,
        item_h, "Boot HD", boot_names[mainMenu_bootHD % 3], *selected_item == 4);
    vita_draw_button_item(card_x, start_y + 5.0f * (item_h + 8.0f), card_w, 44.0f,
        "Eject All HDF Files", "Remove all mounted hard-disk images", "EJECT",
        *selected_item == 5, false);
}

static SDL_Surface *s_whdload_cover = NULL;
static char s_whdload_cover_game[128] = "";
static float s_whdload_cover_angle = 0.0f;

static void whdload_cover_unload(void)
{
    if (s_whdload_cover) {
        SDL_FreeSurface(s_whdload_cover);
        s_whdload_cover = NULL;
    }
    s_whdload_cover_game[0] = '\0';
}

static void whdload_cover_load(const char *game_name)
{
    if (!game_name || game_name[0] == '\0') {
        whdload_cover_unload();
        return;
    }
    if (s_whdload_cover && strcmp(s_whdload_cover_game, game_name) == 0)
        return;

    whdload_cover_unload();

    char path[512];
    snprintf(path, sizeof(path), "ux0:/data/uae4all/covers/%s.png", game_name);
    FILE *test = fopen(path, "rb");
    if (test) {
        fclose(test);
        s_whdload_cover = IMG_Load(path);
    }
    if (!s_whdload_cover) {
        snprintf(path, sizeof(path), "ux0:/data/uae4all/covers/%s.jpg", game_name);
        test = fopen(path, "rb");
        if (test) {
            fclose(test);
            s_whdload_cover = IMG_Load(path);
        }
    }
    if (s_whdload_cover)
        strncpy(s_whdload_cover_game, game_name, sizeof(s_whdload_cover_game) - 1);
}

static void whdload_install_flow(void)
{
    char archive_path[512];
    char installed_path[512];
    archive_path[0] = '\0';
    installed_path[0] = '\0';
    int result = vita_gui_run_browser(archive_path, currentDir, 9);
    if (result == 1) {
        if (vita_whdload_install_lha(archive_path, installed_path, sizeof(installed_path))) {
            vita_show_message_box("WHDLoad Installed", "The LHA archive was extracted to the WHDLoad library.", "OK (X)");
        } else {
            char err_buf[300];
            snprintf(err_buf, sizeof(err_buf), "The LHA archive could not be extracted.\n%s", vita_whdload_get_last_error());
            vita_show_message_box("Installation Failed", err_buf, "OK (X)");
        }
    }
}

#define WHDLOAD_FAVORITES_FILE "ux0:/data/uae4all/favorites.txt"
#define WHDLOAD_RECENT_FILE    "ux0:/data/uae4all/recent.txt"

static int s_whdload_filter = 0;
static char s_whdload_last_game[128] = "";

static char s_favs[256][128];
static int s_fav_count = 0;
static char s_recents[20][128];
static int s_recent_count = 0;

static void whdload_read_name_list(const char *path, char names[][128], int max_names, int *count)
{
    *count = 0;
    FILE *f = fopen(path, "rb");
    if (!f) return;
    char line[160];
    while (*count < max_names && fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0') continue;
        strncpy(names[*count], line, 127);
        names[*count][127] = '\0';
        (*count)++;
    }
    fclose(f);
}

static void whdload_refresh_meta(void)
{
    whdload_read_name_list(WHDLOAD_FAVORITES_FILE, s_favs, 256, &s_fav_count);
    whdload_read_name_list(WHDLOAD_RECENT_FILE, s_recents, 20, &s_recent_count);
}

static bool whdload_is_favorite(const char *game)
{
    for (int i = 0; i < s_fav_count; i++) {
        if (strcmp(s_favs[i], game) == 0) return true;
    }
    return false;
}

static bool whdload_is_recent(const char *game)
{
    for (int i = 0; i < s_recent_count; i++) {
        if (strcmp(s_recents[i], game) == 0) return true;
    }
    return false;
}

static void whdload_toggle_favorite(const char *game)
{
    char lines[256][128];
    int count = 0;
    bool was_favorite = false;
    FILE *f = fopen(WHDLOAD_FAVORITES_FILE, "rb");
    if (f) {
        char line[160];
        while (count < 256 && fgets(line, sizeof(line), f)) {
            line[strcspn(line, "\r\n")] = '\0';
            if (line[0] == '\0') continue;
            if (strcmp(line, game) == 0) {
                was_favorite = true;
                continue;
            }
            strncpy(lines[count], line, 127);
            lines[count][127] = '\0';
            count++;
        }
        fclose(f);
    }
    if (!was_favorite && count < 256) {
        strncpy(lines[count], game, 127);
        lines[count][127] = '\0';
        count++;
    }
    f = fopen(WHDLOAD_FAVORITES_FILE, "wb");
    if (f) {
        for (int i = 0; i < count; i++)
            fprintf(f, "%s\n", lines[i]);
        fclose(f);
    }
}

static void whdload_mark_recent(const char *game)
{
    char lines[20][128];
    int count = 0;
    FILE *f = fopen(WHDLOAD_RECENT_FILE, "rb");
    if (f) {
        char line[160];
        while (count < 20 && fgets(line, sizeof(line), f)) {
            line[strcspn(line, "\r\n")] = '\0';
            if (line[0] && strcmp(line, game) != 0) {
                strncpy(lines[count], line, 127);
                lines[count][127] = '\0';
                count++;
            }
        }
        fclose(f);
    }
    f = fopen(WHDLOAD_RECENT_FILE, "wb");
    if (f) {
        fprintf(f, "%s\n", game);
        for (int i = 0; i < count; i++)
            fprintf(f, "%s\n", lines[i]);
        fclose(f);
    }
}

static int vita_has_inserted_floppy(void)
{
    return uae4all_image_file0[0] != '\0' || uae4all_image_file1[0] != '\0' ||
           uae4all_image_file2[0] != '\0' || uae4all_image_file3[0] != '\0';
}

static int vita_has_mounted_hdf(void)
{
    return uae4all_hard_file0[0] != '\0' || uae4all_hard_file1[0] != '\0' ||
           uae4all_hard_file2[0] != '\0' || uae4all_hard_file3[0] != '\0';
}

static void vita_eject_all_floppies(void)
{
    uae4all_image_file0[0] = '\0';
    uae4all_image_file1[0] = '\0';
    uae4all_image_file2[0] = '\0';
    uae4all_image_file3[0] = '\0';
    gui_update();
}

static void vita_eject_all_hdf(void)
{
    uae4all_hard_file0[0] = '\0';
    uae4all_hard_file1[0] = '\0';
    uae4all_hard_file2[0] = '\0';
    uae4all_hard_file3[0] = '\0';
    uae4all_hard_file_ro[0] = 0;
    uae4all_hard_file_ro[1] = 0;
    uae4all_hard_file_ro[2] = 0;
    uae4all_hard_file_ro[3] = 0;
    reset_hdConf();
    gui_update();
}

int vita_confirm_eject_for_hard_disk_launch(void)
{
    if (!vita_has_inserted_floppy())
        return 1;
    if (!vita_show_confirm_box("Floppy Disk Detected",
            "A floppy disk is inserted. Eject it before launching this hard-disk game?",
            "Eject and Launch (X)", "Cancel Launch (O)"))
        return 0;
    vita_eject_all_floppies();
    return 1;
}

static int vita_confirm_eject_for_whdload_launch(void)
{
    int has_floppy = vita_has_inserted_floppy();
    int has_hdf = vita_has_mounted_hdf();
    if (!has_floppy && !has_hdf)
        return 1;
    const char *message;
    if (has_floppy && has_hdf)
        message = "A floppy disk and an HDF image are mounted. Eject them before launching this WHDLoad game?";
    else if (has_hdf)
        message = "An HDF image is mounted. Eject it before launching this WHDLoad game?";
    else
        message = "A floppy disk is inserted. Eject it before launching this WHDLoad game?";
    if (!vita_show_confirm_box("Media Detected", message,
            "Eject and Launch (X)", "Cancel Launch (O)"))
        return 0;
    if (has_floppy)
        vita_eject_all_floppies();
    if (has_hdf)
        vita_eject_all_hdf();
    return 1;
}

static void whdload_ensure_game_dir(const char *game)
{
    sceIoMkdir("ux0:/data/uae4all/saves", 0777);
    char dir[256];
    snprintf(dir, sizeof(dir), "ux0:/data/uae4all/saves/%s", game);
    sceIoMkdir(dir, 0777);
}

void vita_view_whdload(VitaInputState *input, int *selected_item)
{
    char games[64][128];
    int game_count = vita_whdload_list(games, 64);

    whdload_refresh_meta();

    int vis_index[64];
    int vis_count = 0;
    for (int k = 0; k < game_count; k++) {
        bool keep;
        if (s_whdload_filter == 1)
            keep = whdload_is_favorite(games[k]);
        else if (s_whdload_filter == 2)
            keep = whdload_is_recent(games[k]);
        else
            keep = true;
        if (keep)
            vis_index[vis_count++] = k;
    }

    const int total_items = 4 + vis_count;
    if (*selected_item < 0) *selected_item = 0;
    if (*selected_item >= total_items) *selected_item = total_items - 1;
    if (total_items <= 0) return;

    if (input->pressed & SCE_CTRL_UP) {
        (*selected_item)--;
        if (*selected_item < 0) *selected_item = total_items - 1;
    }
    if (input->pressed & SCE_CTRL_DOWN) {
        (*selected_item)++;
        if (*selected_item >= total_items) *selected_item = 0;
    }

    if (*selected_item >= 4) {
        const char *game = games[vis_index[*selected_item - 4]];
        strncpy(s_whdload_last_game, game, sizeof(s_whdload_last_game) - 1);
        s_whdload_last_game[sizeof(s_whdload_last_game) - 1] = '\0';
        strncpy(mainMenu_whdload_game, game, sizeof(mainMenu_whdload_game) - 1);
        mainMenu_whdload_game[sizeof(mainMenu_whdload_game) - 1] = '\0';
        whdload_ensure_game_dir(game);
        whdload_cover_load(game);
    } else {
        whdload_cover_unload();
    }

    if ((input->pressed & (SCE_CTRL_LEFT | SCE_CTRL_RIGHT)) && *selected_item == 3) {
        s_whdload_filter = (s_whdload_filter + ((input->pressed & SCE_CTRL_RIGHT) ? 1 : -1) + 3) % 3;
    }

    if (input->pressed & SCE_CTRL_CROSS) {
        if (*selected_item == 0) {
            if (s_whdload_last_game[0] == '\0') {
                vita_show_message_box("Download Cover", "Select a game first, then choose Download Cover Art.", "OK (X)");
            } else {
                char out_path[512];
                out_path[0] = '\0';
                int rc = vita_cover_download(s_whdload_last_game, out_path, sizeof(out_path));
                if (rc == 0) {
                    whdload_cover_load(s_whdload_last_game);
                    vita_show_message_box("Cover Downloaded", "Boxart saved to:\nux0:/data/uae4all/covers/", "OK (X)");
                } else if (rc == -6) {
                    vita_show_message_box("Cover Not Found", "No boxart for this game on the cover server.", "OK (X)");
                } else {
                    vita_show_message_box("Download Failed", "Network error or no internet connection.\nServer: ux0:/data/uae4all/covers/source.txt", "OK (X)");
                }
            }
        } else if (*selected_item == 1) {
            whdload_install_flow();
        } else if (*selected_item == 2) {
            strncpy(uae4all_hard_dir, vita_whdload_root(), 255);
            uae4all_hard_dir[255] = '\0';
            mainMenu_bootHD = 1;
            reset_hdConf();
            gui_update();
            mainMenu_whdload_game[0] = '\0';
            vita_show_message_box("WHDLoad Directory", "The WHDLoad library is selected as the HD directory. A Workbench environment is required.", "OK (X)");
        } else if (*selected_item == 3) {
            s_whdload_filter = (s_whdload_filter + 1) % 3;
        } else {
            const char *game_name = games[vis_index[*selected_item - 4]];
            if (vita_whdload_prepare_launch(game_name)) {
                if (!vita_confirm_eject_for_whdload_launch())
                    return;

                strncpy(mainMenu_whdload_game, game_name, sizeof(mainMenu_whdload_game) - 1);
                mainMenu_whdload_game[sizeof(mainMenu_whdload_game) - 1] = '\0';
                whdload_ensure_game_dir(game_name);
                whdload_mark_recent(game_name);

                strncpy(uae4all_hard_dir, vita_whdload_root(), 255);
                uae4all_hard_dir[255] = '\0';
                ApplyAutomaticGamePreset(2);
                vita_set_kickstart(kickstart, 0);

                gui_update();
                mainMenu_case = MAIN_MENU_CASE_RUN;
            } else {
                vita_show_message_box("WHDLoad Error", "No .slave file was found or the startup script could not be prepared.", "OK (X)");
            }
        }
    }

    if (input->pressed & SCE_CTRL_SQUARE) {
        whdload_install_flow();
    }

    if (input->pressed & SCE_CTRL_TRIANGLE) {
        mainMenu_case = MAIN_MENU_CASE_RESET;
    }

    if ((input->pressed & SCE_CTRL_SELECT) && *selected_item >= 4) {
        whdload_toggle_favorite(games[vis_index[*selected_item - 4]]);
    }

    float card_x = 20.0f;
    float card_w = 560.0f;
    float start_y = 90.0f;
    float item_h = 56.0f;
    int visible_items = 6;
    int first_item = *selected_item >= visible_items ? *selected_item - visible_items + 1 : 0;

    for (int i = 0; i < visible_items; i++) {
        int item = first_item + i;
        if (item >= total_items) break;
        float y = start_y + (float)i * (item_h + 8.0f);
        const char *title;
        const char *subtitle;
        const char *badge;
        unsigned int badge_col = VITA_COLOR_AMIGA_BLUE;

        if (item == 0) {
            title = "Download Cover Art";
            subtitle = s_whdload_last_game[0] ? s_whdload_last_game : "Select a game first, then fetch its boxart";
            badge = "WEB";
            badge_col = VITA_COLOR_AMIGA_ORANGE;
        } else if (item == 1) {
            title = "Install Game from LHA";
            subtitle = "Extract a WHDLoad archive into the Vita game library";
            badge = "INSTALL";
        } else if (item == 2) {
            title = "Use WHDLoad Directory";
            subtitle = "Select the library as the Amiga HD directory";
            badge = "HD DIR";
        } else if (item == 3) {
            title = "Library Filter";
            subtitle = "All games / favorites only / recently played";
            badge = (s_whdload_filter == 1) ? "FAVORITES" : (s_whdload_filter == 2) ? "RECENT" : "ALL GAMES";
            badge_col = VITA_COLOR_AMIGA_ORANGE;
        } else {
            int orig = vis_index[item - 4];
            bool fav = whdload_is_favorite(games[orig]);
            bool rec = whdload_is_recent(games[orig]);
            title = games[orig];
            subtitle = fav ? "Favorite - installed WHDLoad game" : "Installed WHDLoad game";
            badge = fav ? "* FAV" : (rec ? "RECENT" : "READY");
            if (fav) badge_col = VITA_COLOR_AMIGA_RED;
            else if (rec) badge_col = VITA_COLOR_AMIGA_GREEN;
        }
        vita_draw_button_item_custom(card_x, y, card_w, item_h, title, subtitle, badge, badge_col, *selected_item == item, false);
    }

    if (game_count == 0) {
        vita_draw_text(card_x + 16.0f, 470.0f, VITA_COLOR_TEXT_MUTED, 0.85f, "No LHA games installed");
    } else if (vis_count == 0) {
        vita_draw_text(card_x + 16.0f, 470.0f, VITA_COLOR_TEXT_MUTED, 0.85f, "No games match the current filter");
    }

    float preview_x = 596.0f;
    float preview_w = VITA_SCREEN_W - 20.0f - preview_x;
    float preview_y = 90.0f;
    float preview_h = 392.0f;
    vita_draw_card_custom(preview_x, preview_y, preview_w, preview_h, VITA_COLOR_CARD, VITA_COLOR_CARD_BORDER);

    bool is_game = (*selected_item >= 4);
    const char *preview_title = is_game ? games[vis_index[*selected_item - 4]]
                              : (*selected_item == 0 ? "Download Cover Art"
                                  : (*selected_item == 1 ? "Install Game from LHA"
                                      : (*selected_item == 2 ? "Use WHDLoad Directory" : "Library Filter")));
    bool preview_fav = is_game && whdload_is_favorite(games[vis_index[*selected_item - 4]]);

    vita_draw_badge(preview_x + 14.0f, preview_y + 12.0f,
        is_game ? "WHDLOAD GAME" : "WHDLOAD LIBRARY",
        is_game ? VITA_COLOR_AMIGA_RED : RGBA8(40, 50, 70, 255), VITA_COLOR_TEXT_WHITE);
    float art_x = preview_x + 16.0f;
    float art_y = preview_y + 46.0f;
    float art_w = preview_w - 32.0f;
    float art_h = 230.0f;

    if (s_whdload_cover) {
        float surf_w = (float)s_whdload_cover->w;
        float surf_h = (float)s_whdload_cover->h;
        float scale_x = art_w / surf_w;
        float scale_y = art_h / surf_h;
        float final_scale = (scale_x < scale_y) ? scale_x : scale_y;
        if (final_scale > 1.0f) final_scale = 1.0f;
        if (final_scale < 0.05f) final_scale = 0.05f;

        int draw_w = (int)(surf_w * final_scale);
        int draw_h = (int)(surf_h * final_scale);
        int draw_x = (int)(art_x + (art_w - draw_w) * 0.5f);
        int draw_y = (int)(art_y + (art_h - draw_h) * 0.5f);

        vita_draw_rounded_rect(art_x - 3.0f, art_y - 3.0f, art_w + 6.0f, art_h + 6.0f, 6.0f, RGBA8(10, 13, 20, 255));
        SDL_Rect dst_r = { (Sint16)draw_x, (Sint16)draw_y, (Uint16)draw_w, (Uint16)draw_h };
        SDL_SoftStretch(s_whdload_cover, NULL, prSDLScreen, &dst_r);
    } else {
        s_whdload_cover_angle += 0.08f;
        vita_draw_rounded_rect(art_x, art_y, art_w, art_h, 6.0f, RGBA8(18, 22, 32, 255));
        vita_draw_boing_ball_icon(art_x + (art_w * 0.5f), art_y + (art_h * 0.5f) - 10.0f, 34.0f, s_whdload_cover_angle);
        vita_draw_text_centered(art_x + (art_w * 0.5f), art_y + art_h - 28.0f,
            is_game ? VITA_COLOR_TEXT_MUTED : VITA_COLOR_AMIGA_ORANGE, 0.85f,
            is_game ? "No cover found" : "Cover: <GameName>.png");
    }

    vita_draw_text(preview_x + 16.0f, preview_y + 290.0f, VITA_COLOR_TEXT_DIM, 0.75f, "GAME TITLE");
    char title_buf[128];
    float title_max_w = preview_w - 32.0f - (preview_fav ? 78.0f : 0.0f);
    vita_truncate_text(preview_title, title_max_w, 0.95f, title_buf, sizeof(title_buf));
    vita_draw_text(preview_x + 16.0f, preview_y + 310.0f, VITA_COLOR_TEXT_WHITE, 0.95f, title_buf);
    if (preview_fav) {
        vita_draw_badge(preview_x + preview_w - 86.0f, preview_y + 306.0f, "* FAV", VITA_COLOR_AMIGA_RED, VITA_COLOR_TEXT_WHITE);
    }

    vita_draw_text(preview_x + 16.0f, preview_y + 336.0f, VITA_COLOR_TEXT_DIM, 0.75f, "RECOMMENDED HARDWARE");
    if (is_game) {
        vita_draw_badge(preview_x + 16.0f, preview_y + 352.0f, "A1200 AGA", VITA_COLOR_AMIGA_ORANGE, RGBA8(20, 24, 34, 255));
        vita_draw_hint_item(preview_x + preview_w - 150.0f, preview_y + 352.0f, VITA_BTN_CROSS, "LAUNCH");
        char hw_buf[128];
        vita_truncate_text("68020 14MHz | Kickstart 3.1 | 2MB Chip + 4MB Fast", preview_w - 32.0f, 0.78f, hw_buf, sizeof(hw_buf));
        vita_draw_text(preview_x + 16.0f, preview_y + 378.0f, VITA_COLOR_TEXT_MUTED, 0.78f, hw_buf);
    } else {
        char hw_buf[128];
        vita_truncate_text("WHDLoad slave games run best on an A1200 AGA setup", preview_w - 190.0f, 0.80f, hw_buf, sizeof(hw_buf));
        vita_draw_text(preview_x + 16.0f, preview_y + 354.0f, VITA_COLOR_TEXT_MUTED, 0.80f, hw_buf);
        vita_draw_hint_item(preview_x + preview_w - 150.0f, preview_y + 352.0f, VITA_BTN_CROSS, "SELECT");
    }
}

void vita_view_presets(VitaInputState *input, int *selected_item)
{
    const int total_items = 5;
    if (*selected_item < 0) *selected_item = 0;
    if (*selected_item >= total_items) *selected_item = total_items - 1;

    if (input->pressed & SCE_CTRL_UP) {
        (*selected_item)--;
        if (*selected_item < 0) *selected_item = total_items - 1;
    }
    if (input->pressed & SCE_CTRL_DOWN) {
        (*selected_item)++;
        if (*selected_item >= total_items) *selected_item = 0;
    }

        if (input->pressed & SCE_CTRL_CROSS) {
        if (*selected_item == 0) {
            kickstart = 1;
            extfile[0] = '\0';
            mainMenu_CPU_model = 0;
            mainMenu_chipset = 0x100;
            mainMenu_chipMemory = 0;
            mainMenu_slowMemory = 1;
            mainMenu_fastMemory = 0;
            UpdateCPUModelSettings();
            UpdateMemorySettings();
            UpdateChipsetSettings();
            int kickstart_loaded = vita_set_kickstart(kickstart, 0);
            bReloadKickstart = 1;
            if (kickstart_loaded)
                vita_show_message_box("Preset Applied", "Amiga 500 (OCS 1.3, 512K+512K RAM) configured! Press Save Game Configuration to save it.", "OK (X)");
            else
                vita_show_message_box("Kickstart Missing", "Kickstart 1.3 ROM (kick13.rom / kick34005.A500) not found in ux0:/data/uae4all/kickstarts/.", "OK (X)");
        } else if (*selected_item == 1) {
            kickstart = 2;
            extfile[0] = '\0';
            mainMenu_CPU_model = 0;
            mainMenu_chipset = 1 | 0x100;
            mainMenu_chipMemory = 1;
            mainMenu_slowMemory = 0;
            mainMenu_fastMemory = 1;
            UpdateCPUModelSettings();
            UpdateMemorySettings();
            UpdateChipsetSettings();
            int kickstart_loaded = vita_set_kickstart(kickstart, 0);
            bReloadKickstart = 1;
            if (kickstart_loaded)
                vita_show_message_box("Preset Applied", "Amiga 500+ (ECS 2.04, 1MB+1MB RAM) configured! Press Save Game Configuration to save it.", "OK (X)");
            else
                vita_show_message_box("Kickstart Missing", "Kickstart 2.04 ROM (kick20.rom / kick37175.A500) not found in ux0:/data/uae4all/kickstarts/.", "OK (X)");
        } else if (*selected_item == 2) {
            kickstart = 11;
            extfile[0] = '\0';
            mainMenu_CPU_model = 0;
            mainMenu_chipset = 1 | 0x100;
            mainMenu_chipMemory = 2;
            mainMenu_slowMemory = 0;
            mainMenu_fastMemory = 4;
            UpdateCPUModelSettings();
            UpdateMemorySettings();
            UpdateChipsetSettings();
            int kickstart_loaded = vita_set_kickstart(kickstart, 0);
            bReloadKickstart = 1;
            if (kickstart_loaded)
                vita_show_message_box("Preset Applied", "Amiga 600 (ECS 2.05, 2MB Chip + 8MB Fast RAM) configured! Press Save Game Configuration to save it.", "OK (X)");
            else
                vita_show_message_box("Kickstart Missing", "Kickstart 2.05 ROM (kick37350.A600 / kick205.rom) not found in ux0:/data/uae4all/kickstarts/.", "OK (X)");
        } else if (*selected_item == 3) {
            kickstart = 3;
            extfile[0] = '\0';
            mainMenu_CPU_model = 1;
            mainMenu_chipset = 2 | 0x100;
            mainMenu_chipMemory = 2;
            mainMenu_slowMemory = 0;
            mainMenu_fastMemory = 3;
            UpdateCPUModelSettings();
            UpdateMemorySettings();
            UpdateChipsetSettings();
            ApplyA1200Profile();
            int kickstart_loaded = vita_set_kickstart(kickstart, 0);
            bReloadKickstart = 1;
            if (kickstart_loaded)
                vita_show_message_box("Preset Applied", "Amiga 1200 (AGA 3.1, 68020 2MB+4MB RAM) configured! Press Save Game Configuration to save it.", "OK (X)");
            else
                vita_show_message_box("Kickstart Missing", "Kickstart 3.1 ROM (kick31.rom / kick40068.A1200) not found in ux0:/data/uae4all/kickstarts/.", "OK (X)");
        } else if (*selected_item == 4) {
            kickstart = 6;
            mainMenu_CPU_model = 1;
            mainMenu_chipset = 2 | 0x100;
            mainMenu_chipMemory = 2;
            mainMenu_slowMemory = 0;
            mainMenu_fastMemory = 0;
            UpdateCPUModelSettings();
            UpdateMemorySettings();
            UpdateChipsetSettings();
            int kickstart_loaded = vita_set_kickstart(kickstart, 0);
            bReloadKickstart = 1;
            if (kickstart_loaded)
                vita_show_message_box("Preset Applied", "Amiga CD32 (Akiko, CD32 Kickstart + Extended ROM) configured! Press Save Game Configuration to save it.", "OK (X)");
            else
                vita_show_message_box("Kickstart Missing", "Kickstart CD32 ROM (kick40060.CD32) not found in ux0:/data/uae4all/kickstarts/.", "OK (X)");
        }
    }

    
    float card_x = 20.0f;
    float card_w = VITA_SCREEN_W - 40.0f;
    float start_y = 90.0f;
    float item_h = 78.0f;

    struct PresetInfo {
        const char *title;
        const char *desc;
        const char *tag;
        const char *recom;
    } presets[5] = {
        { "Amiga 500 (Classic OCS 1.3)", "68000 7MHz | Kickstart 1.3 | 512KB Chip + 512KB Slow RAM", "OCS", "Recommended for 95% of classic Amiga games (1985-1993)" },
        { "Amiga 500+ (Enhanced ECS 2.04)", "68000 7MHz | Kickstart 2.04 | 1MB Chip + 1MB Fast RAM", "ECS", "Recommended for late ECS titles and productivity software" },
        { "Amiga 600 (Enhanced ECS 2.05)", "68000 7MHz | Kickstart 2.05 | 2MB Chip + 8MB Fast RAM", "ECS", "Recommended for Amiga 600 games and ECS software" },
        { "Amiga 1200 (Advanced AGA 3.1)", "68020 14MHz | Kickstart 3.1 | 2MB Chip + 4MB Fast RAM", "AGA", "Recommended for AGA games (Alien Breed 3D, Slam Tilt, Gloom)" },
        { "Amiga CD32 (Console CD Mode)", "68020 14MHz | Kickstart 3.1 CD32 | 2MB Chip RAM + Akiko", "CD32", "Recommended for Amiga CD32 ISO and CUE disc images" }
    };

    const int visible_items = 4;
    int first_item = *selected_item >= visible_items ? *selected_item - visible_items + 1 : 0;

    for (int i = 0; i < visible_items; i++) {
        int item = first_item + i;
        if (item >= total_items) break;
        float cy = start_y + (float)i * (item_h + 10.0f);
        bool focused = (*selected_item == item);

        vita_draw_card(card_x, cy, card_w, item_h, focused, false);

        unsigned int badge_col = (item == 3) ? VITA_COLOR_AMIGA_RED : ((item == 0) ? VITA_COLOR_AMIGA_BLUE : RGBA8(40, 50, 70, 255));
        float badge_x = card_x + ((item == 4) ? 8.0f : 14.0f);
        vita_draw_badge(badge_x, cy + 14.0f, presets[item].tag, badge_col, VITA_COLOR_TEXT_WHITE);

        vita_draw_text(card_x + 72.0f, cy + 12.0f, focused ? VITA_COLOR_TEXT_WHITE : RGBA8(230, 240, 255, 255), 1.05f, presets[item].title);

        char desc_buf[256];
        vita_truncate_text(presets[item].desc, card_w - 180.0f, 0.85f, desc_buf, sizeof(desc_buf));
        vita_draw_text(card_x + 72.0f, cy + 34.0f, VITA_COLOR_TEXT_MUTED, 0.85f, desc_buf);

        char rec_buf[256];
        vita_truncate_text(presets[item].recom, card_w - 180.0f, 0.82f, rec_buf, sizeof(rec_buf));
        vita_draw_text(card_x + 72.0f, cy + 52.0f, VITA_COLOR_AMIGA_ORANGE, 0.82f, rec_buf);

        if (focused) {
            vita_draw_hint_item(card_x + card_w - 140.0f, cy + 28.0f, VITA_BTN_CROSS, "Apply");
        }
    }
}

void vita_view_hardware(VitaInputState *input, int *selected_item)
{
    const int total_items = 8;
    if (*selected_item < 0) *selected_item = 0;
    if (*selected_item >= total_items) *selected_item = total_items - 1;

    if (input->pressed & SCE_CTRL_UP) {
        (*selected_item)--;
        if (*selected_item < 0) *selected_item = total_items - 1;
    }
    if (input->pressed & SCE_CTRL_DOWN) {
        (*selected_item)++;
        if (*selected_item >= total_items) *selected_item = 0;
    }

    if (input->pressed & SCE_CTRL_CROSS && *selected_item == 6) {
        char new_file[512];
        new_file[0] = '\0';
        int res = vita_gui_run_browser(new_file, currentDir, 8);
        if (res == 1) {
            if (cdrom_open_image(new_file)) {
                ApplyCd32Profile();
                vita_set_kickstart(kickstart, 0);
                bReloadKickstart = 1;
                vita_show_message_box("CD32 Image", "CD image inserted and CD32 profile applied.", "OK (X)");
            } else
                vita_show_message_box("CD32 Image Error", "The selected image could not be opened.", "OK (X)");
        }
    }
    if (input->pressed & SCE_CTRL_TRIANGLE && *selected_item == 6) {
        cdrom_close_image();
        vita_show_message_box("CD32 Image", "CD image ejected.", "OK (X)");
    }

    int dir = 0;
    if (input->pressed & (SCE_CTRL_RIGHT | SCE_CTRL_CROSS)) dir = 1;
    if (input->pressed & SCE_CTRL_LEFT) dir = -1;

    if (dir != 0) {
        switch (*selected_item) {
            case 0:
                kickstart = (kickstart + dir + KICKSTART_ROM_COUNT) % KICKSTART_ROM_COUNT;
                bReloadKickstart = 1;
                vita_set_kickstart(kickstart, 1);
                break;
            case 1:
                mainMenu_CPU_model = (mainMenu_CPU_model + dir + 2) % 2;
                bReloadKickstart = 1;
                UpdateCPUModelSettings();
                break;
            case 2:
                mainMenu_chipset = (mainMenu_chipset + dir + 3) % 3;
                bReloadKickstart = 1;
                UpdateChipsetSettings();
                break;
            case 3:
                mainMenu_chipMemory = (mainMenu_chipMemory + dir + 4) % 4;
                bReloadKickstart = 1;
                UpdateMemorySettings();
                break;
            case 4:
                mainMenu_fastMemory = (mainMenu_fastMemory + dir + 5) % 5;
                bReloadKickstart = 1;
                UpdateMemorySettings();
                break;
            case 5:
                mainMenu_slowMemory = (mainMenu_slowMemory + dir + 4) % 4;
                bReloadKickstart = 1;
                UpdateMemorySettings();
                break;
            case 7:
                mainMenu_midiSynth = 1 - mainMenu_midiSynth;
                midi_synth_set_enabled(mainMenu_midiSynth);
                if (mainMenu_midiSynth)
                    vita_show_message_box("MIDI Synth", "Serial MIDI capture enabled. Games writing to SERDAT / serial port will play through the built-in synthesizer.", "OK (X)");
                break;
        }
    }

    float card_x = 20.0f;
    float card_w = VITA_SCREEN_W - 40.0f;
    float start_y = 90.0f;
    float item_h = 44.0f;

    const char *ks_names[KICKSTART_ROM_COUNT] = {
        "Kickstart 1.2 (A500/A2000)", "Kickstart 1.3 (A500/A2000)", "Kickstart 2.04 (A500+)",
        "Kickstart 3.1 (A1200)", "Custom ROM", "AROS ROM", "Kickstart 3.1 CD32",
        "Kickstart 1.1 NTSC (A1000)", "Kickstart 1.1 PAL (A1000)", "Kickstart 1.2 (A500)",
        "Kickstart 2.04 (A500+)", "Kickstart 2.05 (A600)", "Kickstart 3.1 (A600)",
        "Kickstart 3.0 (A1200)", "Kickstart 3.1 (A1200)", "Kickstart 3.0 (A4000)",
        "Kickstart 3.1 (A4000)", "Kickstart 1.3 + CDTV Extended ROM"
    };
    const char *cpu_names[2] = { "Motorola 68000 (7 MHz)", "Motorola 68020 (14 MHz AGA)" };
    const char *chipset_names[3] = { "OCS (Original Chip Set)", "ECS (Enhanced Chip Set)", "AGA (Advanced Graphics)" };
    const char *chip_ram_names[4] = { "512 KB (Standard)", "1 MB", "2 MB (Expanded)", "None" };
    const char *fast_ram_names[5] = { "None", "1 MB", "2 MB", "4 MB (AGA recommended)", "8 MB" };
    const char *slow_ram_names[4] = { "None", "512 KB (Trapdoor)", "1 MB", "1.5 MB" };
    const char *cd_image_name = current_cd_image[0] ? get_filename_only(current_cd_image) : "No image selected";

    vita_draw_selector_item(card_x, start_y + 0.0f * (item_h + 6.0f), card_w, item_h, "Kickstart ROM", ks_names[kickstart % KICKSTART_ROM_COUNT], *selected_item == 0);
    vita_draw_selector_item(card_x, start_y + 1.0f * (item_h + 6.0f), card_w, item_h, "CPU Architecture", cpu_names[mainMenu_CPU_model % 2], *selected_item == 1);
    vita_draw_selector_item(card_x, start_y + 2.0f * (item_h + 6.0f), card_w, item_h, "Amiga Chipset", chipset_names[mainMenu_chipset % 3], *selected_item == 2);
    vita_draw_selector_item(card_x, start_y + 3.0f * (item_h + 6.0f), card_w, item_h, "Chip RAM", chip_ram_names[mainMenu_chipMemory % 4], *selected_item == 3);
    vita_draw_selector_item(card_x, start_y + 4.0f * (item_h + 6.0f), card_w, item_h, "Fast RAM", fast_ram_names[mainMenu_fastMemory % 5], *selected_item == 4);
    vita_draw_selector_item(card_x, start_y + 5.0f * (item_h + 6.0f), card_w, item_h, "Slow / Trapdoor RAM", slow_ram_names[mainMenu_slowMemory % 4], *selected_item == 5);
    vita_draw_selector_item(card_x, start_y + 6.0f * (item_h + 6.0f), card_w, item_h, "CD32 CD Image", cd_image_name, *selected_item == 6);
    vita_draw_switch_item(card_x, start_y + 7.0f * (item_h + 6.0f), card_w, item_h, "MIDI / MT-32 Synth Emulation", mainMenu_midiSynth == 1, *selected_item == 7);
}

void vita_view_display(VitaInputState *input, int *selected_item)
{
    const int total_items = 7;
    if (*selected_item < 0) *selected_item = 0;
    if (*selected_item >= total_items) *selected_item = total_items - 1;

    if (input->pressed & SCE_CTRL_UP) {
        (*selected_item)--;
        if (*selected_item < 0) *selected_item = total_items - 1;
    }
    if (input->pressed & SCE_CTRL_DOWN) {
        (*selected_item)++;
        if (*selected_item >= total_items) *selected_item = 0;
    }

    int dir = 0;
    if (input->pressed & (SCE_CTRL_RIGHT | SCE_CTRL_CROSS)) dir = 1;
    if (input->pressed & SCE_CTRL_LEFT) dir = -1;

    if (dir != 0) {
        switch (*selected_item) {
            case 0:
                mainMenu_shader = (mainMenu_shader + dir + 9) % 9;
                break;
            case 1:
                mainMenu_ntsc = (mainMenu_ntsc + dir + 2) % 2;
                break;
            case 2:
                mainMenu_showStatus = (mainMenu_showStatus + dir + 4) % 4;
                break;
            case 3: {
                int width_group = (presetModeId / 10) * 10;
                int height_mode = (presetModeId % 10 + dir + 9) % 9;
                SetPresetMode(width_group + height_mode);
                break;
            }
            case 4:
                moveY += dir;
                if (moveY < -26) moveY = -26;
                if (moveY > 128) moveY = 128;
                break;
            case 5:
                mainMenu_footerSize += dir * 8;
                if (mainMenu_footerSize < -64) mainMenu_footerSize = -64;
                if (mainMenu_footerSize > 160) mainMenu_footerSize = 160;
                break;
            case 6:
                mainMenu_screenOffsetY += dir * 8;
                if (mainMenu_screenOffsetY < -128) mainMenu_screenOffsetY = -128;
                if (mainMenu_screenOffsetY > 128) mainMenu_screenOffsetY = 128;
                break;
        }
    }

    float card_x = 20.0f;
    float card_w = VITA_SCREEN_W - 40.0f;
    float start_y = 90.0f;
    float item_h = 48.0f;

    const char *shader_names[9] = { "None / Raw", "Sharp Bilinear", "Sharp Bilinear Simple", "CRT Easymode (Scanlines)", "LCD 3x (Grid)", "FXAA", "Advanced AA", "Bicubic", "GTU" };
    const char *ntsc_names[2] = { "PAL (50 Hz European Standard)", "NTSC (60 Hz US Standard)" };
    const char *status_names[4] = { "Bottom Bar (Floppy LED & FPS)", "Top Bar (LED blocks)", "Disabled (Clean Screen)", "Vertical Bar" };

    vita_draw_selector_item(card_x, start_y + 0.0f * (item_h + 6.0f), card_w, item_h, "Hardware Vita Shader", shader_names[mainMenu_shader % 9], *selected_item == 0);
    vita_draw_selector_item(card_x, start_y + 1.0f * (item_h + 6.0f), card_w, item_h, "Screen Refresh & Region", ntsc_names[mainMenu_ntsc % 2], *selected_item == 1);
    vita_draw_selector_item(card_x, start_y + 2.0f * (item_h + 6.0f), card_w, item_h, "Status Bar (Floppy LED/FPS)", status_names[mainMenu_showStatus % 4], *selected_item == 2);
    char aspect_mode[64];
    if (presetModeId % 10 == 7)
        snprintf(aspect_mode, sizeof(aspect_mode), "Fullscreen 16:9 - %s", presetMode);
    else if (presetModeId % 10 == 8)
        snprintf(aspect_mode, sizeof(aspect_mode), "5:4 Correct - %s", presetMode);
    else
        snprintf(aspect_mode, sizeof(aspect_mode), "4:3 Correct - %s", presetMode);
    vita_draw_selector_item(card_x, start_y + 3.0f * (item_h + 6.0f), card_w, item_h, "Aspect Ratio / Scaling", aspect_mode, *selected_item == 3);

    char vertical_position[32];
    snprintf(vertical_position, sizeof(vertical_position), "%d (higher = up)", moveY);
    vita_draw_selector_item(card_x, start_y + 4.0f * (item_h + 6.0f), card_w, item_h, "Vertical Position", vertical_position, *selected_item == 4);

    char footer_size[64];
    if (mainMenu_footerSize > 0)
        snprintf(footer_size, sizeof(footer_size), "+%d px footer (top fixed)", mainMenu_footerSize);
    else if (mainMenu_footerSize < 0)
        snprintf(footer_size, sizeof(footer_size), "%d px crop (top fixed)", mainMenu_footerSize);
    else
        snprintf(footer_size, sizeof(footer_size), "0 px (full height)");
    vita_draw_selector_item(card_x, start_y + 5.0f * (item_h + 6.0f), card_w, item_h, "Game Footer Height", footer_size, *selected_item == 5);

    char screen_offset[64];
    if (mainMenu_screenOffsetY < 0)
        snprintf(screen_offset, sizeof(screen_offset), "%d px up", mainMenu_screenOffsetY);
    else if (mainMenu_screenOffsetY > 0)
        snprintf(screen_offset, sizeof(screen_offset), "+%d px down", mainMenu_screenOffsetY);
    else
        snprintf(screen_offset, sizeof(screen_offset), "0 px (centered at top)");
    vita_draw_selector_item(card_x, start_y + 6.0f * (item_h + 6.0f), card_w, item_h, "Game Screen Offset", screen_offset, *selected_item == 6);
}

void vita_view_controls(VitaInputState *input, int *selected_item)
{
    const int total_items = 7;
    if (*selected_item < 0) *selected_item = 0;
    if (*selected_item >= total_items) *selected_item = total_items - 1;

    if (input->pressed & SCE_CTRL_UP) {
        (*selected_item)--;
        if (*selected_item < 0) *selected_item = total_items - 1;
    }
    if (input->pressed & SCE_CTRL_DOWN) {
        (*selected_item)++;
        if (*selected_item >= total_items) *selected_item = 0;
    }

    int dir = 0;
    if (input->pressed & (SCE_CTRL_RIGHT | SCE_CTRL_CROSS)) dir = 1;
    if (input->pressed & SCE_CTRL_LEFT) dir = -1;

    if (dir != 0) {
        switch (*selected_item) {
            case 0:
                mainMenu_leftStickMouse = 1 - mainMenu_leftStickMouse;
                break;
            case 1:
                mainMenu_touchControls = (mainMenu_touchControls + dir + 3) % 3;
                break;
            case 2:
                mainMenu_autofire = (mainMenu_autofire + dir + 4) % 4;
                break;
            case 3:
                mainMenu_pinballMode = (mainMenu_pinballMode + dir + 3) % 3;
                break;
            case 4:
                break;
            case 5:
                mainMenu_diskSoundVolume += dir * 10;
                if (mainMenu_diskSoundVolume < 0) mainMenu_diskSoundVolume = 0;
                if (mainMenu_diskSoundVolume > 100) mainMenu_diskSoundVolume = 100;
                disk_sound_set_volume(mainMenu_diskSoundVolume);
                break;
            case 6:
                mainMenu_autoEjectFloppy = 1 - mainMenu_autoEjectFloppy;
                break;
        }
    }

    float card_x = 20.0f;
    float card_w = VITA_SCREEN_W - 40.0f;
    float start_y = 90.0f;
    float item_h = 48.0f;

    const char *touch_modes[3] = { "Direct Touchscreen (Finger)", "Relative Trackpad Cursor", "Disabled" };
    const char *autofire_names[4] = { "Off", "Slow (1)", "Medium (2)", "Turbo (3)" };
    const char *pinball_names[3] = { "Disabled", "L1 / R1 Flippers", "Dual (L1/Left + R1/Circle)" };

    const char *layout_desc = (mainMenu_pinballMode != 0)
        ? "Pinball: L1 = Left Flipper | R1 = Right Flipper | Cross = Plunger | []/O/D-Pad = Tilt"
        : "Cross = Fire 1 | Circle = Fire 2 | Square = Space | Triangle = Virtual Keyboard";

    vita_draw_switch_item(card_x, start_y + 0.0f * (item_h + 6.0f), card_w, item_h, "Right Analog Stick as Amiga Mouse", mainMenu_leftStickMouse == 1, *selected_item == 0);
    vita_draw_selector_item(card_x, start_y + 1.0f * (item_h + 6.0f), card_w, item_h, "PS Vita Touchscreen Mode", touch_modes[mainMenu_touchControls % 3], *selected_item == 1);
    vita_draw_selector_item(card_x, start_y + 2.0f * (item_h + 6.0f), card_w, item_h, "Autofire Rate", autofire_names[mainMenu_autofire % 4], *selected_item == 2);
    vita_draw_selector_item(card_x, start_y + 3.0f * (item_h + 6.0f), card_w, item_h, "Pinball Flippers (L1/R1)", pinball_names[mainMenu_pinballMode % 3], *selected_item == 3);
    vita_draw_button_item(card_x, start_y + 4.0f * (item_h + 6.0f), card_w, item_h, "Physical Controller Layout", layout_desc, "LAYOUT", *selected_item == 4, false);
    vita_draw_slider_item(card_x, start_y + 5.0f * (item_h + 6.0f), card_w, item_h, "Floppy / HDF Sound Volume", mainMenu_diskSoundVolume, 0, 100, "%", *selected_item == 5);
    vita_draw_switch_item(card_x, start_y + 6.0f * (item_h + 6.0f), card_w, item_h, "WHDLoad Auto-Eject Floppy on Launch", mainMenu_autoEjectFloppy == 1, *selected_item == 6);
}

static bool vita_savestate_file_exists(const char *path)
{
    if (!path || path[0] == '\0') return false;
    FILE *file = fopen(path, "rb");
    if (!file) return false;
    fclose(file);
    return true;
}

static const char *vita_get_active_game_path(void)
{
    if (uae4all_image_file0[0] != '\0') return uae4all_image_file0;
    if (mainMenu_bootHD == 2) {
        if (uae4all_hard_file0[0] != '\0') return uae4all_hard_file0;
        if (uae4all_hard_file1[0] != '\0') return uae4all_hard_file1;
        if (uae4all_hard_file2[0] != '\0') return uae4all_hard_file2;
        if (uae4all_hard_file3[0] != '\0') return uae4all_hard_file3;
    }
    if (mainMenu_bootHD == 1 && uae4all_hard_dir[0] != '\0') return uae4all_hard_dir;
    if (uae4all_image_file1[0] != '\0') return uae4all_image_file1;
    if (uae4all_image_file2[0] != '\0') return uae4all_image_file2;
    if (uae4all_image_file3[0] != '\0') return uae4all_image_file3;
    return NULL;
}

static void vita_get_game_label(char *destination, size_t destination_size)
{
    if (!destination || destination_size == 0) return;
    const char *path = vita_get_active_game_path();
    const char *name = get_filename_only(path);
    strncpy(destination, name, destination_size - 1);
    destination[destination_size - 1] = '\0';
    char *extension = strrchr(destination, '.');
    if (extension && extension != destination)
        *extension = '\0';
    if (destination[0] == '\0')
        strncpy(destination, "No game loaded", destination_size - 1);
    destination[destination_size - 1] = '\0';
}

static void vita_get_savestate_paths(int slot, char *state_path, char *thumb_path)
{
    int old_slot = saveMenu_n_savestate;
    saveMenu_n_savestate = slot;
    make_savestate_filenames(state_path, thumb_path);
    saveMenu_n_savestate = old_slot;
}

void vita_view_savestates(VitaInputState *input, int *selected_item)
{
    const int total_items = 4;
    if (*selected_item < 0) *selected_item = 0;
    if (*selected_item >= total_items) *selected_item = total_items - 1;

    if (input->pressed & SCE_CTRL_LEFT) {
        (*selected_item)--;
        if (*selected_item < 0) *selected_item = total_items - 1;
    }
    if (input->pressed & SCE_CTRL_RIGHT) {
        (*selected_item)++;
        if (*selected_item >= total_items) *selected_item = 0;
    }

    if (input->pressed & SCE_CTRL_CROSS) {
        if (emulating) {
            saveMenu_n_savestate = *selected_item + 1;
            make_savestate_filenames(savestate_filename, screenshot_filename);
            savestate_state = STATE_DOSAVE;
            vita_show_message_box("Save State", "Save queued. The state will be written when the game resumes.", "Resume (X)");
            mainMenu_case = MAIN_MENU_CASE_RUN;
        } else {
            vita_show_message_box("Savestate Notice", "You must launch a game before saving state.", "OK (X)");
        }
    }

    if (input->pressed & SCE_CTRL_SQUARE) {
        if (emulating) {
            saveMenu_n_savestate = *selected_item + 1;
            make_savestate_filenames(savestate_filename, screenshot_filename);
            if (!vita_savestate_file_exists(savestate_filename)) {
                vita_show_message_box("Load State", "No save state exists in this slot.", "OK (X)");
            } else {
                savestate_state = STATE_DORESTORE;
                vita_show_message_box("Load State", "State queued. It will be restored when the game resumes.", "Resume (X)");
                mainMenu_case = MAIN_MENU_CASE_RUN;
            }
        } else {
            vita_show_message_box("Savestate Notice", "You must launch a game before loading state.", "OK (X)");
        }
    }

    char game_label[128];
    if (mainMenu_whdload_game[0] != '\0') {
        strncpy(game_label, mainMenu_whdload_game, sizeof(game_label) - 1);
        game_label[sizeof(game_label) - 1] = '\0';
    } else {
        vita_get_game_label(game_label, sizeof(game_label));
    }

    static SDL_Surface *s_slot_thumbs[4] = { NULL, NULL, NULL, NULL };
    static char s_slot_thumb_paths[4][256] = { {0}, {0}, {0}, {0} };

    float slot_w = (VITA_SCREEN_W - 40.0f - 36.0f) / 4.0f;
    float slot_h = 320.0f;
    float start_x = 20.0f;
    float start_y = 94.0f;

    for (int i = 0; i < 4; i++) {
        float sx = start_x + (float)i * (slot_w + 12.0f);
        bool focused = (*selected_item == i);
        char state_path[256];
        char thumb_path[256];
        vita_get_savestate_paths(i + 1, state_path, thumb_path);
        bool saved = vita_savestate_file_exists(state_path);

        vita_draw_card(sx, start_y, slot_w, slot_h, focused, saved);

        char slot_tag[32];
        snprintf(slot_tag, sizeof(slot_tag), "SLOT %d", i + 1);
        vita_draw_badge(sx + 14.0f, start_y + 14.0f, slot_tag, focused ? VITA_COLOR_AMIGA_RED : RGBA8(40, 50, 70, 255), VITA_COLOR_TEXT_WHITE);

        char game_label_buf[128];
        vita_truncate_text(game_label, slot_w - 24.0f, 0.62f, game_label_buf, sizeof(game_label_buf));
        vita_draw_text_centered(sx + (slot_w * 0.5f), start_y + 39.0f,
            VITA_COLOR_TEXT_MUTED, 0.62f, game_label_buf);

        float thumb_x = sx + 12.0f;
        float thumb_y = start_y + 56.0f;
        float thumb_w = slot_w - 24.0f;
        float thumb_h = 148.0f;

        vita_draw_rounded_rect(thumb_x, thumb_y, thumb_w, thumb_h, 6.0f, RGBA8(12, 15, 22, 255));

        if (saved && thumb_path[0] != '\0' &&
            (s_slot_thumb_paths[i][0] == '\0' || strcmp(s_slot_thumb_paths[i], thumb_path) != 0)) {
            if (s_slot_thumbs[i]) {
                SDL_FreeSurface(s_slot_thumbs[i]);
                s_slot_thumbs[i] = NULL;
            }
            strncpy(s_slot_thumb_paths[i], thumb_path, 255);
            s_slot_thumb_paths[i][255] = '\0';
            FILE *tf = fopen(thumb_path, "rb");
            if (tf) {
                fclose(tf);
                s_slot_thumbs[i] = IMG_Load(thumb_path);
            }
        }
        if (!saved) {
            if (s_slot_thumbs[i]) {
                SDL_FreeSurface(s_slot_thumbs[i]);
                s_slot_thumbs[i] = NULL;
            }
            s_slot_thumb_paths[i][0] = '\0';
        }

        if (s_slot_thumbs[i]) {
            float surf_w = (float)s_slot_thumbs[i]->w;
            float surf_h = (float)s_slot_thumbs[i]->h;
            float scale_x = thumb_w / surf_w;
            float scale_y = thumb_h / surf_h;
            float final_scale = (scale_x < scale_y) ? scale_x : scale_y;
            if (final_scale > 1.0f) final_scale = 1.0f;
            if (final_scale < 0.05f) final_scale = 0.05f;

            int draw_w = (int)(surf_w * final_scale);
            int draw_h = (int)(surf_h * final_scale);
            int draw_x = (int)(thumb_x + (thumb_w - draw_w) * 0.5f);
            int draw_y = (int)(thumb_y + (thumb_h - draw_h) * 0.5f);

            SDL_Rect dst_r = { (Sint16)draw_x, (Sint16)draw_y, (Uint16)draw_w, (Uint16)draw_h };
            SDL_SoftStretch(s_slot_thumbs[i], NULL, prSDLScreen, &dst_r);
        } else {
            vita_draw_boing_ball_icon(thumb_x + (thumb_w * 0.5f), thumb_y + 58.0f, 26.0f, 0.0f);
            vita_draw_text_centered(thumb_x + (thumb_w * 0.5f), thumb_y + 105.0f,
                saved ? VITA_COLOR_SUCCESS : VITA_COLOR_TEXT_MUTED, 0.90f,
                saved ? "SAVED" : "EMPTY");
        }

        float action_x = sx + 10.0f;
        float action_w = slot_w - 20.0f;
        float save_y = start_y + 215.0f;
        float load_y = start_y + 260.0f;

        vita_draw_card(action_x, save_y, action_w, 38.0f, focused, false);
        vita_draw_button_glyph(action_x + 12.0f, save_y + 8.0f, VITA_BTN_CROSS);
        vita_draw_text(action_x + 48.0f, save_y + 11.0f,
            focused ? VITA_COLOR_TEXT_WHITE : VITA_COLOR_TEXT_MUTED, 0.90f, "SAVE STATE");

        vita_draw_card(action_x, load_y, action_w, 38.0f, false, false);
        vita_draw_button_glyph(action_x + 12.0f, load_y + 8.0f, VITA_BTN_SQUARE);
        vita_draw_text(action_x + 48.0f, load_y + 11.0f,
            VITA_COLOR_TEXT_MUTED, 0.90f, "LOAD STATE");
    }
}

void vita_view_system(VitaInputState *input, int *selected_item)
{
    const int total_items = 5;
    if (*selected_item < 0) *selected_item = 0;
    if (*selected_item >= total_items) *selected_item = total_items - 1;

    if (input->pressed & SCE_CTRL_UP) {
        (*selected_item)--;
        if (*selected_item < 0) *selected_item = total_items - 1;
    }
    if (input->pressed & SCE_CTRL_DOWN) {
        (*selected_item)++;
        if (*selected_item >= total_items) *selected_item = 0;
    }

    if (input->pressed & SCE_CTRL_CROSS) {
        switch (*selected_item) {
            case 0:
                if (uae4all_image_file0[0]) {
                    saveconfig(0);
                    saveconfig(1);
                } else {
                    saveconfig(1);
                }
                vita_show_message_box("Config Saved", "Configuration saved successfully to ux0:/data/uae4all/conf/", "OK (X)");
                break;
            case 1:
                if (vita_show_confirm_box("Reset Settings", "Restore default settings for all parameters?", "Yes", "No")) {
                    SetDefaultMenuSettings(1);
                    int default_kickstart_loaded = vita_set_kickstart(kickstart, 1);
                    bReloadKickstart = 1;
                    gui_update();
                    if (default_kickstart_loaded)
                        vita_show_message_box("Settings Reset", "Default settings restored successfully.", "OK (X)");
                    else
                        vita_show_message_box("Settings Reset", "Defaults restored, but the default Kickstart ROM is missing.", "OK (X)");
                }
                break;
            case 2:
                mainMenu_case = MAIN_MENU_CASE_RESET;
                break;
            case 3:
                if (emulating) {
                    vita_screenshot_request = 1;
                    mainMenu_case = MAIN_MENU_CASE_RUN;
                } else {
                    vita_show_message_box("Screenshot", "Launch a game before taking a screenshot.", "OK (X)");
                }
                break;
            case 4:
                if (vita_show_confirm_box("About", "Open UAE4All2 and credits?", "Yes", "No")) {
                    vita_show_about_box();
                }
                break;
        }
    }

    float card_x = 20.0f;
    float card_w = VITA_SCREEN_W - 40.0f;
    float start_y = 90.0f;
    float item_h = 56.0f;

    vita_draw_button_item(card_x, start_y + 0.0f * (item_h + 12.0f), card_w, item_h, "Save Game Configuration", "Save all disk, display, and hardware settings for current game", "SAVE", *selected_item == 0, false);
    vita_draw_button_item(card_x, start_y + 1.0f * (item_h + 12.0f), card_w, item_h, "Restore Default Settings", "Reset all emulator configurations to factory defaults", "RESET", *selected_item == 1, false);
    vita_draw_button_item(card_x, start_y + 2.0f * (item_h + 12.0f), card_w, item_h, "Reboot Amiga Emulation", "Hard reset the Amiga virtual machine with current settings", "REBOOT", *selected_item == 2, false);
    vita_draw_button_item(card_x, start_y + 3.0f * (item_h + 12.0f), card_w, item_h, "Take Screenshot", "Capture the next emulated frame as a PNG in the screenshots folder", "SHOT", *selected_item == 3, false);
    vita_draw_button_item(card_x, start_y + 4.0f * (item_h + 12.0f), card_w, item_h, "About UAE4All2", "Credits, original authors, contributors and project acknowledgements", "ABOUT", *selected_item == 4, false);
}

#endif
