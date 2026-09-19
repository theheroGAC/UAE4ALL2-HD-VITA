#include "sysconfig.h"
#include "sysdeps.h"
#include "config.h"

#include "uae.h"
#include "options.h"
#include "menu.h"
#include "menu_config.h"
#include "sound.h"
#include "disk.h"
#include "memory-uae.h"
#include "custom.h"
#include "xwin.h"
#include "drawing.h"

#if defined(ANDROID)
#include <SDL_screenkeyboard.h>
#include <android/log.h>
#endif

#ifdef USE_SDL2
#include "sdl2_to_sdl1.h"
#endif

#if defined(__PSP2__) // NOT __SWITCH__
#include "psp2_shader.h"
#include "vita2d_fbo/includes/vita2d.h"
PSP2Shader *shader = NULL;
extern int mainMenu_shader;
#ifndef PRIVATE_HW_DATA
#define PRIVATE_HW_DATA
typedef struct private_hwdata {
	vita2d_texture *texture;
	SDL_Rect dst;
} private_hwdata;
#endif //PRIVATE_HW_DATA
#endif

#ifdef __SWITCH__
#include <switch.h>
#endif

#if defined(__PSP2__) // NOT __SWITCH__
//Allow locking PS Button
#include <psp2/shellutil.h>
#endif

extern int screenWidth;
extern int mainMenu_case;

extern int lastCpuSpeed;
extern int ntsc;

extern char launchDir[300];
extern char currentDir[300];

extern int displaying_menu;

extern void gp2x_stop_sound(void);

#ifdef __SWITCH__
extern void update_joycon_mode();
#endif


int saveAdfDir() {
    char path[300];
    snprintf(path, 300, "%s/conf/adfdir.conf", launchDir);
    FILE *f = fopen(path, "w");
    if (!f) return 0;
    char buffer[310];
    snprintf((char *) buffer, 310, "path=%s\n", currentDir);
    fputs(buffer, f);
    fclose(f);
    return 1;
}

void extractFileName(char *str, char *buffer) {
    if (!buffer) return;
    buffer[0] = '\0';
    if (!str || str[0] == '\0') return;

    const char *p = strrchr(str, '/');
    const char *backslash = strrchr(str, '\\');
    if (backslash && (!p || backslash > p)) p = backslash;
    if (p) p++;
    else p = str;

    strncpy(buffer, p, 254);
    buffer[254] = '\0';
}

void stateFilenameToThumbFilename(char *src, char *dst) {
    if (!dst) return;
    char buffer[255] = "";
    extractFileName(src, buffer);
    if (buffer[0] == '\0') {
        dst[0] = '\0';
        return;
    }

    char *ext = strrchr(buffer, '.');
    if (ext && ext > buffer) {
        *ext = '\0';
    }
    snprintf(dst, 255, "%s%s.png", THUMB_PREFIX, buffer);
    dst[254] = '\0';
}

void exit_safely(int quit_via_home) {
#ifndef USE_SDLSOUND
	gp2x_stop_sound();
#endif
    saveAdfDir();	
    
#if defined(__PSP2__) // NOT __SWITCH__
    //unlock PS Button
    sceShellUtilUnlock(SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN);
#endif
    
    leave_program();

#if !defined(__PSP2__) && !defined(__SWITCH__)
    sync();
#endif
#ifdef __SWITCH__
    mainMenu_singleJoycons = 0;
    update_joycon_mode();
#endif
    exit(0);
}

