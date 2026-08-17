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

#include "uae_gui_vita.h"

extern char uae4all_image_file0[256];
extern char uae4all_image_file1[256];
extern char uae4all_image_file2[256];
extern char uae4all_image_file3[256];
extern char uae4all_hard_file0[256];
extern char uae4all_hard_file1[256];
extern char uae4all_hard_file2[256];
extern char uae4all_hard_file3[256];
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
extern int moveY;
extern int mainMenu_soundStereo;
extern int mainMenu_soundStereoSep;
extern unsigned int sound_rate;
extern int saveMenu_n_savestate;
extern char *savestate_filename;
extern char *screenshot_filename;
extern int savestate_state;
extern int presetModeId;
extern int mainMenu_case;
extern int emulating;

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

void vita_view_hard_disk(VitaInputState *input, int *selected_item)
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
                mainMenu_bootHD = 2;
                reset_hdConf();
                gui_update();
            } else if (res == 2) {
                hdf_files[*selected_item][0] = 0;
                reset_hdConf();
                gui_update();
            }
        } else if (*selected_item == 4) {
            mainMenu_bootHD = (mainMenu_bootHD + 1) % 3;
            reset_hdConf();
        } else if (*selected_item == 5) {
            for (int i = 0; i < 4; i++)
                hdf_files[i][0] = 0;
            mainMenu_bootHD = 0;
            reset_hdConf();
            gui_update();
        }
    }

    if ((input->pressed & SCE_CTRL_TRIANGLE) && *selected_item < 4) {
        hdf_files[*selected_item][0] = 0;
        reset_hdConf();
        gui_update();
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

void vita_view_presets(VitaInputState *input, int *selected_item)
{
    const int total_items = 4;
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
            mainMenu_CPU_model = 0; /* 68000 */
            mainMenu_chipset = 0;
            mainMenu_chipMemory = 1;
            mainMenu_slowMemory = 1;
            mainMenu_fastMemory = 0;
            UpdateCPUModelSettings();
            UpdateMemorySettings();
            UpdateChipsetSettings();
            snprintf(romfile, 256, "%s/kickstarts/%s", launchDir, kickstarts_rom_names[kickstart]);
            bReloadKickstart = 1;
            uae4all_init_rom(romfile);
            saveconfig(1);
            vita_show_message_box("Preset Applied", "Amiga 500 (OCS 1.3, 512K+512K RAM) configured!", "OK (X)");
        } else if (*selected_item == 1) {
            kickstart = 2;
            mainMenu_CPU_model = 0; /* 68000 */
            mainMenu_chipset = 1;
            mainMenu_chipMemory = 2;
            mainMenu_slowMemory = 0;
            mainMenu_fastMemory = 1;
            UpdateCPUModelSettings();
            UpdateMemorySettings();
            UpdateChipsetSettings();
            snprintf(romfile, 256, "%s/kickstarts/%s", launchDir, kickstarts_rom_names[kickstart]);
            bReloadKickstart = 1;
            uae4all_init_rom(romfile);
            saveconfig(1);
            vita_show_message_box("Preset Applied", "Amiga 500+ (ECS 2.04, 1MB+1MB RAM) configured!", "OK (X)");
        } else if (*selected_item == 2) {
            kickstart = 3;
            mainMenu_CPU_model = 1; /* 68020 */
            mainMenu_chipset = 2;
            mainMenu_chipMemory = 2;
            mainMenu_slowMemory = 0;
            mainMenu_fastMemory = 3;
            UpdateCPUModelSettings();
            UpdateMemorySettings();
            UpdateChipsetSettings();
            snprintf(romfile, 256, "%s/kickstarts/%s", launchDir, kickstarts_rom_names[kickstart]);
            bReloadKickstart = 1;
            uae4all_init_rom(romfile);
            saveconfig(1);
            vita_show_message_box("Preset Applied", "Amiga 1200 (AGA 3.1, 68020 2MB+4MB RAM) configured!", "OK (X)");
        } else if (*selected_item == 3) {
            kickstart = 6;
            mainMenu_CPU_model = 1;
            mainMenu_chipset = 2;
            mainMenu_chipMemory = 2;
            mainMenu_slowMemory = 0;
            mainMenu_fastMemory = 0;
            UpdateCPUModelSettings();
            UpdateMemorySettings();
            UpdateChipsetSettings();
            snprintf(romfile, 256, "%s/kickstarts/%s", launchDir, kickstarts_rom_names[kickstart]);
            snprintf(extfile, 256, "%s/kickstarts/%s", launchDir, extended_rom_names[kickstart]);
            bReloadKickstart = 1;
            uae4all_init_rom(romfile);
            saveconfig(1);
            vita_show_message_box("Preset Applied", "Amiga CD32 (Akiko, CD32 Kickstart + Extended ROM) configured!", "OK (X)");
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
    } presets[4] = {
        { "Amiga 500 (Classic OCS 1.3)", "68000 7MHz | Kickstart 1.3 | 512KB Chip + 512KB Slow RAM", "OCS", "Recommended for 95% of classic Amiga games (1985-1993)" },
        { "Amiga 500+ (Enhanced ECS 2.04)", "68000 7MHz | Kickstart 2.04 | 1MB Chip + 1MB Fast RAM", "ECS", "Recommended for late ECS titles and productivity software" },
        { "Amiga 1200 (Advanced AGA 3.1)", "68020 14MHz | Kickstart 3.1 | 2MB Chip + 4MB Fast RAM", "AGA", "Recommended for AGA games (Alien Breed 3D, Slam Tilt, Gloom)" },
        { "Amiga CD32 (Console CD Mode)", "68020 14MHz | Kickstart 3.1 CD32 | 2MB Chip RAM + Akiko", "CD32", "Recommended for Amiga CD32 ISO and CUE disc images" }
    };

    for (int i = 0; i < 4; i++) {
        float cy = start_y + (float)i * (item_h + 10.0f);
        bool focused = (*selected_item == i);

        vita_draw_card(card_x, cy, card_w, item_h, focused, false);

        unsigned int badge_col = (i == 2) ? VITA_COLOR_AMIGA_RED : ((i == 0) ? VITA_COLOR_AMIGA_BLUE : RGBA8(40, 50, 70, 255));
        float badge_x = card_x + ((i == 3) ? 8.0f : 14.0f);
        vita_draw_badge(badge_x, cy + 14.0f, presets[i].tag, badge_col, VITA_COLOR_TEXT_WHITE);

        vita_draw_text(card_x + 72.0f, cy + 12.0f, focused ? VITA_COLOR_TEXT_WHITE : RGBA8(230, 240, 255, 255), 1.05f, presets[i].title);

        char desc_buf[256];
        vita_truncate_text(presets[i].desc, card_w - 180.0f, 0.85f, desc_buf, sizeof(desc_buf));
        vita_draw_text(card_x + 72.0f, cy + 34.0f, VITA_COLOR_TEXT_MUTED, 0.85f, desc_buf);

        char rec_buf[256];
        vita_truncate_text(presets[i].recom, card_w - 180.0f, 0.82f, rec_buf, sizeof(rec_buf));
        vita_draw_text(card_x + 72.0f, cy + 52.0f, VITA_COLOR_AMIGA_ORANGE, 0.82f, rec_buf);

        if (focused) {
            vita_draw_hint_item(card_x + card_w - 140.0f, cy + 28.0f, VITA_BTN_CROSS, "Apply");
        }
    }
}

