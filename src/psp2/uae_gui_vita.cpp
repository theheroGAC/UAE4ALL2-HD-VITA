#ifdef __PSP2__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <time.h>
#include <SDL.h>
#include <SDL_image.h>

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
#include "cdrom.h"

#include "uae_gui_vita.h"

extern int screenWidth;
extern int mainMenu_case;
extern int bReloadKickstart;
extern int oldkickstart;
extern int kickstart;
extern int emulating;
extern int inside_menu;
extern bool resetOnStartingApp;
extern char launchDir[300];
extern char currentDir[300];
extern char uae4all_image_file0[256];
extern char uae4all_image_file1[256];
extern char uae4all_image_file2[256];
extern char uae4all_image_file3[256];
extern void setCpuSpeed(void);
extern int displaying_menu;
extern int kickstart_warning;

extern SDL_Surface *prSDLScreen;

#ifndef PRIVATE_HW_DATA
#define PRIVATE_HW_DATA
typedef struct private_hwdata {
    vita2d_texture *texture;
    SDL_Rect dst;
} private_hwdata;
#endif

static void vita_gui_free_screen(void)
{
    if (prSDLScreen == NULL)
        return;

    write_log("[VITA] vita_gui_free_screen: freeing %dx%d\n", prSDLScreen->w, prSDLScreen->h);
    if (prSDLScreen->hwdata != NULL) {
        private_hwdata *hw = (private_hwdata *)prSDLScreen->hwdata;
        write_log("[VITA] vita_gui_free_screen: wait rendering\n");
        vita2d_wait_rendering_done();
        write_log("[VITA] vita_gui_free_screen: rendering done\n");
        if (hw->texture != NULL) {
            vita2d_free_texture(hw->texture);
            hw->texture = NULL;
        }
        SDL_free(prSDLScreen->hwdata);
        prSDLScreen->hwdata = NULL;
        prSDLScreen->pixels = NULL;
    }
    SDL_FreeSurface(prSDLScreen);
    prSDLScreen = NULL;
    write_log("[VITA] vita_gui_free_screen: done\n");
}

static bool s_gui_initialized = false;
static float s_boing_angle = 0.0f;
static VitaGuiTab s_active_tab = VITA_TAB_FLOPPY;
static int s_tab_selected_item[VITA_TAB_COUNT] = {0};

static const unsigned char s_font_8x8[96][8] = {
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // 32 ' '
    {0x18,0x3C,0x3C,0x18,0x18,0x00,0x18,0x00}, // 33 '!'
    {0x66,0x66,0x24,0x00,0x00,0x00,0x00,0x00}, // 34 '"'
    {0x6C,0x6C,0xFE,0x6C,0xFE,0x6C,0x6C,0x00}, // 35 '#'
    {0x18,0x7E,0x18,0x3C,0x66,0x3C,0x18,0x7E}, // 36 '$'
    {0x00,0xC6,0xCC,0x18,0x30,0x66,0xC6,0x00}, // 37 '%'
    {0x38,0x6C,0x38,0x76,0xDC,0xCC,0x76,0x00}, // 38 '&'
    {0x18,0x18,0x30,0x00,0x00,0x00,0x00,0x00}, // 39 '''
    {0x0C,0x18,0x30,0x30,0x30,0x18,0x0C,0x00}, // 40 '('
    {0x30,0x18,0x0C,0x0C,0x0C,0x18,0x30,0x00}, // 41 ')'
    {0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00}, // 42 '*'
    {0x00,0x18,0x18,0x7E,0x18,0x18,0x00,0x00}, // 43 '+'
    {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x30}, // 44 ','
    {0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00}, // 45 '-'
    {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00}, // 46 '.'
    {0x06,0x0C,0x18,0x30,0x60,0xC0,0x80,0x00}, // 47 '/'
    {0x3C,0x66,0x6E,0x76,0x66,0x66,0x3C,0x00}, // 48 '0'
    {0x18,0x38,0x18,0x18,0x18,0x18,0x7E,0x00}, // 49 '1'
    {0x3C,0x66,0x06,0x0C,0x18,0x30,0x7E,0x00}, // 50 '2'
    {0x3C,0x66,0x06,0x1C,0x06,0x66,0x3C,0x00}, // 51 '3'
    {0x0C,0x1C,0x34,0x64,0x7E,0x0C,0x0C,0x00}, // 52 '4'
    {0x7E,0x60,0x7C,0x06,0x06,0x66,0x3C,0x00}, // 53 '5'
    {0x1C,0x30,0x60,0x7C,0x66,0x66,0x3C,0x00}, // 54 '6'
    {0x7E,0x06,0x0C,0x18,0x30,0x30,0x30,0x00}, // 55 '7'
    {0x3C,0x66,0x66,0x3C,0x66,0x66,0x3C,0x00}, // 56 '8'
    {0x3C,0x66,0x66,0x3E,0x06,0x0C,0x38,0x00}, // 57 '9'
    {0x00,0x18,0x18,0x00,0x18,0x18,0x00,0x00}, // 58 ':'
    {0x00,0x18,0x18,0x00,0x18,0x18,0x30,0x00}, // 59 ';'
    {0x0C,0x18,0x30,0x60,0x30,0x18,0x0C,0x00}, // 60 '<'
    {0x00,0x00,0x7E,0x00,0x7E,0x00,0x00,0x00}, // 61 '='
    {0x30,0x18,0x0C,0x06,0x0C,0x18,0x30,0x00}, // 62 '>'
    {0x3C,0x66,0x06,0x0C,0x18,0x00,0x18,0x00}, // 63 '?'
    {0x3C,0x66,0x6E,0x6E,0x60,0x62,0x3C,0x00}, // 64 '@'
    {0x18,0x3C,0x66,0x66,0x7E,0x66,0x66,0x00}, // 65 'A'
    {0x7C,0x66,0x66,0x7C,0x66,0x66,0x7C,0x00}, // 66 'B'
    {0x3C,0x66,0x60,0x60,0x60,0x66,0x3C,0x00}, // 67 'C'
    {0x78,0x6C,0x66,0x66,0x66,0x6C,0x78,0x00}, // 68 'D'
    {0x7E,0x60,0x60,0x7C,0x60,0x60,0x7E,0x00}, // 69 'E'
    {0x7E,0x60,0x60,0x7C,0x60,0x60,0x60,0x00}, // 70 'F'
    {0x3C,0x66,0x60,0x6E,0x66,0x66,0x3C,0x00}, // 71 'G'
    {0x66,0x66,0x66,0x7E,0x66,0x66,0x66,0x00}, // 72 'H'
    {0x7E,0x18,0x18,0x18,0x18,0x18,0x7E,0x00}, // 73 'I'
    {0x06,0x06,0x06,0x06,0x06,0x66,0x3C,0x00}, // 74 'J'
    {0x66,0x6C,0x78,0x70,0x78,0x6C,0x66,0x00}, // 75 'K'
    {0x60,0x60,0x60,0x60,0x60,0x60,0x7E,0x00}, // 76 'L'
    {0x63,0x77,0x7F,0x6B,0x63,0x63,0x63,0x00}, // 77 'M'
    {0x66,0x76,0x7E,0x7E,0x6E,0x66,0x66,0x00}, // 78 'N'
    {0x3C,0x66,0x66,0x66,0x66,0x66,0x3C,0x00}, // 79 'O'
    {0x7C,0x66,0x66,0x7C,0x60,0x60,0x60,0x00}, // 80 'P'
    {0x3C,0x66,0x66,0x66,0x6E,0x3C,0x0E,0x00}, // 81 'Q'
    {0x7C,0x66,0x66,0x7C,0x78,0x6C,0x66,0x00}, // 82 'R'
    {0x3C,0x66,0x60,0x3C,0x06,0x66,0x3C,0x00}, // 83 'S'
    {0x7E,0x18,0x18,0x18,0x18,0x18,0x18,0x00}, // 84 'T'
    {0x66,0x66,0x66,0x66,0x66,0x66,0x3C,0x00}, // 85 'U'
    {0x66,0x66,0x66,0x66,0x66,0x3C,0x18,0x00}, // 86 'V'
    {0x63,0x63,0x63,0x6B,0x7F,0x77,0x63,0x00}, // 87 'W'
    {0x66,0x66,0x3C,0x18,0x3C,0x66,0x66,0x00}, // 88 'X'
    {0x66,0x66,0x66,0x3C,0x18,0x18,0x18,0x00}, // 89 'Y'
    {0x7E,0x06,0x0C,0x18,0x30,0x60,0x7E,0x00}, // 90 'Z'
    {0x3C,0x30,0x30,0x30,0x30,0x30,0x3C,0x00}, // 91 '['
    {0xC0,0x60,0x30,0x18,0x0C,0x06,0x02,0x00}, // 92 '\'
    {0x3C,0x0C,0x0C,0x0C,0x0C,0x0C,0x3C,0x00}, // 93 ']'
    {0x18,0x3C,0x66,0x00,0x00,0x00,0x00,0x00}, // 94 '^'
    {0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00}, // 95 '_'
    {0x30,0x18,0x0C,0x00,0x00,0x00,0x00,0x00}, // 96 '`'
    {0x00,0x00,0x3C,0x06,0x3E,0x66,0x3E,0x00}, // 97 'a'
    {0x60,0x60,0x7C,0x66,0x66,0x66,0x7C,0x00}, // 98 'b'
    {0x00,0x00,0x3C,0x66,0x60,0x66,0x3C,0x00}, // 99 'c'
    {0x06,0x06,0x3E,0x66,0x66,0x66,0x3E,0x00}, // 100 'd'
    {0x00,0x00,0x3C,0x66,0x7E,0x60,0x3C,0x00}, // 101 'e'
    {0x1C,0x30,0x7C,0x30,0x30,0x30,0x30,0x00}, // 102 'f'
    {0x00,0x00,0x3E,0x66,0x66,0x3E,0x06,0x3C}, // 103 'g'
    {0x60,0x60,0x7C,0x66,0x66,0x66,0x66,0x00}, // 104 'h'
    {0x18,0x00,0x38,0x18,0x18,0x18,0x3C,0x00}, // 105 'i'
    {0x0C,0x00,0x1C,0x0C,0x0C,0x0C,0x6C,0x38}, // 106 'j'
    {0x60,0x60,0x66,0x6C,0x78,0x6C,0x66,0x00}, // 107 'k'
    {0x38,0x18,0x18,0x18,0x18,0x18,0x3C,0x00}, // 108 'l'
    {0x00,0x00,0x66,0x7F,0x7B,0x63,0x63,0x00}, // 109 'm'
    {0x00,0x00,0x7C,0x66,0x66,0x66,0x66,0x00}, // 110 'n'
    {0x00,0x00,0x3C,0x66,0x66,0x66,0x3C,0x00}, // 111 'o'
    {0x00,0x00,0x7C,0x66,0x66,0x7C,0x60,0x60}, // 112 'p'
    {0x00,0x00,0x3E,0x66,0x66,0x3E,0x06,0x06}, // 113 'q'
    {0x00,0x00,0x7C,0x66,0x60,0x60,0x60,0x00}, // 114 'r'
    {0x00,0x00,0x3E,0x60,0x3C,0x06,0x7C,0x00}, // 115 's'
    {0x18,0x18,0x7E,0x18,0x18,0x18,0x0C,0x00}, // 116 't'
    {0x00,0x00,0x66,0x66,0x66,0x66,0x3E,0x00}, // 117 'u'
    {0x00,0x00,0x66,0x66,0x66,0x3C,0x18,0x00}, // 118 'v'
    {0x00,0x00,0x63,0x6B,0x7F,0x77,0x36,0x00}, // 119 'w'
    {0x00,0x00,0x66,0x3C,0x18,0x3C,0x66,0x00}, // 120 'x'
    {0x00,0x00,0x66,0x66,0x66,0x3E,0x06,0x3C}, // 121 'y'
    {0x00,0x00,0x7E,0x0C,0x18,0x30,0x7E,0x00}, // 122 'z'
    {0x0E,0x18,0x18,0x70,0x18,0x18,0x0E,0x00}, // 123 '{'
    {0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x00}, // 124 '|'
    {0x70,0x18,0x18,0x0E,0x18,0x18,0x70,0x00}, // 125 '}'
    {0x76,0xDC,0x00,0x00,0x00,0x00,0x00,0x00}, // 126 '~'
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}  // 127
};

