int kickstart=1;
int oldkickstart=-1;	                          

extern char launchDir[300];

extern "C" int main( int argc, char *argv[] );

  
                                 
   
                
   
                            
                                            
    
#include "sysconfig.h"
#include "sysdeps.h"
#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#ifdef USE_UAE4ALL_VKBD
#include "vkbd.h"
#endif
#include "config.h"
#include "uae.h"
#include "options.h"
#include "thread.h"
#include "debug_uae4all.h"
#include "gensound.h"
#include "events.h"
#include "memory-uae.h"
#include "audio.h"
#include "sound.h"
#include "custom.h"
#include "m68k/m68k_intrf.h"
#include "disk.h"
#include "debug.h"
#include "xwin.h"
#include "joystick.h"
#include "keybuf.h"
#include "gui.h"
#include "zfile.h"
#include "autoconf.h"
#include "osemu.h"
#include "exectasks.h"
#include "bsdsocket.h"
#include "drawing.h"
#include "menu.h" 
#include "menu_config.h"
#include "gp2xutil.h"
               
#include "native2amiga.h"

#ifdef USE_SDL
#include "SDL.h"
extern SDL_Surface *current_screenshot;
#endif
#ifdef GP2X
#include "gp2xutil.h"
#endif

#ifdef USE_UAE4ALL_VKBD
#include "vkbd.h"
#endif

#if defined(__PSP2__)                  
                         
#include <psp2/shellutil.h>
             
#include "psp2_touch.h"
               
#include <psp2/appmgr.h>
#ifdef DEBUG_UAE4ALL
                                                                             
#endif
#endif

#if defined(__SWITCH__)
             
#include "switch_touch.h"
#endif

long int version = 256*65536L*UAEMAJOR + 65536L*UAEMINOR + UAESUBREV;

struct uae_prefs currprefs, changed_prefs; 

int no_gui = 0;
int joystickpresent = 0;
int cloanto_rom = 0;

int64_t g_uae_epoch = 0;

struct gui_info gui_data;

char warning_buffer[256];

#ifdef __PSP2__
static FILE *vita_debug_log_file = NULL;

static void vita_debug_log_init(void)
{
    mkdir("ux0:/data/uae4all", 0777);
    mkdir("ux0:/data/uae4all/roms", 0777);
    mkdir("ux0:/data/uae4all/saves", 0777);
    mkdir("ux0:/data/uae4all/conf", 0777);
    mkdir("ux0:/data/uae4all/kickstarts", 0777);
    mkdir("ux0:/data/uae4all/thumbs", 0777);
    mkdir("ux0:/data/uae4all/tmp", 0777);
    vita_debug_log_file = fopen("ux0:/data/uae4all/crash.log", "w");
    if (vita_debug_log_file) {
        fprintf(vita_debug_log_file, "[VITA] vita_debug_log_init() open\n");
        fflush(vita_debug_log_file);
    }
}

void vita_write_log (const char *fmt, ...)
{
    if (!vita_debug_log_file)
        return;
    va_list ap;
    va_start(ap, fmt);
    vfprintf(vita_debug_log_file, fmt, ap);
    fflush(vita_debug_log_file);
    va_end(ap);
}

static void vita_debug_log_close(void)
{
    if (vita_debug_log_file) {
        fprintf(vita_debug_log_file, "[VITA] vita_debug_log_close()\n");
        fclose(vita_debug_log_file);
        vita_debug_log_file = NULL;
    }
}
#endif

bool resetOnStartingApp = false;
extern char config_load_filename[300];

                                                                   
                                 
                                                                      
                                                          
  
                                                                 
                                           
   


void discard_prefs ()
{
}

