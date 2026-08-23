#ifndef UAE_GUI_VITA_H
#define UAE_GUI_VITA_H

#ifdef __PSP2__

#include <psp2/types.h>
#include <psp2/ctrl.h>
#include <psp2/touch.h>
#include <psp2/power.h>
#include <psp2/rtc.h>
#include "vita2d_fbo/includes/vita2d.h"

#ifdef __cplusplus
extern "C" {
#endif

#define VITA_SCREEN_W 960
#define VITA_SCREEN_H 544

#ifndef RGBA8
#define RGBA8(r,g,b,a) ((((a)&0xFF)<<24) | (((b)&0xFF)<<16) | (((g)&0xFF)<<8) | (((r)&0xFF)<<0))
#endif

#define VITA_COLOR_BG            RGBA8(15, 18, 26, 255)       /* Deep slate/charcoal */
#define VITA_COLOR_HEADER        RGBA8(22, 27, 39, 255)       /* Top bar background */
#define VITA_COLOR_FOOTER        RGBA8(20, 24, 34, 255)       /* Bottom bar background */
#define VITA_COLOR_CARD          RGBA8(28, 34, 48, 230)       /* Translucent card */
#define VITA_COLOR_CARD_FOCUSED  RGBA8(38, 48, 70, 255)       /* Focused item card */
#define VITA_COLOR_CARD_BORDER   RGBA8(50, 60, 85, 255)       /* Subtle border */
#define VITA_COLOR_FOCUS_BORDER  RGBA8(229, 37, 33, 255)      /* Amiga Red highlight */
#define VITA_COLOR_ACTIVE_PILL   RGBA8(229, 37, 33, 255)      /* Active tab pill */
#define VITA_COLOR_AMIGA_RED     RGBA8(229, 37, 33, 255)      /* Classic Boing Red */
#define VITA_COLOR_AMIGA_BLUE    RGBA8(0, 136, 204, 255)      /* Amiga Blue */
#define VITA_COLOR_AMIGA_ORANGE  RGBA8(255, 140, 0, 255)      /* Floppy LED Amber */
#define VITA_COLOR_AMIGA_GREEN   RGBA8(34, 197, 94, 255)      /* Power LED Green */
#define VITA_COLOR_TEXT_WHITE    RGBA8(255, 255, 255, 255)    /* Primary text */
#define VITA_COLOR_TEXT_MUTED    RGBA8(148, 163, 184, 255)    /* Secondary text */
#define VITA_COLOR_TEXT_DIM      RGBA8(90, 105, 125, 255)     /* Disabled text */
#define VITA_COLOR_ACCENT_GOLD   RGBA8(245, 158, 11, 255)     /* Rating/Star */
#define VITA_COLOR_SUCCESS       RGBA8(16, 185, 129, 255)     /* Success badge */
#define VITA_COLOR_DANGER        RGBA8(239, 68, 68, 255)      /* Danger/Eject */
#define VITA_COLOR_OVERLAY_BG    RGBA8(8, 10, 16, 210)        /* Modal/Dark backdrop */

typedef enum {
    VITA_TAB_FLOPPY = 0,
    VITA_TAB_HARD_DISK,
    VITA_TAB_WHDLOAD,
    VITA_TAB_PRESETS,
    VITA_TAB_HARDWARE,
    VITA_TAB_DISPLAY,
    VITA_TAB_CONTROLS,
    VITA_TAB_SAVESTATES,
    VITA_TAB_SYSTEM,
    VITA_TAB_COUNT
} VitaGuiTab;

typedef struct {
    SceCtrlData pad;
    SceCtrlData prev_pad;
    unsigned int pressed;
    unsigned int released;
    unsigned int held;
    int touch_x;
    int touch_y;
    int touch_active;
    int touch_tap;
    int touch_hold_frames;
} VitaInputState;

typedef struct {
    int battery_percent;
    bool is_charging;
    char time_str[16];
    char date_str[32];
} VitaSystemInfo;

int  vita_set_kickstart(int index, int load_rom);
int  vita_confirm_eject_for_hard_disk_launch(void);
int  vita_gui_init(void);
void vita_gui_shutdown(void);
void vita_gui_shutdown_final(void);
void vita_gui_prepare_exit(void);
void vita_gui_update_input(VitaInputState *input);
void vita_gui_update_system_info(VitaSystemInfo *sysinfo);

void vita_draw_rounded_rect(float x, float y, float w, float h, float r, unsigned int color);
void vita_draw_rounded_rect_outline(float x, float y, float w, float h, float r, float thickness, unsigned int color);
void vita_draw_card(float x, float y, float w, float h, bool focused, bool active);
void vita_draw_card_custom(float x, float y, float w, float h, unsigned int bg_col, unsigned int border_col);
void vita_draw_header(const char *title, VitaGuiTab current_tab, const VitaSystemInfo *sysinfo);
void vita_draw_footer(const char *left_hint, const char *right_hint);
void vita_draw_tab_bar(VitaGuiTab current_tab, float y);

typedef enum {
    VITA_BTN_CROSS = 0,
    VITA_BTN_CIRCLE,
    VITA_BTN_SQUARE,
    VITA_BTN_TRIANGLE,
    VITA_BTN_L,
    VITA_BTN_R,
    VITA_BTN_START,
    VITA_BTN_SELECT,
    VITA_BTN_DPAD,
    VITA_BTN_ANALOG
} VitaButtonGlyph;

void vita_draw_text(float x, float y, unsigned int color, float scale, const char *text);
void vita_draw_textf(float x, float y, unsigned int color, float scale, const char *fmt, ...);
void vita_draw_text_centered(float cx, float y, unsigned int color, float scale, const char *text);
void vita_draw_text_right(float rx, float y, unsigned int color, float scale, const char *text);
void vita_draw_text_wrapped(float x, float y, float max_w, unsigned int color, float scale, const char *text);
void vita_truncate_text(const char *in_text, float max_w, float scale, char *out_buf, size_t out_size);
int  vita_get_text_width(float scale, const char *text);
int  vita_get_text_height(float scale);

void vita_draw_button_glyph(float x, float y, VitaButtonGlyph glyph);
void vita_draw_hint_item(float x, float y, VitaButtonGlyph glyph, const char *label);
void vita_draw_badge(float x, float y, const char *label, unsigned int bg_col, unsigned int text_col);
void vita_draw_boing_ball_icon(float cx, float cy, float radius, float rot_angle);
void vita_draw_led(float x, float y, const char *label, bool state, unsigned int led_col);
void vita_draw_button_item(float x, float y, float w, float h, const char *title, const char *subtitle, const char *badge, bool focused, bool active);
void vita_draw_button_item_custom(float x, float y, float w, float h, const char *title, const char *subtitle, const char *badge, unsigned int badge_col, bool focused, bool active);
void vita_draw_selector_item(float x, float y, float w, float h, const char *title, const char *current_value, bool focused);
void vita_draw_switch_item(float x, float y, float w, float h, const char *title, bool enabled, bool focused);
void vita_draw_slider_item(float x, float y, float w, float h, const char *title, int val, int min, int max, const char *suffix, bool focused);

void vita_show_message_box(const char *title, const char *message, const char *btn_label);
void vita_show_about_box(void);
bool vita_show_confirm_box(const char *title, const char *message, const char *yes_label, const char *no_label);
void vita_gui_draw_progress(const char *title, const char *subtitle, float fraction, const char *item_name);

void vita_view_floppy(VitaInputState *input, int *selected_item);
void vita_view_hard_disk(VitaInputState *input, int *selected_item);
void vita_view_whdload(VitaInputState *input, int *selected_item);
void vita_view_presets(VitaInputState *input, int *selected_item);
void vita_view_hardware(VitaInputState *input, int *selected_item);
void vita_view_display(VitaInputState *input, int *selected_item);
void vita_view_controls(VitaInputState *input, int *selected_item);
void vita_view_savestates(VitaInputState *input, int *selected_item);
void vita_view_system(VitaInputState *input, int *selected_item);

int  vita_gui_run_browser(char *out_path, const char *start_dir, int disk_drive_idx);

int  run_mainMenu_vita(void);
int  run_overlay_vita(void);

#ifdef __cplusplus
}
#endif

#endif
#endif