#ifdef ANDROIDSDL
void update_onscreen()
{
    SDL_ANDROID_SetScreenKeyboardFloatingJoystick(mainMenu_FloatingJoystick);
    if (mainMenu_case != MAIN_MENU_CASE_DISPLAY && mainMenu_case != MAIN_MENU_CASE_MEMDISK && mainMenu_onScreen==0)
    {
      SDL_ANDROID_SetScreenKeyboardShown(0);
    }
    else
    {
      SDL_ANDROID_SetScreenKeyboardShown(1);
        SDL_Rect pos_textinput, pos_dpad, pos_button1, pos_button2, pos_button3, pos_button4, pos_button5, pos_button6;
        pos_textinput.x = mainMenu_pos_x_textinput*(SDL_ListModes(NULL, 0)[0]->w/(float)480);
        pos_textinput.y = mainMenu_pos_y_textinput*(SDL_ListModes(NULL, 0)[0]->h/(float)360);
        pos_textinput.h=SDL_ListModes(NULL, 0)[0]->h / (float)10 * mainMenu_button_size;
        pos_textinput.w=pos_textinput.h;
        SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_TEXT, &pos_textinput);
        pos_dpad.x = mainMenu_pos_x_dpad*(SDL_ListModes(NULL, 0)[0]->w/(float)480);
        pos_dpad.y = mainMenu_pos_y_dpad*(SDL_ListModes(NULL, 0)[0]->h/(float)360);
        pos_dpad.h=SDL_ListModes(NULL, 0)[0]->h / (float)2.5 * mainMenu_button_size;
        pos_dpad.w=pos_dpad.h;
        SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD, &pos_dpad);
        pos_button1.x = mainMenu_pos_x_button1*(SDL_ListModes(NULL, 0)[0]->w/(float)480);
        pos_button1.y = mainMenu_pos_y_button1*(SDL_ListModes(NULL, 0)[0]->h/(float)360);
        pos_button1.h=SDL_ListModes(NULL, 0)[0]->h / (float)5 * mainMenu_button_size;
        pos_button1.w=pos_button1.h;
        SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_0, &pos_button1);
        pos_button2.x = mainMenu_pos_x_button2*(SDL_ListModes(NULL, 0)[0]->w/(float)480);
        pos_button2.y = mainMenu_pos_y_button2*(SDL_ListModes(NULL, 0)[0]->h/(float)360);
        pos_button2.h=SDL_ListModes(NULL, 0)[0]->h / (float)5 * mainMenu_button_size;
        pos_button2.w=pos_button2.h;
        SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_1, &pos_button2);
        pos_button3.x = mainMenu_pos_x_button3*(SDL_ListModes(NULL, 0)[0]->w/(float)480);
        pos_button3.y = mainMenu_pos_y_button3*(SDL_ListModes(NULL, 0)[0]->h/(float)360);
        pos_button3.h=SDL_ListModes(NULL, 0)[0]->h / (float)5 * mainMenu_button_size;
        pos_button3.w=pos_button3.h;
        SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_2, &pos_button3);
        pos_button4.x = mainMenu_pos_x_button4*(SDL_ListModes(NULL, 0)[0]->w/(float)480);
        pos_button4.y = mainMenu_pos_y_button4*(SDL_ListModes(NULL, 0)[0]->h/(float)360);
        pos_button4.h=SDL_ListModes(NULL, 0)[0]->h / (float)5 * mainMenu_button_size;
        pos_button4.w=pos_button4.h;
        SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_3, &pos_button4);
        pos_button5.x = mainMenu_pos_x_button5*(SDL_ListModes(NULL, 0)[0]->w/(float)480);
        pos_button5.y = mainMenu_pos_y_button5*(SDL_ListModes(NULL, 0)[0]->h/(float)360);
        pos_button5.h=SDL_ListModes(NULL, 0)[0]->h / (float)5 * mainMenu_button_size;
        pos_button5.w=pos_button5.h;
        SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_4, &pos_button5);
        pos_button6.x = mainMenu_pos_x_button6*(SDL_ListModes(NULL, 0)[0]->w/(float)480);
        pos_button6.y = mainMenu_pos_y_button6*(SDL_ListModes(NULL, 0)[0]->h/(float)360);
        pos_button6.h=SDL_ListModes(NULL, 0)[0]->h / (float)5 * mainMenu_button_size;
        pos_button6.w=pos_button6.h;
        SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_5, &pos_button6);

        SDL_ANDROID_SetScreenKeyboardButtonShown(SDL_ANDROID_SCREENKEYBOARD_BUTTON_TEXT, mainMenu_onScreen_textinput);
        SDL_ANDROID_SetScreenKeyboardButtonShown(SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD, mainMenu_onScreen_dpad);
        SDL_ANDROID_SetScreenKeyboardButtonShown(SDL_ANDROID_SCREENKEYBOARD_BUTTON_0, mainMenu_onScreen_button1);
        SDL_ANDROID_SetScreenKeyboardButtonShown(SDL_ANDROID_SCREENKEYBOARD_BUTTON_1, mainMenu_onScreen_button2);
        SDL_ANDROID_SetScreenKeyboardButtonShown(SDL_ANDROID_SCREENKEYBOARD_BUTTON_2, mainMenu_onScreen_button3);
        SDL_ANDROID_SetScreenKeyboardButtonShown(SDL_ANDROID_SCREENKEYBOARD_BUTTON_3, mainMenu_onScreen_button4);
        SDL_ANDROID_SetScreenKeyboardButtonShown(SDL_ANDROID_SCREENKEYBOARD_BUTTON_4, mainMenu_onScreen_button5);
        SDL_ANDROID_SetScreenKeyboardButtonShown(SDL_ANDROID_SCREENKEYBOARD_BUTTON_5, mainMenu_onScreen_button6);
    }
}
#endif