void vita_view_hardware(VitaInputState *input, int *selected_item)
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
                kickstart = (kickstart + dir + 7) % 7;
                snprintf(romfile, 256, "%s/kickstarts/%s", launchDir, kickstarts_rom_names[kickstart]);
                if (extended_rom_names[kickstart][0] != '\0')
                    snprintf(extfile, 256, "%s/kickstarts/%s", launchDir, extended_rom_names[kickstart]);
                else
                    extfile[0] = '\0';
                bReloadKickstart = 1;
                uae4all_init_rom(romfile);
                break;
            case 1:
                mainMenu_CPU_model = (mainMenu_CPU_model + dir + 2) % 2;
                UpdateCPUModelSettings();
                break;
            case 2:
                mainMenu_chipset = (mainMenu_chipset + dir + 3) % 3;
                UpdateChipsetSettings();
                break;
            case 3:
                mainMenu_chipMemory = (mainMenu_chipMemory + dir + 4) % 4;
                UpdateMemorySettings();
                break;
            case 4:
                mainMenu_fastMemory = (mainMenu_fastMemory + dir + 5) % 5;
                UpdateMemorySettings();
                break;
            case 5:
                mainMenu_slowMemory = (mainMenu_slowMemory + dir + 4) % 4;
                UpdateMemorySettings();
                break;
        }
    }

    float card_x = 20.0f;
    float card_w = VITA_SCREEN_W - 40.0f;
    float start_y = 90.0f;
    float item_h = 50.0f;

    const char *ks_names[7] = { "Kickstart 1.2", "Kickstart 1.3 (A500)", "Kickstart 2.04 (A500+)", "Kickstart 3.1 (A1200)", "Custom ROM", "AROS ROM", "Kickstart CD32" };
    const char *cpu_names[2] = { "Motorola 68000 (7 MHz)", "Motorola 68020 (14 MHz AGA)" };
    const char *chipset_names[3] = { "OCS (Original Chip Set)", "ECS (Enhanced Chip Set)", "AGA (Advanced Graphics)" };
    const char *chip_ram_names[4] = { "512 KB (Standard)", "1 MB", "2 MB (Expanded)", "None" };
    const char *fast_ram_names[5] = { "None", "1 MB", "2 MB", "4 MB (AGA recommended)", "8 MB" };
    const char *slow_ram_names[4] = { "None", "512 KB (Trapdoor)", "1 MB", "1.5 MB" };

    vita_draw_selector_item(card_x, start_y + 0.0f * (item_h + 8.0f), card_w, item_h, "Kickstart ROM", ks_names[kickstart % 7], *selected_item == 0);
    vita_draw_selector_item(card_x, start_y + 1.0f * (item_h + 8.0f), card_w, item_h, "CPU Architecture", cpu_names[mainMenu_CPU_model % 2], *selected_item == 1);
    vita_draw_selector_item(card_x, start_y + 2.0f * (item_h + 8.0f), card_w, item_h, "Amiga Chipset", chipset_names[mainMenu_chipset % 3], *selected_item == 2);
    vita_draw_selector_item(card_x, start_y + 3.0f * (item_h + 8.0f), card_w, item_h, "Chip RAM", chip_ram_names[mainMenu_chipMemory % 4], *selected_item == 3);
    vita_draw_selector_item(card_x, start_y + 4.0f * (item_h + 8.0f), card_w, item_h, "Fast RAM", fast_ram_names[mainMenu_fastMemory % 5], *selected_item == 4);
    vita_draw_selector_item(card_x, start_y + 5.0f * (item_h + 8.0f), card_w, item_h, "Slow / Trapdoor RAM", slow_ram_names[mainMenu_slowMemory % 4], *selected_item == 5);
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
                mainMenu_showStatus = (mainMenu_showStatus + dir + 3) % 3;
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
    const char *status_names[3] = { "Bottom Bar (Floppy LED & FPS)", "Top Bar", "Disabled (Clean Screen)" };

    vita_draw_selector_item(card_x, start_y + 0.0f * (item_h + 6.0f), card_w, item_h, "Hardware Vita Shader", shader_names[mainMenu_shader % 9], *selected_item == 0);
    vita_draw_selector_item(card_x, start_y + 1.0f * (item_h + 6.0f), card_w, item_h, "Screen Refresh & Region", ntsc_names[mainMenu_ntsc % 2], *selected_item == 1);
    vita_draw_selector_item(card_x, start_y + 2.0f * (item_h + 6.0f), card_w, item_h, "Status Bar (Floppy LED/FPS)", status_names[mainMenu_showStatus % 3], *selected_item == 2);
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
    const int total_items = 4;
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
                break;
        }
    }

    float card_x = 20.0f;
    float card_w = VITA_SCREEN_W - 40.0f;
    float start_y = 90.0f;
    float item_h = 56.0f;

    const char *touch_modes[3] = { "Direct Touchscreen (Finger)", "Relative Trackpad Cursor", "Disabled" };
    const char *autofire_names[4] = { "Off", "Slow (1)", "Medium (2)", "Turbo (3)" };

    vita_draw_switch_item(card_x, start_y + 0.0f * (item_h + 10.0f), card_w, item_h, "Right Analog Stick as Amiga Mouse", mainMenu_leftStickMouse == 1, *selected_item == 0);
    vita_draw_selector_item(card_x, start_y + 1.0f * (item_h + 10.0f), card_w, item_h, "PS Vita Touchscreen Mode", touch_modes[mainMenu_touchControls % 3], *selected_item == 1);
    vita_draw_selector_item(card_x, start_y + 2.0f * (item_h + 10.0f), card_w, item_h, "Autofire Rate", autofire_names[mainMenu_autofire % 4], *selected_item == 2);
    vita_draw_button_item(card_x, start_y + 3.0f * (item_h + 10.0f), card_w, item_h, "Physical Controller Layout", "Cross = Fire 1 | Circle = Fire 2 | Square = Space | Triangle = Virtual Keyboard", "LAYOUT", *selected_item == 3, false);
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
    vita_get_game_label(game_label, sizeof(game_label));

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
        vita_draw_boing_ball_icon(thumb_x + (thumb_w * 0.5f), thumb_y + 58.0f, 26.0f, 0.0f);
        vita_draw_text_centered(thumb_x + (thumb_w * 0.5f), thumb_y + 105.0f,
            saved ? VITA_COLOR_SUCCESS : VITA_COLOR_TEXT_MUTED, 0.90f,
            saved ? "SAVED" : "EMPTY");

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
    const int total_items = 4;
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
                    vita_show_message_box("Settings Reset", "Default settings restored successfully.", "OK (X)");
                }
                break;
            case 2:
                mainMenu_case = MAIN_MENU_CASE_RESET;
                break;
            case 3:
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
    vita_draw_button_item(card_x, start_y + 3.0f * (item_h + 12.0f), card_w, item_h, "About UAE4All2", "Credits, original authors, contributors and project acknowledgements", "ABOUT", *selected_item == 3, false);
}

#endif