static const unsigned char s_char_widths[96] = {
    4, 3, 5, 7, 7, 7, 7, 3, 4, 4, 6, 7, 3, 6, 3, 6, // 32-47
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 3, 3, 5, 7, 5, 7, // 48-63
    7, 7, 7, 7, 7, 7, 7, 7, 7, 4, 6, 7, 6, 8, 7, 7, // 64-79
    7, 7, 7, 7, 7, 7, 7, 8, 7, 7, 7, 4, 6, 4, 5, 7, // 80-95
    3, 6, 6, 6, 6, 6, 5, 6, 3, 4, 6, 3, 8, 6, 6, 6, // 96-111
    6, 5, 6, 5, 6, 6, 8, 6, 6, 6, 4, 3, 4, 7, 0      // 112-127
};

static const char *s_tab_names[VITA_TAB_COUNT] = {
    "Floppy",
    "Hard Disk",
    "WHDLoad",
    "Presets",
    "Hardware",
    "Display",
    "Controls",
    "Savestate",
    "System"
};

int vita_gui_init(void)
{
    write_log("[VITA] vita_gui_init: entry\n");
    if (s_gui_initialized && prSDLScreen != NULL && prSDLScreen->w == VITA_SCREEN_W && prSDLScreen->h == VITA_SCREEN_H)
        return 0;

    /* Release any previous (game) surface before switching to the menu mode */
    vita_gui_free_screen();
    write_log("[VITA] vita_gui_init: free_screen done\n");

    displaying_menu = 1;

    /* Set up native 960x544 full HD screen mode */
    prSDLScreen = SDL_SetVideoMode(VITA_SCREEN_W, VITA_SCREEN_H, 16, SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (prSDLScreen == NULL) {
        /* Fall back to a software surface if the hardware mode failed */
        prSDLScreen = SDL_SetVideoMode(VITA_SCREEN_W, VITA_SCREEN_H, 16, SDL_SWSURFACE | SDL_DOUBLEBUF);
    }
    if (prSDLScreen == NULL) {
        write_log("[VITA] vita_gui_init: SDL_SetVideoMode failed\n");
        return -1;
    }
    SDL_SetVideoModeScaling(0, 0, VITA_SCREEN_W, VITA_SCREEN_H);
    SDL_SetVideoModeBilinear(1);

    sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_START);
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);

    write_log("[VITA] vita_gui_init: menu screen ready (%dx%d)\n", prSDLScreen->w, prSDLScreen->h);
    s_gui_initialized = true;
    return 0;
}

void vita_gui_shutdown(void)
{
    /* Keep the menu surface alive until update_display() performs the SDL-Vita
       mode transition. That transition must drain the old video surface before
       asking SDL for the game framebuffer. */
    s_gui_initialized = false;
}

void vita_gui_shutdown_final(void)
{
    /* Used only when the emulator is quitting before the emulation loop has
       started. There will be no SDL teardown after gui_init returns. */
    s_gui_initialized = false;
    vita_gui_free_screen();
    displaying_menu = 0;
}

void vita_gui_prepare_exit(void)
{
    /* During emulation, graphics_leave() still owns SDL's video surface. Free
       only the Vita texture here; freeing SDL_Surface as well would leave
       SDL_VideoSurface dangling and make SDL_VideoQuit crash. */
    if (prSDLScreen && prSDLScreen->hwdata) {
        private_hwdata *hw = (private_hwdata *)prSDLScreen->hwdata;
        vita2d_wait_rendering_done();
        if (hw->texture) {
            vita2d_free_texture(hw->texture);
            hw->texture = NULL;
        }
        SDL_free(prSDLScreen->hwdata);
        prSDLScreen->hwdata = NULL;
        prSDLScreen->pixels = NULL;
    }
    s_gui_initialized = false;
    displaying_menu = 0;
}

void vita_gui_update_input(VitaInputState *input)
{
    input->prev_pad = input->pad;
    sceCtrlPeekBufferPositive(0, &input->pad, 1);

    /* Convert left analog stick to D-pad if tilted significantly */
    if (input->pad.ly < 50)  input->pad.buttons |= SCE_CTRL_UP;
    if (input->pad.ly > 205) input->pad.buttons |= SCE_CTRL_DOWN;
    if (input->pad.lx < 50)  input->pad.buttons |= SCE_CTRL_LEFT;
    if (input->pad.lx > 205) input->pad.buttons |= SCE_CTRL_RIGHT;

    input->pressed  = input->pad.buttons & ~input->prev_pad.buttons;
    input->released = ~input->pad.buttons & input->prev_pad.buttons;
    input->held     = input->pad.buttons;

    /* Touch screen polling */
    SceTouchData touch;
    sceTouchPeek(SCE_TOUCH_PORT_FRONT, &touch, 1);

    if (touch.reportNum > 0) {
        input->touch_x = (touch.report[0].x * VITA_SCREEN_W) / 1920;
        input->touch_y = (touch.report[0].y * VITA_SCREEN_H) / 1088;
        input->touch_active = 1;
        input->touch_hold_frames++;
        input->touch_tap = (input->touch_hold_frames == 1) ? 1 : 0;
    } else {
        input->touch_active = 0;
        input->touch_hold_frames = 0;
        input->touch_tap = 0;
    }
}

void vita_gui_update_system_info(VitaSystemInfo *sysinfo)
{
    SceDateTime dt;
    sceRtcGetCurrentClockLocalTime(&dt);
    snprintf(sysinfo->time_str, sizeof(sysinfo->time_str), "%02d:%02d", dt.hour, dt.minute);
    snprintf(sysinfo->date_str, sizeof(sysinfo->date_str), "%02d/%02d/%04d", dt.day, dt.month, dt.year);

    sysinfo->battery_percent = scePowerGetBatteryLifePercent();
    sysinfo->is_charging = (scePowerIsBatteryCharging() == 1);
}

static inline Uint32 to_sdl_color(unsigned int col)
{
    Uint8 r = (col >> 0) & 0xFF;
    Uint8 g = (col >> 8) & 0xFF;
    Uint8 b = (col >> 16) & 0xFF;
    return SDL_MapRGB(prSDLScreen->format, r, g, b);
}


void vita_draw_rounded_rect(float x, float y, float w, float h, float r, unsigned int color)
{
    if (!prSDLScreen || w <= 0 || h <= 0) return;
    Uint32 sdl_col = to_sdl_color(color);

    int xi = (int)x;
    int yi = (int)y;
    int wi = (int)w;
    int hi = (int)h;

    /* Clamp the corner radius to the smallest half-dimension */
    float rad = r;
    float min_half = ((wi < hi) ? (float)wi : (float)hi) * 0.5f;
    if (rad <= 0.0f || rad < 1.0f) {
        SDL_Rect full = { (Sint16)xi, (Sint16)yi, (Uint16)wi, (Uint16)hi };
        SDL_FillRect(prSDLScreen, &full, sdl_col);
        return;
    }
    if (rad > min_half) rad = min_half;
    int ri = (int)rad;
    if (ri < 1) ri = 1;

    /* Full-width middle band */
    int mid_y = yi + ri;
    int mid_h = hi - 2 * ri;
    if (mid_h > 0) {
        SDL_Rect mid = { (Sint16)xi, (Sint16)mid_y, (Uint16)wi, (Uint16)mid_h };
        SDL_FillRect(prSDLScreen, &mid, sdl_col);
    }

    /* Rounded corner arcs, one scanline per row */
    float radius_sq = (float)ri * (float)ri;
    for (int row = 0; row < ri; row++) {
        float dist_y = (float)ri - (float)row - 0.5f;
        float chord = sqrtf(radius_sq - dist_y * dist_y);
        int inset = ri - (int)(chord + 0.5f);
        if (inset < 0) inset = 0;
        int row_w = wi - 2 * inset;
        if (row_w <= 0) continue;

        SDL_Rect top = { (Sint16)(xi + inset), (Sint16)(yi + row), (Uint16)row_w, 1 };
        SDL_FillRect(prSDLScreen, &top, sdl_col);

        int bot_y = yi + hi - 1 - row;
        if (bot_y > yi + row) {
            SDL_Rect bot = { (Sint16)(xi + inset), (Sint16)bot_y, (Uint16)row_w, 1 };
            SDL_FillRect(prSDLScreen, &bot, sdl_col);
        }
    }
}