#if defined(__PSP2__)
static const int vita_shader_order[] = {
    0, 4, 5, 7, 1, 6, 3, 8, 2, 9, 10
};
static const char *vita_shader_labels[] = {
    "None / Raw (2x Integer)",
    "Sharp Bilinear",
    "Sharp Bilinear Simple",
    "CRT Easymode (Scanlines)",
    "LCD 3x (Grid)",
    "FXAA",
    "Advanced AA",
    "Bicubic",
    "Scale2x",
    "xBR 2x",
    "GTU CRT"
};
#define VITA_SHADER_COUNT ((int)(sizeof(vita_shader_order) / sizeof(vita_shader_order[0])))

const char *vita_shader_label(int shader_enum)
{
    for (int i = 0; i < VITA_SHADER_COUNT; i++) {
        if (vita_shader_order[i] == shader_enum)
            return vita_shader_labels[i];
    }
    return vita_shader_labels[0];
}

int vita_shader_cycle(int shader_enum, int direction)
{
    int current = 0;
    for (int i = 0; i < VITA_SHADER_COUNT; i++) {
        if (vita_shader_order[i] == shader_enum) {
            current = i;
            break;
        }
    }
    current = (current + direction + VITA_SHADER_COUNT) % VITA_SHADER_COUNT;
    return vita_shader_order[current];
}

#if defined(__PSP2__)
static void auto_display_supported_mode(int *w, int *h)
{
    int need_w = AUTO_DISPLAY_SURFACE_WIDTH;
    int need_h = AUTO_DISPLAY_SURFACE_HEIGHT;
    SDL_PixelFormat format;
    SDL_Rect **modes;
    int i, best_w = 0, best_h = 0;

    *w = need_w;
    *h = need_h;

    memset(&format, 0, sizeof(format));
    format.BitsPerPixel = 16;
    modes = SDL_ListModes(&format, SDL_HWSURFACE);
    if (modes == NULL || modes == (SDL_Rect **)-1)
        return;

    for (i = 0; modes[i] != NULL; i++) {
        if (modes[i]->w < need_w || modes[i]->h < need_h)
            continue;
        if (best_w == 0 || (long)modes[i]->w * (long)modes[i]->h < (long)best_w * (long)best_h) {
            best_w = modes[i]->w;
            best_h = modes[i]->h;
        }
    }

    if (best_w != 0) {
        *w = best_w;
        *h = best_h;
    }
}
#endif

void vita_get_display_geometry(int *x, int *y, float *sw, float *sh)
{
    if (displayAutoMode) {
        struct AutoDisplayRect rect = autoDisplayRect;
        float fx, fy, fsw, fsh;
        int surface_w = prSDLScreen != NULL ? prSDLScreen->w : AUTO_DISPLAY_SURFACE_WIDTH;
        int surface_h = prSDLScreen != NULL ? prSDLScreen->h : AUTO_DISPLAY_SURFACE_HEIGHT;
        if (!rect.valid)
            auto_display_window(diwfirstword, diwlastword, plffirstline, plflastline,
                minfirstline, maxvpos, max_diwlastword, AUTO_DISPLAY_SURFACE_HEIGHT,
                mainMenu_displayHires, &rect);
        auto_display_fit_surface(&rect, surface_w, surface_h, max_diwlastword);
        auto_display_geometry(&rect, 960, 544, surface_w, surface_h, &fx, &fy, &fsw, &fsh);
        if (x) *x = (int)fx;
        if (y) *y = (int)fy;
        if (sw) *sw = fsw;
        if (sh) *sh = fsh;
        return;
    }

    int preset_variant = presetModeId % 10;
    bool fullscreen_scaling = (preset_variant == 7);
    bool five_four_scaling = (preset_variant == 8);

    float out_sw;
    float out_sh = 544.0f;
    int out_x;
    int out_y = 0;

    if (fullscreen_scaling) {
        out_sw = 960.0f;
        out_x = 0;
    } else if (five_four_scaling) {
        out_sw = (mainMenu_shader != 0) ? 680.0f : 675.0f;
        out_x = (int)((960.0f - out_sw) * 0.5f + 0.5f);
    } else {
        out_sw = (mainMenu_shader != 0) ? 725.0f : 720.0f;
        out_x = (int)((960.0f - out_sw) * 0.5f + 0.5f);
    }

    if (out_sw < 1.0f) out_sw = 1.0f;
    if (out_sw > 960.0f) out_sw = 960.0f;

    int screen_offset_x = mainMenu_screenOffsetX;
    if (screen_offset_x < -128) screen_offset_x = -128;
    if (screen_offset_x > 128) screen_offset_x = 128;
    out_x += screen_offset_x;

    if (out_x < 0) out_x = 0;
    if (out_x > (960 - (int)out_sw)) out_x = 960 - (int)out_sw;

    if (x) *x = out_x;
    if (y) *y = out_y;
    if (sw) *sw = out_sw;
    if (sh) *sh = out_sh;
}

