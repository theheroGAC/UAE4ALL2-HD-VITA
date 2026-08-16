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

#if defined(__PSP2__)                  
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
#endif                  
#endif

#ifdef __SWITCH__
#include <switch.h>
#endif

#if defined(__PSP2__)                  
                         
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
    
#if defined(__PSP2__)                  
                      
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
#if defined(__PSP2__)
                                                                          
                                                                             
                                                                          
                                            
    if (prSDLScreen != NULL) {
        write_log("[VITA] update_display: releasing previous video surface\n");
        for (int i = 0; i < 4; i++) {
            SDL_FillRect(prSDLScreen, NULL, SDL_MapRGB(prSDLScreen->format, 0, 0, 0));
            SDL_Flip(prSDLScreen);
        }
        vita2d_wait_rendering_done();
        if (prSDLScreen->hwdata != NULL) {
            private_hwdata *old_hw = (private_hwdata *)prSDLScreen->hwdata;
            if (old_hw->texture != NULL) {
                vita2d_free_texture(old_hw->texture);
                old_hw->texture = NULL;
            }
            SDL_free(prSDLScreen->hwdata);
            prSDLScreen->hwdata = NULL;
            prSDLScreen->pixels = NULL;
        }
        SDL_FreeSurface(prSDLScreen);
        prSDLScreen = NULL;
        write_log("[VITA] update_display: previous video surface released\n");
    }

                                                                           
                                                                            
                                      
    vita2d_texture_set_alloc_memblock_type(SCE_KERNEL_MEMBLOCK_TYPE_USER_RW);
#endif
	displaying_menu = 0;

    prSDLScreen = SDL_SetVideoMode(visibleAreaWidth, mainMenu_displayedLines, 16, SDL_HWSURFACE | SDL_DOUBLEBUF);
    printf("update_display: SDL_SetVideoMode(%i, %i, 16)\n", visibleAreaWidth, mainMenu_displayedLines);
#if defined(__PSP2__)
    write_log("[VITA] update_display: hardware SDL_SetVideoMode returned %p\n", (void *)prSDLScreen);
    if (prSDLScreen == NULL) {
                                                                         
                                                                             
        write_log("[VITA] update_display: retrying software framebuffer\n");
        prSDLScreen = SDL_SetVideoMode(visibleAreaWidth, mainMenu_displayedLines, 16, SDL_SWSURFACE | SDL_DOUBLEBUF);
        write_log("[VITA] update_display: software SDL_SetVideoMode returned %p\n", (void *)prSDLScreen);
    }
#endif
    if (prSDLScreen == NULL) {
#if defined(__PSP2__)
                                                                           
                                                                            
                                                                          
                                              
        if (visibleAreaWidth == 320 && mainMenu_displayedLines != 200) {
            write_log("[VITA] update_display: retrying supported 320x200 mode\n");
            mainMenu_displayedLines = 200;
            prSDLScreen = SDL_SetVideoMode(320, 200, 16, SDL_HWSURFACE | SDL_DOUBLEBUF);
            if (prSDLScreen == NULL) {
                prSDLScreen = SDL_SetVideoMode(320, 200, 16, SDL_SWSURFACE | SDL_DOUBLEBUF);
            }
            write_log("[VITA] update_display: 320x200 framebuffer returned %p\n", (void *)prSDLScreen);
        }
#endif
    }
    if (prSDLScreen == NULL) {
#if defined(__PSP2__)
        write_log("[VITA] update_display: SDL_SetVideoMode failed: %s\n", SDL_GetError());
#endif
        return;
    }

    float sh;
    float sw;
    int x;
    int y;

#if defined(__PSP2__)
      
                                                                      
                                                                            
                                                                          
                                                                             
                                                                            
                     
      
                                                                           
                                                                           
                                                                         
                                                 
       
    int preset_variant = presetModeId % 10;
    bool fullscreen_scaling = (preset_variant == 7);
    if (fullscreen_scaling) {
        x = 0;
        y = 0;
        sw = 960.0f;
        sh = 544.0f;
    } else if (mainMenu_shader != 0) {
        sh = 544.0f;
        sw = sh * (4.0f / 3.0f);
        x = (int)((960.0f - sw) * 0.5f + 0.5f);
        y = 0;
    } else {
                                                                         
                                                                         
        sw = 720.0f;
        sh = 540.0f;
        x = (960 - (int)sw) / 2;
        y = (544 - (int)sh) / 2;
    }
    SDL_SetVideoModeScaling(x, y, sw, sh);
    SDL_SetVideoModeBilinear(mainMenu_shader != 0 ? 1 : 0);
    write_log("[VITA] update_display: preset=%d aspect=%s dst=%dx%d+%d+%d\n",
        presetModeId, fullscreen_scaling ? "fullscreen" : "4:3",
        (int)sw, (int)sh, x, y);
#else
                         
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
    else                                                                                           
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

                   
    for (int i=0; i<2; i++)
	{        SDL_FillRect(prSDLScreen,NULL,SDL_MapRGB(prSDLScreen->format, 0, 0, 0));
        SDL_Flip(prSDLScreen);
    }
#if defined(__PSP2__)
    write_log("[VITA] update_display: clear done\n");
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