void vita_draw_rounded_rect_outline(float x, float y, float w, float h, float r, float thickness, unsigned int color)
{
    if (!prSDLScreen || w <= 0 || h <= 0) return;
    Uint32 sdl_col = to_sdl_color(color);
    Uint16 th = (Uint16)(thickness > 1.0f ? thickness : 1.0f);

    SDL_Rect top = { (Sint16)x, (Sint16)y, (Uint16)w, th };
    SDL_Rect bot = { (Sint16)x, (Sint16)(y + h - th), (Uint16)w, th };
    SDL_Rect lft = { (Sint16)x, (Sint16)y, th, (Uint16)h };
    SDL_Rect rgt = { (Sint16)(x + w - th), (Sint16)y, th, (Uint16)h };

    SDL_FillRect(prSDLScreen, &top, sdl_col);
    SDL_FillRect(prSDLScreen, &bot, sdl_col);
    SDL_FillRect(prSDLScreen, &lft, sdl_col);
    SDL_FillRect(prSDLScreen, &rgt, sdl_col);
}

void vita_draw_card(float x, float y, float w, float h, bool focused, bool active)
{
    unsigned int bg_col = focused ? VITA_COLOR_CARD_FOCUSED : VITA_COLOR_CARD;
    unsigned int border_col = focused ? VITA_COLOR_FOCUS_BORDER : VITA_COLOR_CARD_BORDER;

    vita_draw_rounded_rect(x, y, w, h, 6.0f, bg_col);
    if (focused) {
        vita_draw_rounded_rect_outline(x, y, w, h, 6.0f, 2.0f, border_col);
        vita_draw_rounded_rect(x + 2.0f, y + 3.0f, 4.0f, h - 6.0f, 2.0f, VITA_COLOR_AMIGA_RED);
    } else {
        vita_draw_rounded_rect_outline(x, y, w, h, 6.0f, 1.0f, border_col);
    }
}

void vita_draw_card_custom(float x, float y, float w, float h, unsigned int bg_col, unsigned int border_col)
{
    vita_draw_rounded_rect(x, y, w, h, 6.0f, bg_col);
    vita_draw_rounded_rect_outline(x, y, w, h, 6.0f, 1.0f, border_col);
}


int vita_get_text_width(float scale, const char *text)
{
    if (!text || text[0] == '\0') return 0;
    int total_w = 0;
    float factor = scale * 2.0f;
    for (int i = 0; text[i] != '\0'; i++) {
        unsigned char c = (unsigned char)text[i];
        if (c == '\n') break;
        int cw = (c >= 32 && c <= 126) ? s_char_widths[c - 32] : 6;
        total_w += (int)ceilf((float)(cw + 1) * factor);
    }
    return total_w;
}

int vita_get_text_height(float scale)
{
    return (int)ceilf(8.0f * scale * 2.0f);
}

void vita_truncate_text(const char *in_text, float max_w, float scale, char *out_buf, size_t out_size)
{
    if (!in_text || !out_buf || out_size == 0) return;
    int full_w = vita_get_text_width(scale, in_text);
    if (full_w <= (int)max_w) {
        strncpy(out_buf, in_text, out_size - 1);
        out_buf[out_size - 1] = '\0';
        return;
    }

    int dots_w = vita_get_text_width(scale, "...");
    int cur_w = 0;
    size_t out_idx = 0;
    float factor = scale * 2.0f;

    for (size_t i = 0; in_text[i] != '\0' && out_idx + 4 < out_size; i++) {
        unsigned char c = (unsigned char)in_text[i];
        int cw = (c >= 32 && c <= 126) ? s_char_widths[c - 32] : 6;
        int next_w = cur_w + (int)ceilf((float)(cw + 1) * factor);
        if (next_w + dots_w > (int)max_w) {
            break;
        }
        out_buf[out_idx++] = in_text[i];
        cur_w = next_w;
    }

    out_buf[out_idx++] = '.';
    out_buf[out_idx++] = '.';
    out_buf[out_idx++] = '.';
    out_buf[out_idx] = '\0';
}

void vita_draw_text(float x, float y, unsigned int color, float scale, const char *text)
{
    if (!prSDLScreen || !text || text[0] == '\0') return;

    Uint32 sdl_col = to_sdl_color(color);
    int cur_x = (int)x;
    int cur_y = (int)y;

    float factor = scale * 2.0f;
    int dot_w = (int)ceilf(factor);
    int dot_h = (int)ceilf(factor);
    if (dot_w < 2) dot_w = 2;
    if (dot_h < 2) dot_h = 2;

    for (int idx = 0; text[idx] != '\0'; idx++) {
        unsigned char c = (unsigned char)text[idx];
        if (c == '\n') {
            cur_x = (int)x;
            cur_y += (int)(10.0f * factor) + 4;
            continue;
        }

        if (c >= 32 && c <= 126) {
            const unsigned char *glyph = s_font_8x8[c - 32];
            int char_w = s_char_widths[c - 32];

            for (int r = 0; r < 8; r++) {
                unsigned char row = glyph[r];
                for (int col = 0; col < 8; col++) {
                    if (row & (0x80 >> col)) {
                        SDL_Rect p = {
                            (Sint16)(cur_x + (int)(col * factor)),
                            (Sint16)(cur_y + (int)(r * factor)),
                            (Uint16)dot_w,
                            (Uint16)dot_h
                        };
                        SDL_FillRect(prSDLScreen, &p, sdl_col);
                    }
                }
            }
            cur_x += (int)ceilf((float)(char_w + 1) * factor);
        } else {
            cur_x += (int)ceilf(7.0f * factor);
        }
    }
}