void vita_apply_auto_display_scaling(void)
{
    static int last_enabled = 0;
    static int last_x = 0, last_y = 0;
    static float last_sw = 0.0f, last_sh = 0.0f;
    int x, y;
    float sw, sh;

    if (!displayAutoMode || prSDLScreen == NULL) {
        last_enabled = 0;
        return;
    }

    vita_get_display_geometry(&x, &y, &sw, &sh);
    if (last_enabled && x == last_x && y == last_y && sw == last_sw && sh == last_sh)
        return;

    last_enabled = 1;
    last_x = x;
    last_y = y;
    last_sw = sw;
    last_sh = sh;
    SDL_SetVideoModeScaling(x, y, sw, sh);
}
#endif

void update_display() {
#if defined(__PSP2__)
    write_log("[VITA] update_display: start width=%d height=%d shader=%d menu=%d\n", visibleAreaWidth, mainMenu_displayedLines, mainMenu_shader, displaying_menu);
#endif
    char layersize[20];
    snprintf(layersize, 20, "%dx480", screenWidth);

#ifndef WIN32
#if !defined(__PSP2__) && !defined(__SWITCH__)
    setenv("SDL_OMAP_LAYER_SIZE",layersize,1);
#endif
#endif

    char bordercut[20];
    snprintf(bordercut, 20, "%d,%d,0,0", mainMenu_cutLeft, mainMenu_cutRight);

#ifndef WIN32
#if !defined(__PSP2__) && !defined(__SWITCH__)
    setenv("SDL_OMAP_BORDER_CUT",bordercut,1);
#endif
#endif

#ifdef ANDROIDSDL
    update_onscreen();
#endif

#if defined(__PSP2__) || defined(__SWITCH__)
    int surface_w = visibleAreaWidth;
    int surface_h = mainMenu_displayedLines;
	displaying_menu = 0;

#if defined(__PSP2__)
    if (mainMenu_displayAuto) {
        displayAutoMode = 1;
        auto_display_supported_mode(&surface_w, &surface_h);
        visibleAreaWidth = surface_w;
        mainMenu_displayedLines = AUTO_DISPLAY_SURFACE_HEIGHT;
        if (surface_h < mainMenu_displayedLines)
            mainMenu_displayedLines = surface_h;
        reset_auto_display();
    } else {
        if (displayAutoMode)
            SetPresetMode(presetModeId);
        displayAutoMode = 0;
        surface_w = visibleAreaWidth;
        surface_h = mainMenu_displayedLines;
    }

    bool need_new_surface = (prSDLScreen == NULL || prSDLScreen->w != surface_w || prSDLScreen->h != surface_h);

    if (need_new_surface) {
        if (prSDLScreen != NULL) {
            write_log("[VITA] update_display: releasing previous video surface (%dx%d -> %dx%d)\n",
                      prSDLScreen->w, prSDLScreen->h, surface_w, surface_h);
            vita2d_wait_rendering_done();
            SDL_FreeSurface(prSDLScreen);
            prSDLScreen = NULL;
            write_log("[VITA] update_display: previous video surface released\n");
        }

        vita2d_texture_set_alloc_memblock_type(SCE_KERNEL_MEMBLOCK_TYPE_USER_RW);

        prSDLScreen = SDL_SetVideoMode(surface_w, surface_h, 16, SDL_HWSURFACE | SDL_DOUBLEBUF);
        printf("update_display: SDL_SetVideoMode(%i, %i, 16)\n", surface_w, surface_h);
        write_log("[VITA] update_display: hardware SDL_SetVideoMode returned %p\n", (void *)prSDLScreen);
        if (prSDLScreen == NULL) {
            write_log("[VITA] update_display: retrying hardware surface without doublebuf\n");
            prSDLScreen = SDL_SetVideoMode(surface_w, surface_h, 16, SDL_HWSURFACE);
        }
        if (prSDLScreen == NULL) {
            write_log("[VITA] update_display: retrying software framebuffer\n");
            prSDLScreen = SDL_SetVideoMode(surface_w, surface_h, 16, SDL_SWSURFACE);
            write_log("[VITA] update_display: software SDL_SetVideoMode returned %p\n", (void *)prSDLScreen);
        }
        if (prSDLScreen == NULL && surface_w != 320) {
            write_log("[VITA] update_display: resolution unsupported by SDL driver, falling back to 320x%d\n", surface_h);
            visibleAreaWidth = 320;
            surface_w = 320;
            if (mainMenu_displayedLines > surface_h)
                mainMenu_displayedLines = surface_h;
            mainMenu_displayHires = 0;
            prSDLScreen = SDL_SetVideoMode(320, surface_h, 16, SDL_HWSURFACE | SDL_DOUBLEBUF);
            if (prSDLScreen == NULL)
                prSDLScreen = SDL_SetVideoMode(320, surface_h, 16, SDL_SWSURFACE);
        }
        if (prSDLScreen == NULL && surface_h != 240) {
            write_log("[VITA] update_display: lines unsupported, falling back to 320x240\n");
            surface_w = 320;
            surface_h = 240;
            visibleAreaWidth = 320;
            if (mainMenu_displayedLines > surface_h)
                mainMenu_displayedLines = surface_h;
            mainMenu_displayHires = 0;
            prSDLScreen = SDL_SetVideoMode(320, 240, 16, SDL_HWSURFACE | SDL_DOUBLEBUF);
            if (prSDLScreen == NULL)
                prSDLScreen = SDL_SetVideoMode(320, 240, 16, SDL_SWSURFACE);
        }
        if (prSDLScreen == NULL) {
            write_log("[VITA] update_display: fallback to 320x200\n");
            surface_w = 320;
            surface_h = 200;
            visibleAreaWidth = 320;
            if (mainMenu_displayedLines > surface_h)
                mainMenu_displayedLines = surface_h;
            mainMenu_displayHires = 0;
            prSDLScreen = SDL_SetVideoMode(320, 200, 16, SDL_HWSURFACE | SDL_DOUBLEBUF);
            if (prSDLScreen == NULL)
                prSDLScreen = SDL_SetVideoMode(320, 200, 16, SDL_SWSURFACE);
        }
        if (prSDLScreen == NULL) {
            write_log("[VITA] update_display: SDL_SetVideoMode failed: %s\n", SDL_GetError());
            return;
        }
    } else {
        write_log("[VITA] update_display: reusing existing surface (%dx%d)\n", prSDLScreen->w, prSDLScreen->h);
    }
#else
    prSDLScreen = SDL_SetVideoMode(surface_w, surface_h, 16, SDL_HWSURFACE | SDL_DOUBLEBUF);
    printf("update_display: SDL_SetVideoMode(%i, %i, 16)\n", surface_w, surface_h);
#endif

    float sh;
    float sw;
    int x;
    int y;

#if defined(__PSP2__)
    vita_get_display_geometry(&x, &y, &sw, &sh);
    SDL_SetVideoModeScaling(x, y, sw, sh);
    SDL_SetVideoModeBilinear(mainMenu_shader != 0 ? 1 : 0);
#else
    //is a shader active?
    if (mainMenu_shader != 0)
    {
    	sh = 544;
      if (mainMenu_displayHires)
      	sw = (0.5f*(float)visibleAreaWidth*((float)544/(float)mainMenu_displayedLines));
      else
      	sw = ((float)visibleAreaWidth*((float)544/(float)mainMenu_displayedLines));
    	x = (960 - sw) / 2;
    	y = (544 - sh) / 2;

   	SDL_SetVideoModeScaling(x, y, sw, sh);
   	SDL_SetVideoModeBilinear(1);
    }
    else //otherwise do regular integer 2* scaling without filtering to ensure good picture quality
    {
    	sh = (float) (2 * mainMenu_displayedLines);
    	if (mainMenu_displayHires)
    		sw = (float) (1 * visibleAreaWidth);
    	else
    		sw = (float) (2 * visibleAreaWidth);
    	x = (960 - sw) / 2;
      y = (544 - sh) / 2;
      SDL_SetVideoModeScaling(x, y, sw, sh);
      SDL_SetVideoModeBilinear(0);
	 }
#endif
	 printf("update_display: SDL_SetVideoModeScaling(%i, %i, %i, %i)\n", x, y, (int)sw, (int)sh);

    SDL_SetVideoModeSync(1);
#if defined(__PSP2__)
    write_log("[VITA] update_display: scaling and sync done\n");
#endif

    // clear screen
#if defined(__PSP2__)
    for (int i=0; i<3; i++)
    {
        SDL_FillRect(prSDLScreen,NULL,SDL_MapRGB(prSDLScreen->format, 0, 0, 0));
        SDL_Flip(prSDLScreen);
    }
    write_log("[VITA] update_display: clear done\n");
#else
    for (int i=0; i<2; i++)
	{        SDL_FillRect(prSDLScreen,NULL,SDL_MapRGB(prSDLScreen->format, 0, 0, 0));
        SDL_Flip(prSDLScreen);
    }
#endif

#else
#if defined(PANDORA) && !(defined(WIN32) || defined(AROS))
    prSDLScreen = SDL_SetVideoMode(visibleAreaWidth, mainMenu_displayedLines, 16, SDL_SWSURFACE|SDL_FULLSCREEN|SDL_DOUBLEBUF);
#elif defined(PANDORA) && (defined(WIN32) || defined(AROS))
    prSDLScreen = SDL_SetVideoMode(visibleAreaWidth, mainMenu_displayedLines, 16, SDL_SWSURFACE|SDL_DOUBLEBUF);
#else
    prSDLScreen = SDL_SetVideoMode(visibleAreaWidth, mainMenu_displayedLines, 16, SDL_SWSURFACE|SDL_FULLSCREEN);
#endif
#endif
    SDL_ShowCursor(SDL_DISABLE);

    if (mainMenu_displayHires)
        InitDisplayArea(visibleAreaWidth >> 1);
    else
        InitDisplayArea(visibleAreaWidth);

    init_row_map();
    reset_auto_crop();
    notice_screen_contents_lost();
    notice_new_xcolors();
#if defined(__PSP2__)
    write_log("[VITA] update_display: complete\n");
#endif
}