void default_prefs ()
{
    produce_sound = 2;
    prefs_gfx_framerate = 0;

    strcpy (prefs_df[0], ROM_PATH_PREFIX "df0.adf");
    strcpy (prefs_df[1], ROM_PATH_PREFIX "df1.adf");

	snprintf(romfile, 256, "%s/kickstarts/%s",launchDir,kickstarts_rom_names[kickstart]);
	if(strlen(extended_rom_names[kickstart]) == 0)
	  snprintf(extfile, 256, "");
	else
	  snprintf(extfile, 256, "%s/kickstarts/%s",launchDir,extended_rom_names[kickstart]);
	FILE *f=fopen (romfile, "r" );
	if(!f){
		strcpy (romfile, "kick.rom");
	}
	else fclose(f);
	
	snprintf(romkeyfile, 256, "%s/kickstarts/%s",launchDir,"rom.key");	

	f=fopen (romkeyfile, "r" );
	if(!f)
	{
		strcpy (romkeyfile, "rom.key");
	}
	else fclose(f);
	
#ifdef ANDROIDSDL
	if (uae4all_init_rom(romfile)==-1)
	{
	  snprintf(romfile, 256, "%s/Android/data/com.cloanto.amigaforever.essentials/files/rom/%s",getenv("SDCARD"),af_kickstarts_rom_names[kickstart]);
	  FILE *f3=fopen (romfile, "r" );
	  if(!f3)
	  {
		  strcpy (romfile, "kick.rom");
	  }
	  else fclose(f3);
	  
	  snprintf(romkeyfile, 256, "%s/Android/data/com.cloanto.amigaforever.essentials/files/rom/%s",getenv("SDCARD"),"rom.key");	
	  FILE *f4=fopen (romkeyfile, "r" );
	  if(!f4)
	  {
		strcpy (romkeyfile, "rom.key");
	  }
	  else fclose(f4);
	}
#endif

	         
    prefs_chipmem_size = 0x00100000;
    prefs_bogomem_size = 0;
	changed_prefs.fastmem_size = 0;
}

int quit_program = 0;

void uae_reset (void)
{
    gui_purge_events();
#ifdef USE_UAE4ALL_VKBD
	vkbd_reset_sticky_keys();                                                       
#endif
    black_screen_now();
    quit_program = 2;
    set_special (SPCFLAG_BRK);
}

void uae_quit (void)
{
    if (quit_program != -1)
	quit_program = -1;
}

void reset_all_systems (void)
{
    init_eventtab ();
    memory_reset ();
                                                                                    
    filesys_reset ();
    reset_hdConf();
    filesys_start_threads ();
}

                                                                         
                                                                          
                                                                         
                                                                           
                                                                       
                                                                         
                                                                             
                                           
   
void do_start_program (void)
{
	quit_program = 2;
	m68k_go (1);
}

void do_leave_program (void)
{
#ifdef USE_SDL
#if defined(__PSP2__) || defined(__SWITCH__)                                                                           
#ifdef USE_UAE4ALL_VKBD
	vkbd_quit();
#endif
#ifdef __PSP2__                  
	                            
	psp2QuitTouch();
#endif
#endif
  if(current_screenshot != NULL)
    SDL_FreeSurface(current_screenshot);
#endif
	     
    graphics_leave ();
    close_joystick ();
    close_sound ();
    zfile_exit ();
#ifdef USE_SDL
    SDL_Quit ();
#endif
    memory_cleanup ();
#ifdef __SWITCH__
    socketExit();
#endif
}

void start_program (void)
{
    do_start_program ();
}

void leave_program (void)
{
    do_leave_program ();
#ifdef __PSP2__
    vita_debug_log_close();
#endif
}

