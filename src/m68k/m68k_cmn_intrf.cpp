#define NO_SHORT_EVENTS
#define PROTECT_INFINITE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sysconfig.h"
#include "sysdeps.h"
#include "config.h"
#include "debug_uae4all.h"
#include "uae.h"
#include "options.h"
#include "memory-uae.h"
#include "custom.h"
#include "autoconf.h"
#include "ersatz.h"
#include "debug.h"
#include "gui.h"
#include "savestate.h"
#include "blitter.h"
#include "events.h"
#include "sound.h"

#include "m68k/debug_m68k.h"

int prefs_cpu_model;

unsigned mispcflags=0;

int in_m68k_go = 0;

static int do_specialties (int cycles)
{
    if (mispcflags & SPCFLAG_COPPER)
    {
        do_copper ();
    }

                     
    while ((mispcflags & SPCFLAG_BLTNASTY) && cycles > 0) {
        int c = blitnasty();
        if (!c) {
            cycles -= 2 * CYCLE_UNIT;
            if (cycles < CYCLE_UNIT)
                cycles = 0;
            c = 1;
        }

        do_cycles(c * CYCLE_UNIT);
        if (mispcflags & SPCFLAG_COPPER)
      	{
            do_copper ();
      	}
    }

    if (mispcflags & SPCFLAG_BRK) {
        unset_special (SPCFLAG_BRK);
        return 1;
    }
    return 0;
}

static void uae4all_reset(void)
{
    int i;

    int new_table = (prefs_cpu_model != changed_prefs.cpu_level) ? 1 : 0;
    prefs_cpu_model = changed_prefs.cpu_level;
    if(new_table)
      m68k_init(1);                                           
    m68k_reset();
    for(i=1;i<8;i++)
    	M68KCONTEXT.interrupts[i]=0x18+i;
    M68KCONTEXT.interrupts[0]=0;
    mispcflags=0;
    _68k_areg(7) = get_long (0x00f80000);
    _68k_setpc(get_long (0x00f80004));
                                                         
    mispcflags=0;
}

static void m68k_run (void)
{
	unsigned long cycles;
	unsigned int cycles_actual = M68KCONTEXT.cycles_counter;
	unsigned long last_currcycle = currcycle;
	
	for (;;) 
	{
	                                                                             
	                                              
		cycles = nextevent - last_currcycle;
		cycles >>= 8;

		last_currcycle = currcycle;
		m68k_emulate(cycles);
      
		cycles = (M68KCONTEXT.cycles_counter - cycles_actual) << 8;
                                                                                             
    if(cycles > currcycle - last_currcycle)
    {
      cycles = cycles - (currcycle - last_currcycle);
  		do_cycles(cycles);
      last_currcycle = currcycle;

  		if (mispcflags)
  			if (do_specialties (cycles))                                                     
  				return;
    }
    else
      last_currcycle = currcycle;
 
		unsigned cuentalo = 0;
		while((nextevent - currcycle) <= 2048)
		{
			cycles = 2048;                           

			do_cycles(cycles);
			if (mispcflags)
				if (do_specialties (cycles))                                                     
					return;

			cuentalo++;
			if (cuentalo>1024)
			{
				quit_program=2;
				return;
			}
		}
		
		cycles_actual = M68KCONTEXT.cycles_counter;
	}
}


void m68k_go (int may_quit)
{
	gui_purge_events();

	if (in_m68k_go || !may_quit) 
	{
   	return;
	}

	in_m68k_go++;
	quit_program = 2;
	for (;;) 
	{
		if (quit_program > 0) 
		{
			if (quit_program == 1)
         	break;
         quit_program = 0;
			if (savestate_state == STATE_RESTORE)
	    	{
				restore_state (savestate_filename);
				mispcflags = 0;
	    	}
			reset_all_systems ();
			customreset ();
			sound_default_evtime ();
                                                                     
         handle_active_events ();
         if (mispcflags)
         	do_specialties (0);
      }

		if(!savestate_state)
	   	uae4all_reset();
		savestate_restore_finish();
      m68k_run();
	}
	in_m68k_go--;
}

                           

#define CPUMODE_HALT 1

uae_u8 *restore_cpu (uae_u8 *src)
{
    int i,model,flags;
    uae_u32 l;

    prefs_cpu_model = restore_u32();
              
    flags = restore_u32();
    for (i = 0; i < 8; i++)
	    _68k_dreg(i)=restore_u32 ();
    for (i = 0; i < 8; i++)
	    _68k_areg(i)=restore_u32 ();
    _68k_setpc(restore_u32 ());
                                                                        
                                                                        
                   
                              restore_u32 ();
                                                    
    
                                                                            
    _68k_mspreg = 0;
    _68k_uspreg = restore_u32 ();
                         restore_u32 ();
    _68k_sreg = restore_u16 ();
    l = restore_u32();
    if (l & CPUMODE_HALT) {
	M68KCONTEXT.execinfo|=0x0080;
	mispcflags=SPCFLAG_STOP;
    } else {
	M68KCONTEXT.execinfo&=~0x0080;
	mispcflags=0;
    }

    return src;
}

uae_u8 *save_cpu (int *len)
{
    uae_u8 *dstbak,*dst;
    int model,i;

    dstbak = dst = (uae_u8 *)malloc(4+4+15*4+4+4+4+4+2+4+4+4+4+4+4+4);
    save_u32 (prefs_cpu_model);					           
    save_u32 (1);                                                   
    for(i = 0;i < 8; i++)
	    save_u32 (_68k_dreg(i));
    for(i = 0;i < 8; i++)
	    save_u32 (_68k_areg(i));
    save_u32 (m68k_get_pc());				        
    save_u32 (0);                                        
    
                                                                            
                             
    save_u32 (_68k_uspreg);
    save_u32 (_68k_areg(7));
    save_u16 (_68k_sreg);				            
    save_u32 (M68KCONTEXT.execinfo&0x0080 ? CPUMODE_HALT : 0);	           
    *len = dst - dstbak;
    return dstbak;
}