static bool cpuSpeedChanged = false;

void setCpuSpeed() {
#if !defined(__PSP2__) && !defined(__SWITCH__)
    char speedCmd[128];

    if(mainMenu_cpuSpeed != lastCpuSpeed)
    {
        snprintf((char*)speedCmd, 128, "unset DISPLAY; echo y | sudo -n /usr/pandora/scripts/op_cpuspeed.sh %d", mainMenu_cpuSpeed);
        system(speedCmd);
        lastCpuSpeed = mainMenu_cpuSpeed;
        cpuSpeedChanged = true;
    }
    if(mainMenu_ntsc != ntsc)
    {
        ntsc = mainMenu_ntsc;
        if(ntsc)
            system("sudo /usr/pandora/scripts/op_lcdrate.sh 60");
        else
            system("sudo /usr/pandora/scripts/op_lcdrate.sh 50");
    }
    update_display();
#endif
}


#ifdef PANDORA

void resetCpuSpeed(void) {
    if (cpuSpeedChanged) {
        FILE *f = fopen("/etc/pandora/conf/cpu.conf", "rt");
        if (f) {
            char line[128];
            for (int i = 0; i < 6; ++i) {
                fscanf(f, "%s\n", &line);
                if (strncmp(line, "default:", 8) == 0) {
                    int value = 0;
                    sscanf(line, "default:%d", &value);
                    if (value > 500 && value < 1200) {
                        lastCpuSpeed = value - 10;
                        mainMenu_cpuSpeed = value;
                        setCpuSpeed();
                        printf("CPU speed reset to %d\n", value);
                    }
                }
            }
            fclose(f);
        }
    }
}

#endif