void real_main (int argc, char **argv)
{
#if defined(__PSP2__)                  
    vita_debug_log_init();
	write_log("[VITA] real_main() start argc=%d argv=%p\n", argc, argv);
#ifdef DEBUG_UAE4ALL
	                                                                           
	write_log("[VITA] DEBUG_UAE4ALL active, shell log disabled\n");
#endif
#endif

#if defined(__PSP2__)                  
	write_log("[VITA] init touch\n");
	psp2InitTouch();
	write_log("[VITA] touch init done\n");
#endif

#if defined(__SWITCH__)
    socketInitializeDefault();
#endif

#ifdef USE_SDL
    SDL_Init (SDL_INIT_VIDEO | SDL_INIT_JOYSTICK 
#if !defined(NO_SOUND) && !defined(GP2X)
 			| SDL_INIT_AUDIO
#endif
	);
#endif

#if defined(__PSP2__)
	scePowerSetArmClockFrequency(444);
    write_log("[VITA] set arm clock\n");
    scePowerSetGpuClockFrequency(222);
    write_log("[VITA] set gpu clock\n");
    scePowerSetBusClockFrequency(222);
    write_log("[VITA] set bus clock\n");
    scePowerSetGpuXbarClockFrequency(222);
    write_log("[VITA] set gpu xbar\n");
#endif

                        
  g_uae_epoch = read_processor_time();
  syncbase = 1000000;                

#if defined(__PSP2__)                  
	mkdir("ux0:/data/uae4all", 0777);
	mkdir("ux0:/data/uae4all/roms", 0777);
	mkdir("ux0:/data/uae4all/saves", 0777);
	mkdir("ux0:/data/uae4all/conf", 0777);
	mkdir("ux0:/data/uae4all/kickstarts", 0777);
    mkdir("ux0:/data/uae4all/thumbs", 0777);
    mkdir("ux0:/data/uae4all/tmp", 0777);
	strcpy(launchDir, "ux0:/data/uae4all");
#elif defined(__SWITCH__)
	mkdir("./roms", 0777);
	mkdir("./saves", 0777);
	mkdir("./conf", 0777);
	mkdir("./kickstarts", 0777);
	mkdir("./thumbs", 0777);
	mkdir("./tmp", 0777);
    strcpy(launchDir, ".");
#else
	getcwd(launchDir,250);
#endif
                         
    default_prefs_uae (&currprefs);
    default_prefs();

#ifdef GP2X
    gp2x_init(argc, argv);
#endif
	                                                 
	SetDefaultMenuSettings(1);
                                                       
#if defined(__SWITCH__)
    if (argc == 2) {
        snprintf(config_load_filename, 300, argv[1]);
        resetOnStartingApp = true;
    }
#endif
#if defined(__PSP2__)                  
    char boot_params[1024];
    sceAppMgrGetAppParam(boot_params);
	if (strstr(boot_params,"psgm:play") && strstr(boot_params, "&param=")) {
		snprintf(config_load_filename, 300, strstr(boot_params, "&param=") + 7);
        resetOnStartingApp = true;
    }
#endif
    loadconfig (1);
    write_log("[VITA] loadconfig done\n");
    if (! graphics_setup ()) {
	    write_log("[VITA] graphics_setup failed\n");
	    exit (1);
    }
    write_log("[VITA] graphics_setup done\n");
    rtarea_init ();
    write_log("[VITA] rtarea_init done\n");

    hardfile_install();
    write_log("[VITA] hardfile_install done\n");

    if (! setup_sound ()) {
	write_log ("Sound driver unavailable: Sound output disabled\n");
	produce_sound = 0;
    }
    write_log("[VITA] setup_sound done produce_sound=%d\n", produce_sound);
    init_joystick ();
    write_log("[VITA] init_joystick done\n");

	int err = gui_init ();
	if (err == -1) {
	    write_log ("Failed to initialize the GUI\n");
#ifdef __PSP2__
                                                                              
        vita_debug_log_close();
        return;
#endif
	} else if (err == -2) {
	    write_log ("GUI returned exit status -2\n");
	    exit (0);
	}
    write_log("[VITA] gui_init returned %d\n", err);
    if (sound_available && produce_sound > 1 && ! init_audio ()) {
	write_log ("Sound driver unavailable: Sound output disabled\n");
	produce_sound = 0;
    }
    write_log("[VITA] init_audio done produce_sound=%d sound_available=%d\n", produce_sound, sound_available);
                                                                  
    rtarea_setup ();

    keybuf_init ();                                    

#ifdef USE_AUTOCONFIG
    expansion_init ();
#endif

    memory_init ();

    filesys_install (); 
    native2amiga_install ();

    custom_init ();                                  
    DISK_init ();

    m68k_init(0);

    gui_update ();

#ifdef GP2X
    switch_to_hw_sdl(1);
#endif
	{
		start_program ();
	}
    leave_program ();
}

#ifndef NO_MAIN_IN_MAIN_C
int main (int argc, char *argv[])
{
    real_main (argc, argv);
    return 0;
}


void default_prefs_uae (struct uae_prefs *p)
{
    p->chipset_mask = CSMASK_ECS_AGNUS;
    
    p->cpu_level = M68000;
    
    p->fastmem_size = 0x00000000;
#if !( defined(PANDORA) || defined(ANDROIDSDL) )
    p->z3fastmem_size = 0x00000000;
    p->gfxmem_size = 0x00000000;
#endif

    p->mountinfo = alloc_mountinfo ();
}

void discard_prefs_uae (struct uae_prefs *p)
{
    free_mountinfo (p->mountinfo);
}
#endif
