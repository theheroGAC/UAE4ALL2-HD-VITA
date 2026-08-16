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
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},          
    {0x18,0x3C,0x3C,0x18,0x18,0x00,0x18,0x00},          
    {0x66,0x66,0x24,0x00,0x00,0x00,0x00,0x00},          
    {0x6C,0x6C,0xFE,0x6C,0xFE,0x6C,0x6C,0x00},          
    {0x18,0x7E,0x18,0x3C,0x66,0x3C,0x18,0x7E},          
    {0x00,0xC6,0xCC,0x18,0x30,0x66,0xC6,0x00},          
    {0x38,0x6C,0x38,0x76,0xDC,0xCC,0x76,0x00},          
    {0x18,0x18,0x30,0x00,0x00,0x00,0x00,0x00},          
    {0x0C,0x18,0x30,0x30,0x30,0x18,0x0C,0x00},          
    {0x30,0x18,0x0C,0x0C,0x0C,0x18,0x30,0x00},          
    {0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00},          
    {0x00,0x18,0x18,0x7E,0x18,0x18,0x00,0x00},          
    {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x30},          
    {0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00},          
    {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00},          
    {0x06,0x0C,0x18,0x30,0x60,0xC0,0x80,0x00},          
    {0x3C,0x66,0x6E,0x76,0x66,0x66,0x3C,0x00},          
    {0x18,0x38,0x18,0x18,0x18,0x18,0x7E,0x00},          
    {0x3C,0x66,0x06,0x0C,0x18,0x30,0x7E,0x00},          
    {0x3C,0x66,0x06,0x1C,0x06,0x66,0x3C,0x00},          
    {0x0C,0x1C,0x34,0x64,0x7E,0x0C,0x0C,0x00},          
    {0x7E,0x60,0x7C,0x06,0x06,0x66,0x3C,0x00},          
    {0x1C,0x30,0x60,0x7C,0x66,0x66,0x3C,0x00},          
    {0x7E,0x06,0x0C,0x18,0x30,0x30,0x30,0x00},          
    {0x3C,0x66,0x66,0x3C,0x66,0x66,0x3C,0x00},          
    {0x3C,0x66,0x66,0x3E,0x06,0x0C,0x38,0x00},          
    {0x00,0x18,0x18,0x00,0x18,0x18,0x00,0x00},          
    {0x00,0x18,0x18,0x00,0x18,0x18,0x30,0x00},          
    {0x0C,0x18,0x30,0x60,0x30,0x18,0x0C,0x00},          
    {0x00,0x00,0x7E,0x00,0x7E,0x00,0x00,0x00},          
    {0x30,0x18,0x0C,0x06,0x0C,0x18,0x30,0x00},          
    {0x3C,0x66,0x06,0x0C,0x18,0x00,0x18,0x00},          
    {0x3C,0x66,0x6E,0x6E,0x60,0x62,0x3C,0x00},          
    {0x18,0x3C,0x66,0x66,0x7E,0x66,0x66,0x00},          
    {0x7C,0x66,0x66,0x7C,0x66,0x66,0x7C,0x00},          
    {0x3C,0x66,0x60,0x60,0x60,0x66,0x3C,0x00},          
    {0x78,0x6C,0x66,0x66,0x66,0x6C,0x78,0x00},          
    {0x7E,0x60,0x60,0x7C,0x60,0x60,0x7E,0x00},          
    {0x7E,0x60,0x60,0x7C,0x60,0x60,0x60,0x00},          
    {0x3C,0x66,0x60,0x6E,0x66,0x66,0x3C,0x00},          
    {0x66,0x66,0x66,0x7E,0x66,0x66,0x66,0x00},          
    {0x7E,0x18,0x18,0x18,0x18,0x18,0x7E,0x00},          
    {0x06,0x06,0x06,0x06,0x06,0x66,0x3C,0x00},          
    {0x66,0x6C,0x78,0x70,0x78,0x6C,0x66,0x00},          
    {0x60,0x60,0x60,0x60,0x60,0x60,0x7E,0x00},          
    {0x63,0x77,0x7F,0x6B,0x63,0x63,0x63,0x00},          
    {0x66,0x76,0x7E,0x7E,0x6E,0x66,0x66,0x00},          
    {0x3C,0x66,0x66,0x66,0x66,0x66,0x3C,0x00},          
    {0x7C,0x66,0x66,0x7C,0x60,0x60,0x60,0x00},          
    {0x3C,0x66,0x66,0x66,0x6E,0x3C,0x0E,0x00},          
    {0x7C,0x66,0x66,0x7C,0x78,0x6C,0x66,0x00},          
    {0x3C,0x66,0x60,0x3C,0x06,0x66,0x3C,0x00},          
    {0x7E,0x18,0x18,0x18,0x18,0x18,0x18,0x00},          
    {0x66,0x66,0x66,0x66,0x66,0x66,0x3C,0x00},          
    {0x66,0x66,0x66,0x66,0x66,0x3C,0x18,0x00},          
    {0x63,0x63,0x63,0x6B,0x7F,0x77,0x63,0x00},          
    {0x66,0x66,0x3C,0x18,0x3C,0x66,0x66,0x00},          
    {0x66,0x66,0x66,0x3C,0x18,0x18,0x18,0x00},          
    {0x7E,0x06,0x0C,0x18,0x30,0x60,0x7E,0x00},          
    {0x3C,0x30,0x30,0x30,0x30,0x30,0x3C,0x00},          
    {0xC0,0x60,0x30,0x18,0x0C,0x06,0x02,0x00},          
    {0x3C,0x0C,0x0C,0x0C,0x0C,0x0C,0x3C,0x00},          
    {0x18,0x3C,0x66,0x00,0x00,0x00,0x00,0x00},          
    {0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00},          
    {0x30,0x18,0x0C,0x00,0x00,0x00,0x00,0x00},          
    {0x00,0x00,0x3C,0x06,0x3E,0x66,0x3E,0x00},          
    {0x60,0x60,0x7C,0x66,0x66,0x66,0x7C,0x00},          
    {0x00,0x00,0x3C,0x66,0x60,0x66,0x3C,0x00},          
    {0x06,0x06,0x3E,0x66,0x66,0x66,0x3E,0x00},           
    {0x00,0x00,0x3C,0x66,0x7E,0x60,0x3C,0x00},           
    {0x1C,0x30,0x7C,0x30,0x30,0x30,0x30,0x00},           
    {0x00,0x00,0x3E,0x66,0x66,0x3E,0x06,0x3C},           
    {0x60,0x60,0x7C,0x66,0x66,0x66,0x66,0x00},           
    {0x18,0x00,0x38,0x18,0x18,0x18,0x3C,0x00},           
    {0x0C,0x00,0x1C,0x0C,0x0C,0x0C,0x6C,0x38},           
    {0x60,0x60,0x66,0x6C,0x78,0x6C,0x66,0x00},           
    {0x38,0x18,0x18,0x18,0x18,0x18,0x3C,0x00},           
    {0x00,0x00,0x66,0x7F,0x7B,0x63,0x63,0x00},           
    {0x00,0x00,0x7C,0x66,0x66,0x66,0x66,0x00},           
    {0x00,0x00,0x3C,0x66,0x66,0x66,0x3C,0x00},           
    {0x00,0x00,0x7C,0x66,0x66,0x7C,0x60,0x60},           
    {0x00,0x00,0x3E,0x66,0x66,0x3E,0x06,0x06},           
    {0x00,0x00,0x7C,0x66,0x60,0x60,0x60,0x00},           
    {0x00,0x00,0x3E,0x60,0x3C,0x06,0x7C,0x00},           
    {0x18,0x18,0x7E,0x18,0x18,0x18,0x0C,0x00},           
    {0x00,0x00,0x66,0x66,0x66,0x66,0x3E,0x00},           
    {0x00,0x00,0x66,0x66,0x66,0x3C,0x18,0x00},           
    {0x00,0x00,0x63,0x6B,0x7F,0x77,0x36,0x00},           
    {0x00,0x00,0x66,0x3C,0x18,0x3C,0x66,0x00},           
    {0x00,0x00,0x66,0x66,0x66,0x3E,0x06,0x3C},           
    {0x00,0x00,0x7E,0x0C,0x18,0x30,0x7E,0x00},           
    {0x0E,0x18,0x18,0x70,0x18,0x18,0x0E,0x00},           
    {0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x00},           
    {0x70,0x18,0x18,0x0E,0x18,0x18,0x70,0x00},           
    {0x76,0xDC,0x00,0x00,0x00,0x00,0x00,0x00},           
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}        
};

                                                    
static const unsigned char s_char_widths[96] = {
    4, 3, 5, 7, 7, 7, 7, 3, 4, 4, 6, 7, 3, 6, 3, 6,         
    7, 5, 7, 7, 7, 7, 7, 7, 7, 7, 3, 3, 5, 7, 5, 7,         
    7, 7, 7, 7, 7, 7, 7, 7, 7, 4, 6, 7, 6, 8, 7, 7,         
    7, 7, 7, 7, 7, 7, 7, 8, 7, 7, 7, 4, 6, 4, 5, 7,         
    3, 6, 6, 6, 6, 6, 5, 6, 3, 4, 6, 3, 8, 6, 6, 6,          
    6, 5, 6, 5, 6, 6, 8, 6, 6, 6, 4, 3, 4, 7, 0                
};

               
static const char *s_tab_names[VITA_TAB_COUNT] = {
    "Floppy",
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

                                                                               
    vita_gui_free_screen();
    write_log("[VITA] vita_gui_init: free_screen done\n");

    displaying_menu = 1;

                                                   
    prSDLScreen = SDL_SetVideoMode(VITA_SCREEN_W, VITA_SCREEN_H, 16, SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (prSDLScreen == NULL) {
                                                                         
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
                                                                               
                                                                               
                                              
    s_gui_initialized = false;
}

void vita_gui_shutdown_final(void)
{
                                                                            
                                                                        
    s_gui_initialized = false;
    vita_gui_free_screen();
    displaying_menu = 0;
}

void vita_gui_prepare_exit(void)
{
                                                                              
                                                                          
                                                                 
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

                                                                    
    if (input->pad.ly < 50)  input->pad.buttons |= SCE_CTRL_UP;
    if (input->pad.ly > 205) input->pad.buttons |= SCE_CTRL_DOWN;
    if (input->pad.lx < 50)  input->pad.buttons |= SCE_CTRL_LEFT;
    if (input->pad.lx > 205) input->pad.buttons |= SCE_CTRL_RIGHT;

    input->pressed  = input->pad.buttons & ~input->prev_pad.buttons;
    input->released = ~input->pad.buttons & input->prev_pad.buttons;
    input->held     = input->pad.buttons;

                              
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

                                
    int mid_y = yi + ri;
    int mid_h = hi - 2 * ri;
    if (mid_h > 0) {
        SDL_Rect mid = { (Sint16)xi, (Sint16)mid_y, (Uint16)wi, (Uint16)mid_h };
        SDL_FillRect(prSDLScreen, &mid, sdl_col);
    }

                                                   
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
                for (int col = 0; col < char_w; col++) {
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
        vita_draw_text(x + offset_x, y + 4.0f, RGBA8(220, 230, 245, 255), 0.90f, label);
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

                                        
    s_boing_angle += 0.03f;
    vita_draw_boing_ball_icon(24.0f, 24.0f, 14.0f, s_boing_angle);

                    
    vita_draw_text(46.0f, 15.0f, VITA_COLOR_TEXT_WHITE, 1.10f, "UAE4ALL2");
    vita_draw_badge(210.0f, 13.0f, "HD VITA", VITA_COLOR_AMIGA_RED, VITA_COLOR_TEXT_WHITE);

                     
    const char *model_str = (changed_prefs.cpu_level == M68020) ? "Amiga 1200 (AGA)" : "Amiga 500 (OCS)";
    vita_draw_badge(340.0f, 13.0f, model_str, RGBA8(30, 41, 59, 255), VITA_COLOR_TEXT_MUTED);

                                      
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

        if (active) {
            vita_draw_rounded_rect(tx + 4.0f, y + 4.0f, tab_w - 8.0f, 28.0f, 5.0f, VITA_COLOR_AMIGA_RED);
            vita_draw_text_centered(tx + (tab_w * 0.5f), y + 7.0f, VITA_COLOR_TEXT_WHITE, 0.90f, s_tab_names[i]);
        } else {
            vita_draw_text_centered(tx + (tab_w * 0.5f), y + 7.0f, VITA_COLOR_TEXT_MUTED, 0.85f, s_tab_names[i]);
        }
    }
}

void vita_draw_footer(const char *left_hint, const char *right_hint)
{
    SDL_Rect ftr_r = { 0, VITA_SCREEN_H - 42, VITA_SCREEN_W, 42 };
    SDL_FillRect(prSDLScreen, &ftr_r, to_sdl_color(VITA_COLOR_FOOTER));
    vita_draw_rounded_rect_outline(0, VITA_SCREEN_H - 42, VITA_SCREEN_W, 42, 0.0f, 1.0f, VITA_COLOR_CARD_BORDER);

    float btn_y = VITA_SCREEN_H - 32.0f;

                                                                         
    const char *triangle_label = (s_active_tab == VITA_TAB_FLOPPY) ? "EJECT" : "OPTIONS";
    vita_draw_hint_item(20.0f,  btn_y, VITA_BTN_CROSS,    "SELECT");
    vita_draw_hint_item(145.0f, btn_y, VITA_BTN_CIRCLE,   "BACK");
    vita_draw_hint_item(275.0f, btn_y, VITA_BTN_TRIANGLE, triangle_label);
    vita_draw_hint_item(420.0f, btn_y, VITA_BTN_SQUARE,   "QUICK");

                                
    vita_draw_hint_item(650.0f, btn_y, VITA_BTN_L,     "");
    vita_draw_hint_item(700.0f, btn_y, VITA_BTN_R,     "TAB");
    vita_draw_hint_item(780.0f, btn_y, VITA_BTN_START, "RESUME");
}

void vita_draw_button_item(float x, float y, float w, float h, const char *title, const char *subtitle, const char *badge, bool focused, bool active)
{
    vita_draw_card(x, y, w, h, focused, active);

    char title_buf[128];
    float max_title_w = badge ? (w - 180.0f) : (w - 40.0f);
    vita_truncate_text(title, max_title_w, 0.95f, title_buf, sizeof(title_buf));

    float text_y = subtitle ? (y + 10.0f) : (y + 16.0f);
    unsigned int title_col = focused ? VITA_COLOR_TEXT_WHITE : RGBA8(220, 230, 245, 255);
    vita_draw_text(x + 16.0f, text_y, title_col, 0.95f, title_buf);

    if (subtitle) {
        char sub_buf[256];
        vita_truncate_text(subtitle, w - 40.0f, 0.78f, sub_buf, sizeof(sub_buf));
        vita_draw_text(x + 16.0f, y + 32.0f, VITA_COLOR_TEXT_MUTED, 0.78f, sub_buf);
    }

    if (badge) {
        unsigned int badge_col = focused ? VITA_COLOR_AMIGA_BLUE : RGBA8(40, 52, 75, 255);
        int bw = vita_get_text_width(0.80f, badge) + 14;
        vita_draw_badge(x + w - (float)bw - 16.0f, y + 14.0f, badge, badge_col, VITA_COLOR_TEXT_WHITE);
    }
}

void vita_draw_selector_item(float x, float y, float w, float h, const char *title, const char *current_value, bool focused)
{
    vita_draw_card(x, y, w, h, focused, false);
    vita_draw_text(x + 16.0f, y + 16.0f, focused ? VITA_COLOR_TEXT_WHITE : RGBA8(220, 230, 245, 255), 0.95f, title);

    char val_str[128];
    if (focused) {
        snprintf(val_str, sizeof(val_str), "<  %s  >", current_value ? current_value : "None");
        vita_draw_text_right(x + w - 16.0f, y + 16.0f, VITA_COLOR_FOCUS_BORDER, 0.95f, val_str);
    } else {
        snprintf(val_str, sizeof(val_str), "%s", current_value ? current_value : "None");
        vita_draw_text_right(x + w - 16.0f, y + 16.0f, VITA_COLOR_TEXT_MUTED, 0.90f, val_str);
    }
}

void vita_draw_switch_item(float x, float y, float w, float h, const char *title, bool enabled, bool focused)
{
    vita_draw_card(x, y, w, h, focused, false);
    vita_draw_text(x + 16.0f, y + 16.0f, focused ? VITA_COLOR_TEXT_WHITE : RGBA8(220, 230, 245, 255), 0.95f, title);

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

bool vita_show_confirm_box(const char *title, const char *message, const char *yes_label, const char *no_label)
{
                                                                         
                                                                             
                                                      
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
    (void)rot_angle;                                                            
    if (!prSDLScreen || radius < 2.0f) return;

    Uint32 white_col  = to_sdl_color(RGBA8(240, 240, 240, 255));
    Uint32 red_col    = to_sdl_color(VITA_COLOR_AMIGA_RED);
    Uint32 border_col = to_sdl_color(RGBA8(16, 20, 30, 255));

    int ri = (int)radius;
    float r2 = (float)ri * (float)ri;

                                                                      
    float r_out = (float)ri + 1.0f;
    float r_out2 = r_out * r_out;
    for (int row = -ri - 1; row <= ri + 1; row++) {
        float chord = sqrtf(r_out2 - (float)(row * row));
        int half = (int)(chord + 0.5f);
        if (half < 0) half = 0;
        SDL_Rect seg = { (Sint16)(cx - half), (Sint16)(cy + row), (Uint16)(half * 2), 1 };
        SDL_FillRect(prSDLScreen, &seg, border_col);
    }

                                                                          
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

                                                                           
            int next_x = (int)((float)(gx + 1) * cell) - ri;
            if (next_x > half) next_x = half;
            if (next_x <= col) next_x = col + 1;

            SDL_Rect seg = { (Sint16)(cx + col), (Sint16)(cy + row), (Uint16)(next_x - col), 1 };
            SDL_FillRect(prSDLScreen, &seg, is_red ? red_col : white_col);
            col = next_x;
        }
    }
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
                vita_show_message_box("Kickstart Missing", "Cannot start emulation without a Kickstart ROM. Copy kick13.rom and kick31.rom to ux0:/data/uae4all/kickstarts/.", "OK (X)");
            } else {
                mainMenu_case = MAIN_MENU_CASE_RUN;
                break;
            }
        }

                         
        if (menu_frame <= 3)
            write_log("[VITA] menu: frame %d render begin\n", menu_frame);
        SDL_FillRect(prSDLScreen, NULL, to_sdl_color(VITA_COLOR_BG));

                                   
        vita_draw_header("UAE4ALL2 VITA HD", s_active_tab, &sysinfo);
        vita_draw_tab_bar(s_active_tab, 48.0f);

        int *cur_sel = &s_tab_selected_item[s_active_tab];
        switch (s_active_tab) {
            case VITA_TAB_FLOPPY:
                vita_view_floppy(&input, cur_sel);
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