void vita_draw_textf(float x, float y, unsigned int color, float scale, const char *fmt, ...)
{
    char buf[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    vita_draw_text(x, y, color, scale, buf);
}

void vita_draw_text_centered(float cx, float y, unsigned int color, float scale, const char *text)
{
    int w = vita_get_text_width(scale, text);
    vita_draw_text(cx - ((float)w * 0.5f), y, color, scale, text);
}

void vita_draw_text_right(float rx, float y, unsigned int color, float scale, const char *text)
{
    int w = vita_get_text_width(scale, text);
    vita_draw_text(rx - (float)w, y, color, scale, text);
}

void vita_draw_text_wrapped(float x, float y, float max_w, unsigned int color, float scale, const char *text)
{
    if (!text || text[0] == '\0') return;

    char line_buf[256];
    line_buf[0] = '\0';
    float cur_y = y;
    float line_h = (8.0f * scale * 2.0f) + 6.0f;

    const char *p = text;
    while (*p != '\0') {
        char word[128];
        int w_idx = 0;
        while (*p == ' ') p++;
        if (*p == '\n') {
            if (line_buf[0] != '\0') {
                vita_draw_text(x, cur_y, color, scale, line_buf);
                cur_y += line_h;
                line_buf[0] = '\0';
            }
            p++;
            continue;
        }

        while (*p != '\0' && *p != ' ' && *p != '\n' && w_idx < 120) {
            word[w_idx++] = *p++;
        }
        word[w_idx] = '\0';

        char test_line[256];
        if (line_buf[0] == '\0') {
            snprintf(test_line, sizeof(test_line), "%s", word);
        } else {
            snprintf(test_line, sizeof(test_line), "%s %s", line_buf, word);
        }

        if (vita_get_text_width(scale, test_line) <= (int)max_w) {
            strncpy(line_buf, test_line, sizeof(line_buf) - 1);
        } else {
            if (line_buf[0] != '\0') {
                vita_draw_text(x, cur_y, color, scale, line_buf);
                cur_y += line_h;
            }
            strncpy(line_buf, word, sizeof(line_buf) - 1);
        }
    }

    if (line_buf[0] != '\0') {
        vita_draw_text(x, cur_y, color, scale, line_buf);
    }
}


void vita_draw_button_glyph(float x, float y, VitaButtonGlyph glyph)
{
    if (!prSDLScreen) return;
    Uint32 white_col = to_sdl_color(VITA_COLOR_TEXT_WHITE);

    switch (glyph) {
        case VITA_BTN_CROSS: {
            vita_draw_rounded_rect(x, y, 22.0f, 22.0f, 11.0f, RGBA8(0, 88, 216, 255));
            for (int i = 0; i < 8; i++) {
                SDL_Rect p1 = { (Sint16)(x + 7 + i), (Sint16)(y + 7 + i), 2, 2 };
                SDL_Rect p2 = { (Sint16)(x + 14 - i), (Sint16)(y + 7 + i), 2, 2 };
                SDL_FillRect(prSDLScreen, &p1, white_col);
                SDL_FillRect(prSDLScreen, &p2, white_col);
            }
            break;
        }
        case VITA_BTN_CIRCLE: {
            vita_draw_rounded_rect(x, y, 22.0f, 22.0f, 11.0f, RGBA8(229, 37, 33, 255));
            SDL_Rect r_in = { (Sint16)(x + 5), (Sint16)(y + 5), 12, 12 };
            SDL_Rect r_hole = { (Sint16)(x + 8), (Sint16)(y + 8), 6, 6 };
            SDL_FillRect(prSDLScreen, &r_in, white_col);
            SDL_FillRect(prSDLScreen, &r_hole, to_sdl_color(RGBA8(229, 37, 33, 255)));
            break;
        }
        case VITA_BTN_SQUARE: {
            vita_draw_rounded_rect(x, y, 22.0f, 22.0f, 11.0f, RGBA8(207, 40, 122, 255));
            SDL_Rect r_sq = { (Sint16)(x + 5), (Sint16)(y + 5), 12, 12 };
            SDL_Rect r_hole = { (Sint16)(x + 8), (Sint16)(y + 8), 6, 6 };
            SDL_FillRect(prSDLScreen, &r_sq, white_col);
            SDL_FillRect(prSDLScreen, &r_hole, to_sdl_color(RGBA8(207, 40, 122, 255)));
            break;
        }
        case VITA_BTN_TRIANGLE: {
            vita_draw_rounded_rect(x, y, 22.0f, 22.0f, 11.0f, RGBA8(0, 168, 80, 255));
            for (int r = 0; r < 10; r++) {
                int half = r / 2;
                SDL_Rect p = { (Sint16)(x + 11 - half), (Sint16)(y + 5 + r), (Uint16)(half * 2 + 1), 1 };
                SDL_FillRect(prSDLScreen, &p, white_col);
            }
            break;
        }
        case VITA_BTN_L: {
            vita_draw_rounded_rect(x, y, 28.0f, 22.0f, 5.0f, RGBA8(38, 48, 68, 255));
            vita_draw_rounded_rect_outline(x, y, 28.0f, 22.0f, 5.0f, 1.0f, RGBA8(75, 90, 120, 255));
            vita_draw_text(x + 9.0f, y + 4.0f, VITA_COLOR_TEXT_WHITE, 0.90f, "L");
            break;
        }
        case VITA_BTN_R: {
            vita_draw_rounded_rect(x, y, 28.0f, 22.0f, 5.0f, RGBA8(38, 48, 68, 255));
            vita_draw_rounded_rect_outline(x, y, 28.0f, 22.0f, 5.0f, 1.0f, RGBA8(75, 90, 120, 255));
            vita_draw_text(x + 9.0f, y + 4.0f, VITA_COLOR_TEXT_WHITE, 0.90f, "R");
            break;
        }
        case VITA_BTN_START: {
            vita_draw_rounded_rect(x, y, 58.0f, 22.0f, 5.0f, RGBA8(30, 40, 58, 255));
            vita_draw_rounded_rect_outline(x, y, 58.0f, 22.0f, 5.0f, 1.0f, RGBA8(60, 75, 100, 255));
            vita_draw_text(x + 7.0f, y + 4.0f, VITA_COLOR_TEXT_WHITE, 0.85f, "START");
            break;
        }
        case VITA_BTN_SELECT: {
            vita_draw_rounded_rect(x, y, 66.0f, 22.0f, 5.0f, RGBA8(30, 40, 58, 255));
            vita_draw_rounded_rect_outline(x, y, 66.0f, 22.0f, 5.0f, 1.0f, RGBA8(60, 75, 100, 255));
            vita_draw_text(x + 7.0f, y + 4.0f, VITA_COLOR_TEXT_WHITE, 0.85f, "SELECT");
            break;
        }
        default:
            break;
    }
}

void vita_draw_hint_item(float x, float y, VitaButtonGlyph glyph, const char *label)
{
    vita_draw_button_glyph(x, y, glyph);
    float offset_x = (glyph == VITA_BTN_START) ? 84.0f : (glyph == VITA_BTN_SELECT ? 90.0f : (glyph == VITA_BTN_L || glyph == VITA_BTN_R ? 38.0f : 34.0f));
    if (label && label[0] != '\0') {
        float label_scale = (strcmp(label, "RESUME/START") == 0) ? 0.65f : 0.90f;
        vita_draw_text(x + offset_x, y + 4.0f, RGBA8(220, 230, 245, 255), label_scale, label);
    }
}

void vita_draw_badge(float x, float y, const char *label, unsigned int bg_col, unsigned int text_col)
{
    if (!label) return;
    int text_w = vita_get_text_width(0.80f, label);
    float badge_w = (float)text_w + 14.0f;
    float badge_h = 22.0f;

    vita_draw_rounded_rect(x, y, badge_w, badge_h, 4.0f, bg_col);
    vita_draw_text_centered(x + (badge_w * 0.5f), y + 4.0f, text_col, 0.80f, label);
}

void vita_draw_led(float x, float y, const char *label, bool state, unsigned int led_col)
{
    unsigned int active_col = state ? led_col : RGBA8(50, 50, 50, 255);
    SDL_Rect led_r = { (Sint16)x, (Sint16)(y + 3.0f), 10, 10 };
    SDL_FillRect(prSDLScreen, &led_r, to_sdl_color(active_col));

    if (label) {
        vita_draw_text(x + 16.0f, y + 1.0f, state ? VITA_COLOR_TEXT_WHITE : VITA_COLOR_TEXT_MUTED, 0.85f, label);
    }
}

void vita_draw_header(const char *title, VitaGuiTab current_tab, const VitaSystemInfo *sysinfo)
{
    SDL_Rect hdr_r = { 0, 0, VITA_SCREEN_W, 48 };
    SDL_FillRect(prSDLScreen, &hdr_r, to_sdl_color(VITA_COLOR_HEADER));
    vita_draw_rounded_rect_outline(0, 0, VITA_SCREEN_W, 48, 0.0f, 1.0f, VITA_COLOR_CARD_BORDER);

    /* Amiga Boing Ball animated icon */
    s_boing_angle += 0.03f;
    vita_draw_boing_ball_icon(24.0f, 24.0f, 14.0f, s_boing_angle);

    /* Main Title */
    vita_draw_text(46.0f, 15.0f, VITA_COLOR_TEXT_WHITE, 1.10f, "UAE4ALL2");
    vita_draw_badge(210.0f, 13.0f, "HD VITA", VITA_COLOR_AMIGA_RED, VITA_COLOR_TEXT_WHITE);

    /* Model Badge */
    const char *model_str;
    if (changed_prefs.cpu_level == M68020 && mainMenu_chipset == 2 && mainMenu_fastMemory == 0)
        model_str = "Amiga CD32";
    else if (changed_prefs.cpu_level == M68020)
        model_str = "Amiga 1200 (AGA)";
    else
        model_str = "Amiga 500 (OCS)";
    vita_draw_badge(340.0f, 13.0f, model_str, RGBA8(30, 41, 59, 255), VITA_COLOR_TEXT_MUTED);

    /* Clock & Battery on top right */
    if (sysinfo) {
        char bat_str[32];
        snprintf(bat_str, sizeof(bat_str), "%d%%", sysinfo->battery_percent);
        unsigned int bat_col = sysinfo->battery_percent < 20 ? VITA_COLOR_DANGER : (sysinfo->is_charging ? VITA_COLOR_SUCCESS : VITA_COLOR_TEXT_MUTED);

        vita_draw_text_right(VITA_SCREEN_W - 130.0f, 15.0f, bat_col, 0.90f, bat_str);
        vita_draw_text_right(VITA_SCREEN_W - 16.0f, 15.0f, VITA_COLOR_TEXT_WHITE, 0.95f, sysinfo->time_str);
    }
}

void vita_draw_tab_bar(VitaGuiTab current_tab, float y)
{
    float tab_w = (float)VITA_SCREEN_W / (float)VITA_TAB_COUNT;
    SDL_Rect bar_r = { 0, (Sint16)y, VITA_SCREEN_W, 36 };
    SDL_FillRect(prSDLScreen, &bar_r, to_sdl_color(RGBA8(18, 22, 32, 255)));

    for (int i = 0; i < VITA_TAB_COUNT; i++) {
        float tx = (float)i * tab_w;
        bool active = (i == (int)current_tab);

        float tab_scale = active ? 0.90f : 0.85f;
        while (tab_scale > 0.60f && vita_get_text_width(tab_scale, s_tab_names[i]) > (int)(tab_w - 10.0f))
            tab_scale -= 0.02f;

        if (active) {
            vita_draw_rounded_rect(tx + 4.0f, y + 4.0f, tab_w - 8.0f, 28.0f, 5.0f, VITA_COLOR_AMIGA_RED);
            vita_draw_text_centered(tx + (tab_w * 0.5f), y + 7.0f, VITA_COLOR_TEXT_WHITE, tab_scale, s_tab_names[i]);
        } else {
            vita_draw_text_centered(tx + (tab_w * 0.5f), y + 7.0f, VITA_COLOR_TEXT_MUTED, tab_scale, s_tab_names[i]);
        }
    }
}

void vita_draw_footer(const char *left_hint, const char *right_hint)
{
    SDL_Rect ftr_r = { 0, VITA_SCREEN_H - 42, VITA_SCREEN_W, 42 };
    SDL_FillRect(prSDLScreen, &ftr_r, to_sdl_color(VITA_COLOR_FOOTER));
    vita_draw_rounded_rect_outline(0, VITA_SCREEN_H - 42, VITA_SCREEN_W, 42, 0.0f, 1.0f, VITA_COLOR_CARD_BORDER);

    float btn_y = VITA_SCREEN_H - 32.0f;

    /* Fixed columns with enough separation for the proportional font. */
    if (s_active_tab == VITA_TAB_WHDLOAD) {
        /* Dedicated WHDLoad footer keys: (X) LAUNCH, ([]) INSTALL LHA, (/\\) REBOOT.
         * Columns are spaced so labels never touch the neighbouring glyph. */
        vita_draw_hint_item(20.0f, btn_y, VITA_BTN_CROSS, "LAUNCH");
        vita_draw_hint_item(175.0f, btn_y, VITA_BTN_SQUARE, "INSTALL LHA");
        vita_draw_hint_item(360.0f, btn_y, VITA_BTN_TRIANGLE, "REBOOT");
    } else {
        vita_draw_hint_item(20.0f, btn_y, VITA_BTN_CROSS, "SELECT");

        /* Only show actions that are handled by the active tab. */
        if (s_active_tab == VITA_TAB_FLOPPY) {
            vita_draw_hint_item(145.0f, btn_y, VITA_BTN_TRIANGLE, "EJECT");
            vita_draw_hint_item(275.0f, btn_y, VITA_BTN_SQUARE, "REBOOT");
        } else if (s_active_tab == VITA_TAB_HARD_DISK) {
            vita_draw_hint_item(145.0f, btn_y, VITA_BTN_TRIANGLE, "EJECT");
            vita_draw_hint_item(275.0f, btn_y, VITA_BTN_SQUARE, "MANAGER");
        } else if (s_active_tab == VITA_TAB_SAVESTATES) {
            vita_draw_hint_item(145.0f, btn_y, VITA_BTN_SQUARE, "LOAD");
        }
    }

    /* Right navigation hints */
    vita_draw_hint_item(610.0f, btn_y, VITA_BTN_L,     "");
    vita_draw_hint_item(660.0f, btn_y, VITA_BTN_R,     "TAB");
    float start_x = (s_active_tab == VITA_TAB_FLOPPY) ? 745.0f : 780.0f;
    const char *start_label = (s_active_tab == VITA_TAB_FLOPPY) ? "RESUME/START" : "RESUME";
    vita_draw_hint_item(start_x, btn_y, VITA_BTN_START, start_label);
}

void vita_draw_button_item_custom(float x, float y, float w, float h, const char *title, const char *subtitle, const char *badge, unsigned int badge_col, bool focused, bool active)
{
    vita_draw_card(x, y, w, h, focused, active);

    char title_buf[128];
    float max_title_w = badge ? (w - 180.0f) : (w - 40.0f);
    vita_truncate_text(title, max_title_w, 0.95f, title_buf, sizeof(title_buf));

    bool has_sub = (subtitle != NULL && subtitle[0] != '\0');
    float text_y = has_sub ? (y + (h * 0.5f) - 13.0f) : (y + (h - 14.0f) * 0.5f);
    unsigned int title_col = focused ? VITA_COLOR_TEXT_WHITE : RGBA8(220, 230, 245, 255);
    vita_draw_text(x + 16.0f, text_y, title_col, 0.95f, title_buf);

    if (has_sub) {
        char sub_buf[256];
        float sub_max_w = badge ? (w - 40.0f - 140.0f) : (w - 40.0f);
        vita_truncate_text(subtitle, sub_max_w, 0.72f, sub_buf, sizeof(sub_buf));
        float sub_y = y + (h * 0.5f) + 5.0f;
        vita_draw_text(x + 16.0f, sub_y, VITA_COLOR_TEXT_MUTED, 0.72f, sub_buf);
    }

    if (badge) {
        int bw = vita_get_text_width(0.80f, badge) + 14;
        float badge_y = y + (h - 22.0f) * 0.5f;
        vita_draw_badge(x + w - (float)bw - 16.0f, badge_y, badge, badge_col, VITA_COLOR_TEXT_WHITE);
    }
}

void vita_draw_button_item(float x, float y, float w, float h, const char *title, const char *subtitle, const char *badge, bool focused, bool active)
{
    unsigned int badge_col = focused ? VITA_COLOR_AMIGA_BLUE : RGBA8(40, 52, 75, 255);
    vita_draw_button_item_custom(x, y, w, h, title, subtitle, badge, badge_col, focused, active);
}

void vita_draw_selector_item(float x, float y, float w, float h, const char *title, const char *current_value, bool focused)
{
    vita_draw_card(x, y, w, h, focused, false);
    float text_y = y + (h - 14.0f) * 0.5f;

    char title_buf[128];
    vita_truncate_text(title, w * 0.50f, 0.95f, title_buf, sizeof(title_buf));
    int title_w = vita_get_text_width(0.95f, title_buf);
    vita_draw_text(x + 16.0f, text_y, focused ? VITA_COLOR_TEXT_WHITE : RGBA8(220, 230, 245, 255), 0.95f, title_buf);

    char value_buf[128];
    char val_str[128];
    const char *value = current_value ? current_value : "None";
    float max_val_w = w - (float)title_w - (focused ? 80.0f : 50.0f);
    if (max_val_w < 120.0f) max_val_w = 120.0f;
    vita_truncate_text(value, max_val_w, focused ? 0.95f : 0.90f, value_buf, sizeof(value_buf));

    if (focused) {
        snprintf(val_str, sizeof(val_str), "<  %s  >", value_buf);
        vita_draw_text_right(x + w - 16.0f, text_y, VITA_COLOR_FOCUS_BORDER, 0.95f, val_str);
    } else {
        snprintf(val_str, sizeof(val_str), "%s", value_buf);
        vita_draw_text_right(x + w - 16.0f, text_y, VITA_COLOR_TEXT_MUTED, 0.90f, val_str);
    }
}

void vita_draw_switch_item(float x, float y, float w, float h, const char *title, bool enabled, bool focused)
{
    vita_draw_card(x, y, w, h, focused, false);
    /* Dynamic vertical centering (see vita_draw_selector_item). */
    float text_y = y + (h - 14.0f) * 0.5f;
    vita_draw_text(x + 16.0f, text_y, focused ? VITA_COLOR_TEXT_WHITE : RGBA8(220, 230, 245, 255), 0.95f, title);

    float sw_w = 48.0f;
    float sw_h = 24.0f;
    float sw_x = x + w - sw_w - 18.0f;
    float sw_y = y + (h - sw_h) * 0.5f;

    unsigned int track_col = enabled ? VITA_COLOR_SUCCESS : RGBA8(45, 55, 75, 255);
    vita_draw_rounded_rect(sw_x, sw_y, sw_w, sw_h, 6.0f, track_col);

    float knob_x = enabled ? (sw_x + sw_w - 20.0f) : (sw_x + 4.0f);
    SDL_Rect knob = { (Sint16)knob_x, (Sint16)(sw_y + 4.0f), 16, 16 };
    SDL_FillRect(prSDLScreen, &knob, to_sdl_color(VITA_COLOR_TEXT_WHITE));
}

void vita_draw_slider_item(float x, float y, float w, float h, const char *title, int val, int min, int max, const char *suffix, bool focused)
{
    vita_draw_card(x, y, w, h, focused, false);

    char label_buf[128];
    snprintf(label_buf, sizeof(label_buf), "%s: %d%s", title, val, suffix ? suffix : "");
    vita_draw_text(x + 16.0f, y + 8.0f, focused ? VITA_COLOR_TEXT_WHITE : RGBA8(220, 230, 245, 255), 0.90f, label_buf);

    float bar_x = x + 16.0f;
    float bar_y = y + 28.0f;
    float bar_w = w - 32.0f;
    float bar_h = 8.0f;

    float progress = (float)(val - min) / (float)(max - min);
    if (progress < 0.0f) progress = 0.0f;
    if (progress > 1.0f) progress = 1.0f;

    vita_draw_rounded_rect(bar_x, bar_y, bar_w, bar_h, 4.0f, RGBA8(40, 50, 70, 255));
    unsigned int fill_col = focused ? VITA_COLOR_AMIGA_RED : VITA_COLOR_AMIGA_BLUE;
    vita_draw_rounded_rect(bar_x, bar_y, bar_w * progress, bar_h, 4.0f, fill_col);
}

/* Fractional-Y text renderer used by the credits scroller: rows are placed
 * at (int)(y + row*factor), so a fractional y produces true sub-pixel
 * vertical motion (60 fps smooth scrolling). */
static void vita_draw_text_f(float x, float y, unsigned int color, float scale, const char *text)
{
    if (!prSDLScreen || !text || text[0] == '\0') return;

    Uint32 sdl_col = to_sdl_color(color);
    float factor = scale * 2.0f;
    int dot_w = (int)ceilf(factor);
    int dot_h = dot_w;
    if (dot_w < 2) dot_w = 2;
    if (dot_h < 2) dot_h = 2;

    float cur_x = x;
    for (int idx = 0; text[idx] != '\0'; idx++) {
        unsigned char c = (unsigned char)text[idx];
        if (c == '\n') {
            cur_x = x;
            y += 10.0f * factor + 4.0f;
            continue;
        }
        if (c >= 32 && c <= 126) {
            const unsigned char *glyph = s_font_8x8[c - 32];
            int char_w = s_char_widths[c - 32];
            for (int r = 0; r < 8; r++) {
                unsigned char row = glyph[r];
                if (!row) continue;
                int row_y = (int)(y + (float)r * factor);
                for (int col = 0; col < 8; col++) {
                    if (row & (0x80 >> col)) {
                        SDL_Rect p = {
                            (Sint16)(int)(cur_x + (float)col * factor),
                            (Sint16)row_y,
                            (Uint16)dot_w,
                            (Uint16)dot_h
                        };
                        SDL_FillRect(prSDLScreen, &p, sdl_col);
                    }
                }
            }
            cur_x += ceilf((float)(char_w + 1) * factor);
        } else {
            cur_x += ceilf(7.0f * factor);
        }
    }
}

/* Offscreen surface used by the 3D Boing Ball renderer. */
static SDL_Surface *s_boing3d_surf = NULL;

/* Rotating 3D Boing Ball: a checkered sphere with per-pixel shading,
 * rendered into an offscreen surface and blitted in one pass. */
static void vita_draw_boing_ball_3d(float cx, float cy, float radius, float rot_x, float rot_y)
{
    if (!prSDLScreen || radius < 2.0f) return;

    int size = (int)(radius * 2.0f) + 4;
    if (!s_boing3d_surf || s_boing3d_surf->w != size || s_boing3d_surf->h != size) {
        if (s_boing3d_surf)
            SDL_FreeSurface(s_boing3d_surf);
        s_boing3d_surf = SDL_CreateRGBSurface(0, size, size, 32,
            0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
        if (s_boing3d_surf)
            SDL_SetColorKey(s_boing3d_surf, SDL_SRCCOLORKEY, 0xFF000000);
    }
    if (!s_boing3d_surf)
        return;

    SDL_Surface *surf = s_boing3d_surf;
    SDL_LockSurface(surf);
    int pitch = surf->pitch / 4;
    Uint32 *pix = (Uint32 *)surf->pixels;

    float r = radius;
    float r2 = r * r;
    float center = (float)size * 0.5f;
    float cosx = cosf(rot_x), sinx = sinf(rot_x);
    float cosy = cosf(rot_y), siny = sinf(rot_y);

    for (int row = 0; row < size; row++) {
        for (int col = 0; col < size; col++) {
            float nx = ((float)col - center) / r;
            float ny = ((float)row - center) / r;
            float nz2 = 1.0f - nx * nx - ny * ny;
            if (nz2 < 0.0f) {
                pix[row * pitch + col] = 0xFF000000; /* color-keyed away */
                continue;
            }
            float nz = sqrtf(nz2);

            /* Rotate the sphere point around Y then X. */
            float x1 = nx * cosy + nz * siny;
            float z1 = -nx * siny + nz * cosy;
            float y1 = ny;
            float y2 = y1 * cosx - z1 * sinx;
            float z2 = y1 * sinx + z1 * cosx;
            float x2 = x1;

            /* Checkerboard pattern mapped on the rotated sphere. */
            float u = atan2f(z2, x2) / 6.2831853f + 0.5f;
            float v = asinf(y2) / 3.14159265f + 0.5f;
            int cu = (int)(u * 4.0f) & 1;
            int cv = (int)(v * 4.0f) & 1;
            bool is_red = ((cu + cv) & 1) != 0;

            /* Directional light from the upper left. */
            float shade = nx * -0.35f + ny * -0.45f + nz * 0.82f;
            if (shade < 0.15f) shade = 0.15f;
            if (shade > 1.0f) shade = 1.0f;

            if (is_red)
                pix[row * pitch + col] = RGBA8((int)(229 * shade), (int)(37 * shade), (int)(33 * shade), 255);
            else
                pix[row * pitch + col] = RGBA8((int)(238 * shade), (int)(238 * shade), (int)(240 * shade), 255);
        }
    }

    SDL_UnlockSurface(surf);

    SDL_Rect dst = {
        (Sint16)(int)(cx - center),
        (Sint16)(int)(cy - center),
        (Uint16)size,
        (Uint16)size
    };
    SDL_BlitSurface(surf, NULL, prSDLScreen, &dst);
}

typedef enum {
    CR_EMPTY = 0,
    CR_TITLE,      /* big red title */
    CR_SUBTITLE,   /* orange subtitle */
    CR_TEXT,       /* plain white */
    CR_DIM,        /* muted */
    CR_LINK,       /* Amiga blue link */
    CR_SECTION,    /* orange section header */
    CR_RED         /* red highlight */
} CreditStyle;

typedef struct {
    const char *text;
    CreditStyle style;
} CreditLine;

void vita_show_about_box(void)
{
    static const CreditLine credits[] = {
        { "UAE4ALL2 HD Vita", CR_TITLE },
        { "Version 1.04 - Amiga Emulator for PS Vita", CR_SUBTITLE },
        { "", CR_EMPTY },
        { "A high-definition port of the classic UAE4ALL Amiga emulator,", CR_TEXT },
        { "now with WHDLoad, HDF, IPF and CD32 support on the Vita.", CR_TEXT },
        { "github.com/theheroGAC/UAE4ALL2-HD-VITA", CR_LINK },
        { "", CR_EMPTY },
        { "=====  WHDLoad Support  =====", CR_SECTION },
        { "WHDLoad by Bert Jahn (Wepl)", CR_TEXT },
        { "The legendary hard-disk loader system for Amiga games.", CR_DIM },
        { "Official website: www.whdload.de", CR_LINK },
        { "Game packages & patches: aminet.net", CR_LINK },
        { "Thanks to the WHDLoad team and every slave author.", CR_DIM },
        { "", CR_EMPTY },
        { "=====  Hardware Tribute  =====", CR_SECTION },
        { "In memory of the engineers who made the Amiga possible:", CR_TEXT },
        { "Jay Miner - father of the Amiga chipset", CR_RED },
        { "Dave Needle - Amiga chipset co-designer", CR_RED },
        { "RJ Mical - system software & creator of the Boing Ball demo", CR_RED },
        { "And to every Amiga user, past and present.", CR_DIM },
        { "", CR_EMPTY },
        { "=====  Original Authors & Credits  =====", CR_SECTION },
        { "This project is a derivative work of UAE4All and would not", CR_DIM },
        { "exist without the original authors and contributors:", CR_DIM },
        { "Chui, john4p, TomB, notaz, Bernd Schneider, Toni Wilen,", CR_TEXT },
        { "Pickle, smoku, AnotherGuest, Anonymous engineer, finkel,", CR_TEXT },
        { "Lubomyr, pelya", CR_TEXT },
        { "", CR_EMPTY },
        { "Cpasjuste - original Vita port, SDL-Vita, shader support", CR_TEXT },
        { "rsn8887 - Vita/Switch work and UAE4ALL2 improvements", CR_TEXT },
        { "github.com/rsn8887/uae4all2", CR_LINK },
        { "ScHlAuChi - testing, ideas, virtual keyboard", CR_TEXT },
        { "wronghands - menu font, keyboard styles and design", CR_TEXT },
        { "CrashMidnick - French virtual keyboard", CR_TEXT },
        { "Xerpi & frangarCJ - Vita2D and shader library", CR_TEXT },
        { "The VitaSDK Team and all beta testers", CR_TEXT },
        { "", CR_EMPTY },
        { "Thank you for playing!", CR_RED },
    };
    const int total_lines = (int)(sizeof(credits) / sizeof(credits[0]));
    const float line_h = 27.0f;
    const float content_h = (float)total_lines * line_h;

    /* 60 fps demoscene scroller: fractional scroll offset accumulated every
     * frame and rendered through the sub-pixel text path -> smooth motion. */
    float scroll = 0.0f;
    float speed = 1.0f;
    int paused = 0;
    int frame_count = 0;
    int hold_counter = 0;
    float rot_x = 0.0f, rot_y = 0.0f;

    VitaInputState input;
    memset(&input, 0, sizeof(input));
    vita_gui_init();

    while (1) {
        vita_gui_update_input(&input);
        frame_count++;

        if (frame_count > 6 && (input.pressed & (SCE_CTRL_CIRCLE | SCE_CTRL_CROSS | SCE_CTRL_START)))
            break;

        /* ([]) toggles pause / resume. */
        if (input.pressed & SCE_CTRL_SQUARE)
            paused = !paused;

        /* D-Pad UP: rewind / slow down. D-Pad DOWN: accelerate. */
        bool up_held = (input.held & SCE_CTRL_UP) != 0;
        bool down_held = (input.held & SCE_CTRL_DOWN) != 0;
        if (input.pressed & SCE_CTRL_UP)
            speed -= 0.5f;
        if (input.pressed & SCE_CTRL_DOWN)
            speed += 0.5f;
        if (up_held || down_held) {
            hold_counter++;
            if ((hold_counter & 3) == 0) {
                if (up_held) speed -= 0.25f;
                if (down_held) speed += 0.25f;
            }
        } else {
            hold_counter = 0;
            /* Drift back to the normal scroll speed when no key is held. */
            if (speed < 1.0f) speed += 0.25f;
            else if (speed > 1.0f) speed -= 0.25f;
        }
        if (speed < -5.0f) speed = -5.0f;
        if (speed > 8.0f) speed = 8.0f;

        if (!paused && speed != 0.0f) {
            scroll += speed;
            if (scroll >= content_h) scroll -= content_h;
            if (scroll < 0.0f) scroll += content_h;
        }

        /* --- Rendering --- */
        SDL_FillRect(prSDLScreen, NULL, to_sdl_color(VITA_COLOR_OVERLAY_BG));
        float dx = 24.0f;
        float dy = 14.0f;
        float dw = VITA_SCREEN_W - 48.0f;
        float dh = VITA_SCREEN_H - 28.0f;
        vita_draw_card_custom(dx, dy, dw, dh, VITA_COLOR_HEADER, VITA_COLOR_FOCUS_BORDER);

        /* Header with rotating 3D Boing Ball (demoscene style). */
        rot_x += 0.045f;
        rot_y += 0.065f;
        vita_draw_boing_ball_3d(dx + 46.0f, dy + 40.0f, 24.0f, rot_x, rot_y);
        vita_draw_text(dx + 92.0f, dy + 20.0f, VITA_COLOR_AMIGA_RED, 1.15f, "UAE4ALL2 HD Vita");
        vita_draw_text(dx + 92.0f, dy + 44.0f, VITA_COLOR_AMIGA_ORANGE, 0.85f, "About & Credits - WHDLoad Edition");

        vita_draw_rounded_rect_outline(dx + 16.0f, dy + 76.0f, dw - 32.0f, 1.0f, 0.0f, 1.0f, VITA_COLOR_CARD_BORDER);

        /* Scrolling credits (sub-pixel Y via vita_draw_text_f). */
        float area_top = dy + 90.0f;
        float area_bottom = dy + dh - 44.0f;
        for (int i = 0; i < total_lines; i++) {
            float y = area_top + (float)i * line_h - scroll;
            if (y < area_top - line_h || y > area_bottom)
                continue;

            const CreditLine *line = &credits[i];
            if (line->style == CR_EMPTY)
                continue;

            float scale;
            unsigned int color;
            switch (line->style) {
                case CR_TITLE:    scale = 1.00f; color = VITA_COLOR_AMIGA_RED; break;
                case CR_SUBTITLE: scale = 0.80f; color = VITA_COLOR_AMIGA_ORANGE; break;
                case CR_SECTION:  scale = 0.85f; color = VITA_COLOR_AMIGA_ORANGE; break;
                case CR_LINK:     scale = 0.75f; color = VITA_COLOR_AMIGA_BLUE; break;
                case CR_DIM:      scale = 0.75f; color = VITA_COLOR_TEXT_MUTED; break;
                case CR_RED:      scale = 0.78f; color = VITA_COLOR_AMIGA_RED; break;
                default:          scale = 0.78f; color = VITA_COLOR_TEXT_WHITE; break;
            }

            float cx = VITA_SCREEN_W * 0.5f;
            float x = cx - (float)vita_get_text_width(scale, line->text) * 0.5f;
            vita_draw_text_f(x, y, color, scale, line->text);
        }

        /* Footer controls hint. */
        char footer[96];
        if (paused)
            snprintf(footer, sizeof(footer), "[] RESUME   %d%%   O/X/START CLOSE", (int)(speed * 100.0f));
        else
            snprintf(footer, sizeof(footer), "UP REWIND  DOWN ACCELERATE  [] PAUSE  O/X/START CLOSE");
        vita_draw_text_centered(VITA_SCREEN_W * 0.5f, VITA_SCREEN_H - 32.0f,
            paused ? VITA_COLOR_AMIGA_ORANGE : VITA_COLOR_TEXT_MUTED, 0.72f, footer);

        SDL_Flip(prSDLScreen);
        SDL_Delay(16);
    }
}

void vita_show_message_box(const char *title, const char *message, const char *btn_label)
{
    vita_gui_init();
    VitaInputState input;
    memset(&input, 0, sizeof(input));
    int frame_count = 0;

    while (1) {
        vita_gui_update_input(&input);
        frame_count++;

        if (frame_count > 6 && (input.pressed & (SCE_CTRL_CROSS | SCE_CTRL_CIRCLE | SCE_CTRL_START))) {
            break;
        }

        SDL_FillRect(prSDLScreen, NULL, to_sdl_color(VITA_COLOR_OVERLAY_BG));

        float dw = 640.0f, dh = 270.0f;
        float dx = (VITA_SCREEN_W - dw) * 0.5f;
        float dy = (VITA_SCREEN_H - dh) * 0.5f;
        vita_draw_card_custom(dx, dy, dw, dh, VITA_COLOR_HEADER, VITA_COLOR_FOCUS_BORDER);

        vita_draw_text_centered(dx + (dw * 0.5f), dy + 22.0f, VITA_COLOR_AMIGA_RED, 1.10f, title ? title : "Message");
        vita_draw_text_wrapped(dx + 30.0f, dy + 70.0f, dw - 60.0f, VITA_COLOR_TEXT_WHITE, 0.90f, message ? message : "");

        float bw = 180.0f, bh = 38.0f;
        float bx = dx + (dw - bw) * 0.5f;
        float by = dy + dh - 54.0f;
        vita_draw_rounded_rect(bx, by, bw, bh, 6.0f, VITA_COLOR_AMIGA_RED);

        /* Draw the Cross glyph separately. Parenthesized labels such as
           "OK (X)" were rendered badly by the tiny bitmap font (appearing as
           strings like "<x:"). */
        char clean_label[64];
        const char *source_label = btn_label ? btn_label : "OK";
        strncpy(clean_label, source_label, sizeof(clean_label) - 1);
        clean_label[sizeof(clean_label) - 1] = '\0';
        char *paren = strchr(clean_label, '(');
        if (paren) *paren = '\0';
        size_t label_len = strlen(clean_label);
        while (label_len > 0 && clean_label[label_len - 1] == ' ')
            clean_label[--label_len] = '\0';
        vita_draw_button_glyph(bx + 28.0f, by + 8.0f, VITA_BTN_CROSS);
        vita_draw_text(bx + 62.0f, by + 11.0f, VITA_COLOR_TEXT_WHITE, 0.95f, clean_label);

        SDL_Flip(prSDLScreen);
        SDL_Delay(20);
    }
}

void vita_gui_draw_progress(const char *title, const char *subtitle, float fraction, const char *item_name)
{
    vita_gui_init();
    if (fraction < 0.0f) fraction = 0.0f;
    if (fraction > 1.0f) fraction = 1.0f;

    SDL_FillRect(prSDLScreen, NULL, to_sdl_color(VITA_COLOR_OVERLAY_BG));

    float dw = 640.0f, dh = 240.0f;
    float dx = (VITA_SCREEN_W - dw) * 0.5f;
    float dy = (VITA_SCREEN_H - dh) * 0.5f;
    vita_draw_card_custom(dx, dy, dw, dh, VITA_COLOR_HEADER, VITA_COLOR_FOCUS_BORDER);

    vita_draw_text_centered(dx + (dw * 0.5f), dy + 22.0f, VITA_COLOR_AMIGA_RED, 1.10f, title ? title : "Processing...");
    vita_draw_text(dx + 30.0f, dy + 66.0f, VITA_COLOR_TEXT_WHITE, 0.95f, subtitle ? subtitle : "");

    float bar_x = dx + 30.0f;
    float bar_y = dy + 105.0f;
    float bar_w = dw - 60.0f;
    float bar_h = 24.0f;

    vita_draw_rounded_rect(bar_x, bar_y, bar_w, bar_h, 4.0f, VITA_COLOR_CARD_BORDER);
    vita_draw_rounded_rect(bar_x + 1.0f, bar_y + 1.0f, bar_w - 2.0f, bar_h - 2.0f, 3.0f, VITA_COLOR_BG);

    float fill_w = (bar_w - 4.0f) * fraction;
    if (fill_w > 0.0f) {
        vita_draw_rounded_rect(bar_x + 2.0f, bar_y + 2.0f, fill_w, bar_h - 4.0f, 2.0f, VITA_COLOR_AMIGA_RED);
    }

    if (item_name && item_name[0]) {
        char item_buf[80];
        strncpy(item_buf, item_name, sizeof(item_buf) - 1);
        item_buf[sizeof(item_buf) - 1] = '\0';
        vita_draw_text(dx + 30.0f, dy + 145.0f, VITA_COLOR_TEXT_DIM, 0.80f, item_buf);
    }

    SDL_Flip(prSDLScreen);
}

bool vita_show_confirm_box(const char *title, const char *message, const char *yes_label, const char *no_label)
{
    /* Exit is already an explicit command in the System tab. Do not make
       Cross depend on a second modal input loop, which could lose the button
       edge while the menu surface is being reused. */
    if (title && strcmp(title, "About") == 0) {
        vita_show_about_box();
        return false;
    }
    if (title && strcmp(title, "Exit") == 0)
        return true;

    vita_gui_init();
    VitaInputState input;
    memset(&input, 0, sizeof(input));
    int choice = 0;
    int frame_count = 0;

    while (1) {
        vita_gui_update_input(&input);
        frame_count++;

        if (frame_count > 6) {
            if (input.pressed & (SCE_CTRL_LEFT | SCE_CTRL_RIGHT)) {
                choice = 1 - choice;
            }
            if (input.pressed & SCE_CTRL_CROSS) {
                return (choice == 0);
            }
            if (input.pressed & SCE_CTRL_CIRCLE) {
                return false;
            }
        }

        SDL_FillRect(prSDLScreen, NULL, to_sdl_color(VITA_COLOR_OVERLAY_BG));

        float dw = 640.0f, dh = 270.0f;
        float dx = (VITA_SCREEN_W - dw) * 0.5f;
        float dy = (VITA_SCREEN_H - dh) * 0.5f;
        vita_draw_card_custom(dx, dy, dw, dh, VITA_COLOR_HEADER, VITA_COLOR_FOCUS_BORDER);

        vita_draw_text_centered(dx + (dw * 0.5f), dy + 22.0f, VITA_COLOR_AMIGA_RED, 1.10f, title ? title : "Confirm");
        vita_draw_text_wrapped(dx + 30.0f, dy + 70.0f, dw - 60.0f, VITA_COLOR_TEXT_WHITE, 0.90f, message ? message : "");

        float bw = 160.0f, bh = 38.0f;
        float b1_x = dx + 110.0f;
        float b2_x = dx + dw - 110.0f - bw;
        float by = dy + dh - 54.0f;

        vita_draw_rounded_rect(b1_x, by, bw, bh, 6.0f, choice == 0 ? VITA_COLOR_AMIGA_RED : VITA_COLOR_CARD);
        vita_draw_text_centered(b1_x + (bw * 0.5f), by + 11.0f, VITA_COLOR_TEXT_WHITE, 0.95f, yes_label ? yes_label : "Yes (X)");

        vita_draw_rounded_rect(b2_x, by, bw, bh, 6.0f, choice == 1 ? VITA_COLOR_AMIGA_RED : VITA_COLOR_CARD);
        vita_draw_text_centered(b2_x + (bw * 0.5f), by + 11.0f, VITA_COLOR_TEXT_WHITE, 0.95f, no_label ? no_label : "No (O)");

        SDL_Flip(prSDLScreen);
        SDL_Delay(20);
    }
    return false;
}

void vita_draw_boing_ball_icon(float cx, float cy, float radius, float rot_angle)
{
    (void)rot_angle; /* Rotation is implicit in the animated angle increments */
    if (!prSDLScreen || radius < 2.0f) return;

    Uint32 white_col  = to_sdl_color(RGBA8(240, 240, 240, 255));
    Uint32 red_col    = to_sdl_color(VITA_COLOR_AMIGA_RED);
    Uint32 border_col = to_sdl_color(RGBA8(16, 20, 30, 255));

    int ri = (int)radius;
    float r2 = (float)ri * (float)ri;

    /* Subtle dark outline for contrast against the card background */
    float r_out = (float)ri + 1.0f;
    float r_out2 = r_out * r_out;
    for (int row = -ri - 1; row <= ri + 1; row++) {
        float chord = sqrtf(r_out2 - (float)(row * row));
        int half = (int)(chord + 0.5f);
        if (half < 0) half = 0;
        SDL_Rect seg = { (Sint16)(cx - half), (Sint16)(cy + row), (Uint16)(half * 2), 1 };
        SDL_FillRect(prSDLScreen, &seg, border_col);
    }

    /* Classic red/white checkered ball (4x4 grid masked to the circle) */
    int grid = 4;
    float cell = (radius * 2.0f) / (float)grid;
    for (int row = -ri; row <= ri; row++) {
        float chord = sqrtf(r2 - (float)(row * row));
        int half = (int)(chord + 0.5f);
        if (half < 0) half = 0;

        int gy = (int)(((float)row + radius) / cell);
        if (gy < 0) gy = 0;
        if (gy >= grid) gy = grid - 1;

        int col = -half;
        while (col < half) {
            int gx = (int)(((float)col + radius) / cell);
            if (gx < 0) gx = 0;
            if (gx >= grid) gx = grid - 1;

            bool is_red = ((gx + gy) & 1) != 0;

            /* Fill up to the next grid-cell boundary or the circle edge */
            int next_x = (int)((float)(gx + 1) * cell) - ri;
            if (next_x > half) next_x = half;
            if (next_x <= col) next_x = col + 1;

            SDL_Rect seg = { (Sint16)(cx + col), (Sint16)(cy + row), (Uint16)(next_x - col), 1 };
            SDL_FillRect(prSDLScreen, &seg, is_red ? red_col : white_col);
            col = next_x;
        }
    }
}

int run_overlay_vita(void)
{
    if (vita_gui_init() != 0)
        return 1;

    VitaInputState input;
    VitaSystemInfo sysinfo;
    const char *items[6] = { "Resume", "Save State", "Load State", "Eject DF0", "Eject CD32", "Screenshot" };
    int selected = 0;
    int frame_count = 0;
    memset(&input, 0, sizeof(input));
    memset(&sysinfo, 0, sizeof(sysinfo));
    inside_menu = 1;

    while (1) {
        vita_gui_update_input(&input);
        vita_gui_update_system_info(&sysinfo);
        frame_count++;

        if (input.pressed & (SCE_CTRL_CIRCLE | SCE_CTRL_START)) {
            mainMenu_case = MAIN_MENU_CASE_RUN;
            break;
        }
        if (input.pressed & SCE_CTRL_UP) {
            selected--;
            if (selected < 0) selected = 5;
        }
        if (input.pressed & SCE_CTRL_DOWN) {
            selected++;
            if (selected > 5) selected = 0;
        }
        if (input.pressed & SCE_CTRL_CROSS) {
            extern char *savestate_filename;
            extern char *screenshot_filename;
            if (selected == 0) {
                mainMenu_case = MAIN_MENU_CASE_RUN;
                break;
            } else if (selected == 1) {
                saveMenu_n_savestate = 1;
                make_savestate_filenames(savestate_filename, screenshot_filename);
                savestate_state = STATE_DOSAVE;
                mainMenu_case = MAIN_MENU_CASE_RUN;
                break;
            } else if (selected == 2) {
                saveMenu_n_savestate = 1;
                make_savestate_filenames(savestate_filename, screenshot_filename);
                FILE *state_file = fopen(savestate_filename, "rb");
                if (state_file) {
                    fclose(state_file);
                    savestate_state = STATE_DORESTORE;
                }
                mainMenu_case = MAIN_MENU_CASE_RUN;
                break;
            } else if (selected == 3) {
                uae4all_image_file0[0] = '\0';
                gui_update();
                mainMenu_case = MAIN_MENU_CASE_RUN;
                break;
            } else if (selected == 4) {
                cdrom_close_image();
                mainMenu_case = MAIN_MENU_CASE_RUN;
                break;
            } else {
                vita_screenshot_request = 1;
                mainMenu_case = MAIN_MENU_CASE_RUN;
                break;
            }
        }

        SDL_FillRect(prSDLScreen, NULL, to_sdl_color(VITA_COLOR_OVERLAY_BG));
        vita_draw_header("Quick Menu", VITA_TAB_FLOPPY, &sysinfo);
        for (int i = 0; i < 6; i++) {
            float y = 82.0f + (float)i * 58.0f;
            vita_draw_button_item(80.0f, y, VITA_SCREEN_W - 160.0f, 48.0f, items[i], NULL, NULL, selected == i, false);
        }
        vita_draw_footer("CROSS SELECT", "CIRCLE/START RESUME");
        SDL_Flip(prSDLScreen);
        SDL_Delay(16);
    }

    inside_menu = 0;
    return 1;
}

int run_mainMenu_vita(void)
{
    if (vita_gui_init() != 0) {
        write_log("[VITA] run_mainMenu_vita: gui init failed\n");
        return 1;
    }
    write_log("[VITA] run_mainMenu_vita: enter menu\n");
    inside_menu = 1;

    VitaInputState input;
    memset(&input, 0, sizeof(input));
    VitaSystemInfo sysinfo;
    memset(&sysinfo, 0, sizeof(sysinfo));

    mainMenu_case = -1;
    int menu_frame = 0;

    if (resetOnStartingApp) {
        resetOnStartingApp = false;
        mainMenu_case = MAIN_MENU_CASE_RESET;
    }

    while (mainMenu_case < 0) {
        menu_frame++;
        if (menu_frame <= 3)
            write_log("[VITA] menu: frame %d input begin\n", menu_frame);
        vita_gui_update_input(&input);
        if (menu_frame <= 3)
            write_log("[VITA] menu: frame %d input done buttons=0x%08x\n", menu_frame, input.pressed);
        vita_gui_update_system_info(&sysinfo);

        if (input.pressed & SCE_CTRL_LTRIGGER) {
            s_active_tab = (VitaGuiTab)((s_active_tab + VITA_TAB_COUNT - 1) % VITA_TAB_COUNT);
        }
        if (input.pressed & SCE_CTRL_RTRIGGER) {
            s_active_tab = (VitaGuiTab)((s_active_tab + 1) % VITA_TAB_COUNT);
        }

        if (input.touch_tap && input.touch_y >= 48 && input.touch_y <= 84) {
            float tab_w = (float)VITA_SCREEN_W / (float)VITA_TAB_COUNT;
            int touched_tab = (int)(input.touch_x / tab_w);
            if (touched_tab >= 0 && touched_tab < VITA_TAB_COUNT) {
                s_active_tab = (VitaGuiTab)touched_tab;
            }
        }

        if (input.pressed & SCE_CTRL_START) {
            write_log("[VITA] menu: Start pressed (kickstart_warning=%d)\n", kickstart_warning);
            if (kickstart_warning) {
                write_log("[VITA] run_mainMenu_vita: Start blocked, Kickstart missing\n");
                vita_show_message_box("Kickstart Missing", "Copy kick13.rom and kick31.rom for normal Amiga use, or kick40060.CD32 and kick40060.CD32.ext for CD32, to ux0:/data/uae4all/kickstarts/.", "OK (X)");
            } else {
                mainMenu_case = MAIN_MENU_CASE_RUN;
                break;
            }
        }

        /* Clear Frame */
        if (menu_frame <= 3)
            write_log("[VITA] menu: frame %d render begin\n", menu_frame);
        SDL_FillRect(prSDLScreen, NULL, to_sdl_color(VITA_COLOR_BG));

        /* Draw Header & Tab Bar */
        vita_draw_header("UAE4ALL2 VITA HD", s_active_tab, &sysinfo);
        vita_draw_tab_bar(s_active_tab, 48.0f);

        int *cur_sel = &s_tab_selected_item[s_active_tab];
        switch (s_active_tab) {
            case VITA_TAB_FLOPPY:
                vita_view_floppy(&input, cur_sel);
                break;
            case VITA_TAB_HARD_DISK:
                vita_view_hard_disk(&input, cur_sel);
                break;
            case VITA_TAB_WHDLOAD:
                vita_view_whdload(&input, cur_sel);
                break;
            case VITA_TAB_PRESETS:
                vita_view_presets(&input, cur_sel);
                break;
            case VITA_TAB_HARDWARE:
                vita_view_hardware(&input, cur_sel);
                break;
            case VITA_TAB_DISPLAY:
                vita_view_display(&input, cur_sel);
                break;
            case VITA_TAB_CONTROLS:
                vita_view_controls(&input, cur_sel);
                break;
            case VITA_TAB_SAVESTATES:
                vita_view_savestates(&input, cur_sel);
                break;
            case VITA_TAB_SYSTEM:
                vita_view_system(&input, cur_sel);
                break;
            default:
                break;
        }

        vita_draw_footer(NULL, NULL);

        SDL_Flip(prSDLScreen);
        if (menu_frame <= 3)
            write_log("[VITA] menu: frame %d render done\n", menu_frame);
        SDL_Delay(16);
    }

    inside_menu = 0;
    setCpuSpeed();
    write_log("[VITA] run_mainMenu_vita: exit menu (case=%d)\n", mainMenu_case);
    vita_gui_shutdown();

    return (mainMenu_case == MAIN_MENU_CASE_RESET) ? 2 : (mainMenu_case == MAIN_MENU_CASE_QUIT ? 0 : 1);
}

#endif
