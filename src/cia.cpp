   
                                 
   
                    
   
                                                     
                                                           
    

#include "sysconfig.h"
#include "sysdeps.h"
#include <assert.h>

#include "debug_uae4all.h"

#include "config.h"
#include "uae.h"
#include "options.h"
#include "thread.h"
#include "events.h"
#include "memory-uae.h"
#include "custom.h"
#include "cia.h"
#include "serial.h"
#include "disk.h"
#include "xwin.h"
#include "keybuf.h"
#include "gui.h"
#include "savestate.h"


#define DIV10 (5*CYCLE_UNIT)                             

                     
#define RTC_D_ADJ      8
#define RTC_D_IRQ      4
#define RTC_D_BUSY     2
#define RTC_D_HOLD     1
#define RTC_E_t1       8
#define RTC_E_t0       4
#define RTC_E_INTR     2
#define RTC_E_MASK     1
#define RTC_F_TEST     8
#define RTC_F_24_12    4
#define RTC_F_STOP     2
#define RTC_F_RSET     1

static unsigned int clock_control_d = RTC_D_ADJ + RTC_D_HOLD;
static unsigned int clock_control_e = 0;
static unsigned int clock_control_f = RTC_F_24_12;

static unsigned int ciaaicr, ciaaimask, ciabicr, ciabimask;
static unsigned int ciaacra, ciaacrb, ciabcra, ciabcrb;

                                
static unsigned long ciaata, ciaatb, ciabta, ciabtb;
                                       
static unsigned long ciaata_passed, ciaatb_passed, ciabta_passed, ciabtb_passed;

static unsigned long ciaatod, ciabtod, ciaatol, ciabtol, ciaaalarm, ciabalarm;
static int ciaatlatch, ciabtlatch;

static int oldovl;

static unsigned int ciabpra;

unsigned int gui_ledstate;

static unsigned long ciaala, ciaalb, ciabla, ciablb;
static int ciaatodon, ciabtodon;
static unsigned int ciaapra, ciaaprb, ciaadra, ciaadrb, ciaasdr, ciaasdr_cnt;
static unsigned int ciabprb, ciabdra, ciabdrb, ciabsdr;
static int div10;
static int kbstate, kback, ciaasdr_unread = 0;


static __inline__ void setclr (unsigned int *_GCCRES_ p, unsigned int val)
{
    if (val & 0x80) {
	*p |= val & 0x7F;
    } else {
	*p &= ~val;
    }
}

static void RethinkICRA (void)
{
    if (ciaaimask & ciaaicr) {
	ciaaicr |= 0x80;
	INTREQ_0 (0x8008);
    } else {
	ciaaicr &= 0x7F;
    }
}

static void RethinkICRB (void)
{
    if (ciabimask & ciabicr) {
	ciabicr |= 0x80;
	INTREQ_0 (0xA000);
    } else {
	ciabicr &= 0x7F;
    }
}

void rethink_cias (void)
{
    RethinkICRA ();
    RethinkICRB ();
}

                                                                            
                                   

static void compute_passed_time (void)
{
    unsigned long int ccount = (get_cycles () - eventtab[ev_cia].oldcycles + div10);
    unsigned long int ciaclocks = ccount / DIV10;

    ciaata_passed = ciaatb_passed = ciabta_passed = ciabtb_passed = 0;

                      
    if ((ciaacra & 0x21) == 0x01) {
	assert ((ciaata+1) >= ciaclocks);
	ciaata_passed = ciaclocks;
    }
    if ((ciaacrb & 0x61) == 0x01) {
	assert ((ciaatb+1) >= ciaclocks);
	ciaatb_passed = ciaclocks;
    }

                      
    if ((ciabcra & 0x21) == 0x01) {
	assert ((ciabta+1) >= ciaclocks);
	ciabta_passed = ciaclocks;
    }
    if ((ciabcrb & 0x61) == 0x01) {
	assert ((ciabtb+1) >= ciaclocks);
	ciabtb_passed = ciaclocks;
    }
}

                                                                           
                                                                              
                         

                 
static void CIA_update (void)
{
   unsigned long int ccount = (get_cycles () - eventtab[ev_cia].oldcycles + div10);
   unsigned long int ciaclocks = ccount / DIV10;

   int aovfla = 0, aovflb = 0, asp = 0, bovfla = 0, bovflb = 0, bsp = 0;

   div10 = ccount % DIV10;

                     
   if ((ciaacra & 0x21) == 0x01) {
      assert ((ciaata + 1) >= ciaclocks);
      if ((ciaata + 1) == ciaclocks) {
         if ((ciaacra & 0x48) == 0x40 && ciaasdr_cnt > 0 && --ciaasdr_cnt == 0)
            asp = 1;
         aovfla = 1;
         if ((ciaacrb & 0x61) == 0x41 || (ciaacrb & 0x61) == 0x61) {
            if (ciaatb-- == 0)
               aovflb = 1;
         }
      }
      ciaata -= ciaclocks;
   }
   if ((ciaacrb & 0x61) == 0x01) {
      assert ((ciaatb + 1) >= ciaclocks);
      if ((ciaatb + 1) == ciaclocks) aovflb = 1;
      ciaatb -= ciaclocks;
   }

                     
   if ((ciabcra & 0x21) == 0x01) {
      assert ((ciabta + 1) >= ciaclocks);
      if ((ciabta + 1) == ciaclocks) {
         if ((ciabcra & 0x48) == 0x40                                             )
            bsp = 1;
         bovfla = 1;
         if ((ciabcrb & 0x61) == 0x41 || (ciabcrb & 0x61) == 0x61) {
            if (ciabtb-- == 0)
               bovflb = 1;
         }
      }
      ciabta -= ciaclocks;
   }
   if ((ciabcrb & 0x61) == 0x01) {
      assert ((ciabtb + 1) >= ciaclocks);
      if ((ciabtb + 1) == ciaclocks) bovflb = 1;
      ciabtb -= ciaclocks;
   }

   if (aovfla) {
      ciaaicr |= 1; RethinkICRA ();
      ciaata = ciaala;
      if (ciaacra & 0x8) ciaacra &= ~1;
   }
   if (aovflb) {
      ciaaicr |= 2; RethinkICRA ();
      ciaatb = ciaalb;
      if (ciaacrb & 0x8) ciaacrb &= ~1;
   }
   if (asp) {
      ciaaicr |= 8; RethinkICRA ();
   }
   if (bovfla) {
      ciabicr |= 1; RethinkICRB ();
      ciabta = ciabla;
      if (ciabcra & 0x8) ciabcra &= ~1;
   }
   if (bovflb) {
      ciabicr |= 2; RethinkICRB ();
      ciabtb = ciablb;
      if (ciabcrb & 0x8) ciabcrb &= ~1;
   }
   if (bsp) {
      ciabicr |= 8; RethinkICRB ();
   }
}

                                                                         

static void CIA_calctimers (void)
{
    long int ciaatimea = -1, ciaatimeb = -1, ciabtimea = -1, ciabtimeb = -1;

    eventtab[ev_cia].oldcycles = get_cycles ();
    if ((ciaacra & 0x21) == 0x01) {
	ciaatimea = (DIV10 - div10) + DIV10 * ciaata;
    }
    if ((ciaacrb & 0x61) == 0x01) {
	ciaatimeb = (DIV10 - div10) + DIV10 * ciaatb;
    }

    if ((ciabcra & 0x21) == 0x01) {
	ciabtimea = (DIV10 - div10) + DIV10 * ciabta;
    }
    if ((ciabcrb & 0x61) == 0x01) {
	ciabtimeb = (DIV10 - div10) + DIV10 * ciabtb;
    }
    eventtab[ev_cia].active = (ciaatimea != -1 || ciaatimeb != -1
			       || ciabtimea != -1 || ciabtimeb != -1);
    if (eventtab[ev_cia].active) {
	unsigned long int ciatime = ~0L;
	if (ciaatimea != -1) ciatime = ciaatimea;
	if (ciaatimeb != -1 && ciaatimeb < ciatime) ciatime = ciaatimeb;
	if (ciabtimea != -1 && ciabtimea < ciatime) ciatime = ciabtimea;
	if (ciabtimeb != -1 && ciabtimeb < ciatime) ciatime = ciabtimeb;
	eventtab[ev_cia].evtime = ciatime + get_cycles ();
    }
    events_schedule();
}

void CIA_handler (void)
{
    CIA_update ();
    CIA_calctimers ();
}

void cia_diskindex (void)
{
    ciabicr |= 0x10;
    RethinkICRB();
}

static int checkalarm (unsigned long tod, unsigned long alarm, int inc)
{
    if (tod == alarm)
	return 1;
    if (!inc)
	return 0;
                                    
                                                        
                                                                          
       
    if (tod & 0x000fff)
	return 0;
    if (((tod - 1) & 0xfff000) == alarm)
	return 1;
    return 0;
}

static __inline__ void ciab_checkalarm (int inc)
{
    if (checkalarm (ciabtod, ciabalarm, inc)) {
	ciabicr |= 4;
	RethinkICRB ();
    }
}

static __inline__ void ciaa_checkalarm (int inc)
{
    if (checkalarm (ciaatod, ciaaalarm, inc)) {
	ciaaicr |= 4;
	RethinkICRA ();
    }
}


void CIA_hsync_handler (void)
{
    static unsigned int keytime = 0, sleepyhead = 0;

    if (ciabtodon)
    {
	    ciabtod++;
      ciabtod &= 0xFFFFFF;
	    ciab_checkalarm (1);
	  }
	  
    if (ciabtod == ciabalarm) {
	ciabicr |= 4; RethinkICRB();
    }

    if (keys_available() && kback && (ciaacra & 0x40) == 0 && (++keytime & 15) == 0) {
	  
                                                                   
                                                                   
                                                                 
                                                                  
                                                                   
                                                                  
                                                                 
                                                                      
                                                                    
                                                                   
    
	if (ciaasdr_unread == 2)
	    ciaasdr_unread = 0;
	else if (ciaasdr_unread == 0) {
	    switch (kbstate) {
	     case 0:
		ciaasdr = (uae_s8)~0xFB;                                 
		kbstate++;
		break;
	     case 1:
		kbstate++;
		ciaasdr = (uae_s8)~0xFD;
		break;
	     case 2:
		ciaasdr = ~get_next_key();
		ciaasdr_unread = 1;                                                
		break;
	    }
	    ciaaicr |= 8;
	    RethinkICRA();
	    sleepyhead = 0;
	} else if (!(++sleepyhead & 15))
	    ciaasdr_unread = 0;                                                                      
    }
}

void CIA_vsync_handler ()
{
    if (ciaatodon)
    {
    	ciaatod++;
      ciaatod &= 0xFFFFFF;
    	ciaa_checkalarm (1);
    }
}


#define DIR_LEFT_BIT 9
#define DIR_RIGHT_BIT 1
#define DIR_UP_BIT 8
#define DIR_DOWN_BIT 0
#define DIR_LEFT (1 << DIR_LEFT_BIT)
#define DIR_RIGHT (1 << DIR_RIGHT_BIT)
#define DIR_UP (1 << DIR_UP_BIT)
#define DIR_DOWN (1 << DIR_DOWN_BIT)

static uae_u8 parconvert (uae_u8 v, int jd, int shift)
{
	if (jd & DIR_UP)
		v &= ~(1 << shift);
	if (jd & DIR_DOWN)
		v &= ~(2 << shift);
	if (jd & DIR_LEFT)
		v &= ~(4 << shift);
	if (jd & DIR_RIGHT)
		v &= ~(8 << shift);
	return v;
}

                                                                      
static uae_u8 handle_parport_joystick (int port, uae_u8 pra, uae_u8 dra)
{
	uae_u8 v;
    bool parport_joystick_enabled = true;
	switch (port)
	{
	case 0:
		v = (pra & dra) | (dra ^ 0xff);
		if (parport_joystick_enabled) {
			v = parconvert (v, joy2dir, 0);
			v = parconvert (v, joy3dir, 4);
		}
		return v;
	case 1:
		v = ((pra & dra) | (dra ^ 0xff)) & 0x7;
		if (parport_joystick_enabled) {
			if (joy2button & 0x01)
				v &= ~4;
			if (joy3button & 0x01)
				v &= ~1;
			if (joy2button & 0x02 || joy3button & 0x02)
				v &= ~2;            
		}
		return v;
	default:
		abort ();
		return 0;
	}
}

static uae_u8 ReadCIAA (unsigned int addr)
{
    unsigned int tmp;

    compute_passed_time ();

    switch (addr & 0xf) {
    case 0:
	tmp = (DISK_status() & 0x3C);
	if (!buttonstate[0])
	    tmp |= 0x40;
	if (!(joy1button & 1))
	    tmp |= 0x80;
	tmp |= (ciaapra | (ciaadra ^ 3)) & 0x03;
	if (ciaadra & 0x40)
	    tmp = (tmp & ~0x40) | (ciaapra & 0x40);
	if (ciaadra & 0x80)
	    tmp = (tmp & ~0x80) | (ciaapra & 0x80);
	return tmp;
    case 1:
    tmp = handle_parport_joystick (0, ciaaprb, ciaadrb);
    if (ciaacrb & 2) {
        int pb7 = 0;
        if (ciaacrb & 4)
            pb7 = ciaacrb & 1;
        tmp &= ~0x80;
        tmp |= pb7 ? 0x80 : 00;
    }
    if (ciaacra & 2) {
        int pb6 = 0;
        if (ciaacra & 4)
            pb6 = ciaacra & 1;
        tmp &= ~0x40;
        tmp |= pb6 ? 0x40 : 00;
    }
    return tmp;
    case 2:
	return ciaadra;
    case 3:
	return ciaadrb;
    case 4:
	return (ciaata - ciaata_passed) & 0xff;
    case 5:
	return (ciaata - ciaata_passed) >> 8;
    case 6:
	return (ciaatb - ciaatb_passed) & 0xff;
    case 7:
	return (ciaatb - ciaatb_passed) >> 8;
    case 8:
	if (ciaatlatch) {
	    ciaatlatch = 0;
	    return ciaatol & 0xff;
	} else
	    return ciaatod & 0xff;
    case 9:
	if (ciaatlatch)
	    return (ciaatol >> 8) & 0xff;
	else
	    return (ciaatod >> 8) & 0xff;
    case 10:
	ciaatlatch = 1;
	ciaatol = ciaatod;                                       
	return (ciaatol >> 16) & 0xff;
    case 12:
	if (ciaasdr_unread == 1) 
	  ciaasdr_unread = 2;
	return ciaasdr;
    case 13:
	tmp = ciaaicr; ciaaicr = 0;
	RethinkICRA();
	return tmp;
    case 14:
	return ciaacra;
    case 15:
	return ciaacrb;
    }
    return 0;
}

static uae_u8 ReadCIAB (unsigned int addr)
{
    unsigned int tmp;

    compute_passed_time ();

    switch (addr & 0xf) {
    case 0:
    tmp = 0;
    tmp |= handle_parport_joystick (1, ciabpra, ciabdra);
    return tmp;
    case 1:
	return ciabprb;
    case 2:
	return ciabdra;
    case 3:
	return ciabdrb;
    case 4:
	return (ciabta - ciabta_passed) & 0xff;
    case 5:
	return (ciabta - ciabta_passed) >> 8;
    case 6:
	return (ciabtb - ciabtb_passed) & 0xff;
    case 7:
	return (ciabtb - ciabtb_passed) >> 8;
    case 8:
	if (ciabtlatch) {
	    ciabtlatch = 0;
	    return ciabtol & 0xff;
	} else
	    return ciabtod & 0xff;
    case 9:
	if (ciabtlatch)
	    return (ciabtol >> 8) & 0xff;
	else
	    return (ciabtod >> 8) & 0xff;
    case 10:
	ciabtlatch = 1;
	ciabtol = ciabtod;
	return (ciabtol >> 16) & 0xff;
    case 12:
	return ciabsdr;
    case 13:
	tmp = ciabicr; ciabicr = 0; RethinkICRB();
	return tmp;
    case 14:
	return ciabcra;
    case 15:
	return ciabcrb;
    }
    return 0;
}

static void WriteCIAA (uae_u16 addr,uae_u8 val)
{
    int oldled;
	if ((currprefs.chipset_mask & CSMASK_AGA) && oldovl) {
		int i = (allocated_chipmem>>16) > 32 ? allocated_chipmem >> 16 : 32;
		map_banks (&chipmem_bank, 0, i, allocated_chipmem);
		oldovl = 0;
	}

    switch (addr & 0xf) {
    case 0:
	if (!(currprefs.chipset_mask & CSMASK_AGA)) {
		oldovl = ciaapra & 1;
	}
	oldled = ciaapra & 2;
	ciaapra = (ciaapra & ~0x3) | (val & 0x3);
	gui_ledstate &= ~1;
	gui_ledstate |= ((~ciaapra & 2) >> 1);
	gui_data.powerled = ((~ciaapra & 2) >> 1);

	if (!(currprefs.chipset_mask & CSMASK_AGA)) {
		if ((ciaapra & 1) != oldovl) {
		    int i = (allocated_chipmem>>16) > 32 ? allocated_chipmem >> 16 : 32;
		    
		    if (oldovl || ersatzkickfile) {
			map_banks (&chipmem_bank, 0, i, allocated_chipmem);
		    } else {
			                                                   
			map_banks (&kickmem_bank, 0, i, 0x80000);
		    }
		}
	}
	break;
    case 1:
	ciaaprb = val;
	ciaaicr |= 0x10;
	break;
    case 2:
	ciaadra = val;
	break;
    case 3:
	ciaadrb = val;
	break;
    case 4:
	CIA_update ();
	ciaala = (ciaala & 0xff00) | val;
	CIA_calctimers ();
	break;
    case 5:
	CIA_update ();
	ciaala = (ciaala & 0xff) | (val << 8);
	if ((ciaacra & 1) == 0)
	    ciaata = ciaala;
	if (ciaacra & 8) {
	    ciaata = ciaala;
	    ciaacra |= 1;
	}
	CIA_calctimers ();
	break;
    case 6:
	CIA_update ();
	ciaalb = (ciaalb & 0xff00) | val;
	CIA_calctimers ();
	break;
    case 7:
	CIA_update ();
	ciaalb = (ciaalb & 0xff) | (val << 8);
	if ((ciaacrb & 1) == 0)
	    ciaatb = ciaalb;
	if (ciaacrb & 8) {
	    ciaatb = ciaalb;
	    ciaacrb |= 1;
	}
	CIA_calctimers ();
	break;
    case 8:
	if (ciaacrb & 0x80) {
	    ciaaalarm = (ciaaalarm & ~0xff) | val;
	} else {
	    ciaatod = (ciaatod & ~0xff) | val;
	    ciaatodon = 1;
	}
	break;
    case 9:
	if (ciaacrb & 0x80) {
	    ciaaalarm = (ciaaalarm & ~0xff00) | (val << 8);
	} else {
	    ciaatod = (ciaatod & ~0xff00) | (val << 8);
	    ciaatodon = 0;
	}
	break;
    case 10:
	if (ciaacrb & 0x80) {
	    ciaaalarm = (ciaaalarm & ~0xff0000) | (val << 16);
	} else {
	    ciaatod = (ciaatod & ~0xff0000) | (val << 16);
	    ciaatodon = 0;
	}
	break;
    case 12:
    CIA_update ();
    ciaasdr = val;
    if (ciaacra & 0x40) {
        kback = 1;
    } else {
        ciaasdr_cnt = 0;
    }
    if ((ciaacra & 0x41) == 0x41)
      ciaasdr_cnt = 8 * 2;
    CIA_calctimers ();
	break;
    case 13:
	setclr(&ciaaimask,val);
	break;
    case 14:
	CIA_update ();
	ciaacra = val;
	if (ciaacra & 0x10) {
	    ciaacra &= ~0x10;
	    ciaata = ciaala;
	}
	if (ciaacra & 0x40)
	    kback = 1;
	CIA_calctimers ();
	break;
    case 15:
	CIA_update ();
	ciaacrb = val;
	if (ciaacrb & 0x10) {
	    ciaacrb &= ~0x10;
	    ciaatb = ciaalb;
	}
	CIA_calctimers ();
	break;
    }
}

static void WriteCIAB (uae_u16 addr,uae_u8 val)
{
    int oldval;
    switch (addr & 0xf) {
    case 0:
	    ciabpra  = val;
	break;
    case 1:
	ciabprb = val; DISK_select(val); break;
    case 2:
	ciabdra = val; break;
    case 3:
	ciabdrb = val; break;
    case 4:
	CIA_update ();
	ciabla = (ciabla & 0xff00) | val;
	CIA_calctimers ();
	break;
    case 5:
	CIA_update ();
	ciabla = (ciabla & 0xff) | (val << 8);
	if ((ciabcra & 1) == 0)
	    ciabta = ciabla;
	if (ciabcra & 8) {
	    ciabta = ciabla;
	    ciabcra |= 1;
	}
	CIA_calctimers ();
	break;
    case 6:
	CIA_update ();
	ciablb = (ciablb & 0xff00) | val;
	CIA_calctimers ();
	break;
    case 7:
	CIA_update ();
	ciablb = (ciablb & 0xff) | (val << 8);
	if ((ciabcrb & 1) == 0)
	    ciabtb = ciablb;
	if (ciabcrb & 8) {
	    ciabtb = ciablb;
	    ciabcrb |= 1;
	}
	CIA_calctimers ();
	break;
    case 8:
	if (ciabcrb & 0x80) {
	    ciabalarm = (ciabalarm & ~0xff) | val;
	} else {
	    ciabtod = (ciabtod & ~0xff) | val;
	    ciabtodon = 1;
	}
	break;
    case 9:
	if (ciabcrb & 0x80) {
	    ciabalarm = (ciabalarm & ~0xff00) | (val << 8);
	} else {
	    ciabtod = (ciabtod & ~0xff00) | (val << 8);
	    ciabtodon = 0;
	}
	break;
    case 10:
	if (ciabcrb & 0x80) {
	    ciabalarm = (ciabalarm & ~0xff0000) | (val << 16);
	} else {
	    ciabtod = (ciabtod & ~0xff0000) | (val << 16);
	    ciabtodon = 0;
	}
	break;
    case 12:
   	ciabsdr = val;
   	break;
    case 13:
	setclr(&ciabimask,val);
	break;
    case 14:
	CIA_update ();
	ciabcra = val;
	if (ciabcra & 0x10) {
	    ciabcra &= ~0x10;
	    ciabta = ciabla;
	}
	CIA_calctimers ();
	break;
    case 15:
	CIA_update ();
	ciabcrb = val;
	if (ciabcrb & 0x10) {
	    ciabcrb &= ~0x10;
	    ciabtb = ciablb;
	}
	CIA_calctimers ();
	break;
    }
}

void CIA_reset (void)
{
    kback = 1;
    kbstate = 0;
    ciaasdr_unread = 0;
    ciaasdr_cnt = 0;

    if (!savestate_state)
    {
	if (currprefs.chipset_mask & CSMASK_AGA) {
		oldovl = 1;
	}
    ciaatlatch = ciabtlatch = 0;
    ciaapra = 3; ciaaprb = ciaadra = ciaadrb = ciaasdr = 0;
    ciabprb = ciabdra = ciabdrb = ciabsdr = 0;
    ciaatod = ciabtod = 0; ciaatodon = ciabtodon = 0;
    ciaaicr = ciabicr = ciaaimask = ciabimask = 0;
    ciaacra = ciaacrb = ciabcra = ciabcrb = 0x4;                        
    ciaala = ciaalb = ciabla = ciablb = ciaata = ciaatb = ciabta = ciabtb = 0xFFFF;
    ciabpra = 0x8C;
    div10 = 0;
    }

    CIA_calctimers ();
    if (! ersatzkickfile) {
	int i = allocated_chipmem > 0x200000 ? allocated_chipmem >> 16 : 32;
	map_banks (&kickmem_bank, 0, i, 0x80000);
    }
    if (savestate_state)
    {
	                             
	if (currprefs.chipset_mask & CSMASK_AGA) {
		oldovl = 1;
	}
	uae_u8 v = ReadCIAA (0);
	WriteCIAA (0,3);
	WriteCIAA (0,0);
	WriteCIAA (0,v);
	                   
	DISK_select (ciabprb);
    }

}

                       

static uae_u32 cia_lget (uaecptr) REGPARAM;
static uae_u32 cia_wget (uaecptr) REGPARAM;
static uae_u32 cia_bget (uaecptr) REGPARAM;
static void cia_lput (uaecptr, uae_u32) REGPARAM;
static void cia_wput (uaecptr, uae_u32) REGPARAM;
static void cia_bput (uaecptr, uae_u32) REGPARAM;

addrbank cia_bank = {
    cia_lget, cia_wget, cia_bget,
    cia_lput, cia_wput, cia_bput,
    default_xlate, default_check, NULL
};

static void cia_wait (void)
{
    if (!div10)
	return;
    do_cycles(DIV10 - div10 + CYCLE_UNIT);
    CIA_handler ();
}

uae_u32 REGPARAM2 cia_bget (uaecptr addr)
{
   int r = (addr & 0xf00) >> 8;
   cia_wait ();
   switch ((addr >> 12) & 3)
   {
      case 0:
         return (addr & 1) ? ReadCIAA (r) : ReadCIAB (r);
      case 1:
         return (addr & 1) ? 0xff : ReadCIAB (r);
      case 2:
         return (addr & 1) ? ReadCIAA (r) : 0xff;
   }
   return 0xff;
}

uae_u32 REGPARAM2 cia_wget (uaecptr addr)
{
   int r = (addr & 0xf00) >> 8;
   cia_wait ();
   switch ((addr >> 12) & 3)
   {
      case 0:
         return (ReadCIAB (r) << 8) | ReadCIAA (r);
      case 1:
         return (ReadCIAB (r) << 8) | 0xff;
      case 2:
         return (0xff << 8) | ReadCIAA (r);
   }
   return 0xffff;
}

uae_u32 REGPARAM2 cia_lget (uaecptr addr)
{
    uae_u32 v;
    v = cia_wget (addr) << 16;
    v |= cia_wget (addr + 2);
    return v;
}

void REGPARAM2 cia_bput (uaecptr addr, uae_u32 value)
{
#ifndef USE_FAME_CORE
    value&=0xFF;
#endif
    int r = (addr & 0xf00) >> 8;
    cia_wait ();
    if ((addr & 0x2000) == 0)
	WriteCIAB (r, value);
    if ((addr & 0x1000) == 0)
	WriteCIAA (r, value);
}

void REGPARAM2 cia_wput (uaecptr addr, uae_u32 value)
{
#ifndef USE_FAME_CORE
    value&=0xFFFF;
#endif
    int r = (addr & 0xf00) >> 8;
    cia_wait ();
    if ((addr & 0x2000) == 0)
	WriteCIAB (r, value >> 8);
    if ((addr & 0x1000) == 0)
	WriteCIAA (r, value & 0xff);
}

void REGPARAM2 cia_lput (uaecptr addr, uae_u32 value)
{
    cia_wput (addr, value >> 16);
    cia_wput (addr + 2, value & 0xffff);
}

                             

static uae_u32 clock_lget (uaecptr) REGPARAM;
static uae_u32 clock_wget (uaecptr) REGPARAM;
static uae_u32 clock_bget (uaecptr) REGPARAM;
static void clock_lput (uaecptr, uae_u32) REGPARAM;
static void clock_wput (uaecptr, uae_u32) REGPARAM;
static void clock_bput (uaecptr, uae_u32) REGPARAM;

addrbank clock_bank = {
    clock_lget, clock_wget, clock_bget,
    clock_lput, clock_wput, clock_bput,
    default_xlate, default_check, NULL
};

uae_u32 REGPARAM2 clock_lget (uaecptr addr)
{
    return clock_bget (addr + 3);
}

uae_u32 REGPARAM2 clock_wget (uaecptr addr)
{
    return clock_bget (addr + 1);
}

uae_u32 REGPARAM2 clock_bget (uaecptr addr)
{
   time_t t = time(0);
   struct tm *ct;

   ct = localtime (&t);

   switch (addr & 0x3f) {
      case 0x03: return ct->tm_sec % 10;
      case 0x07: return ct->tm_sec / 10;
      case 0x0b: return ct->tm_min % 10;
      case 0x0f: return ct->tm_min / 10;
      case 0x13: return ct->tm_hour % 10;
      case 0x17: return ct->tm_hour / 10;
      case 0x1b: return ct->tm_mday % 10;
      case 0x1f: return ct->tm_mday / 10;
      case 0x23: return (ct->tm_mon+1) % 10;
      case 0x27: return (ct->tm_mon+1) / 10;
      case 0x2b: return ct->tm_year % 10;
      case 0x2f: return ct->tm_year / 10;

      case 0x33: return ct->tm_wday;                     
      case 0x37: return clock_control_d;
      case 0x3b: return clock_control_e;
      case 0x3f: return clock_control_f;
   }
   return 0;
}

void REGPARAM2 clock_lput (uaecptr addr, uae_u32 value)
{
                
}

void REGPARAM2 clock_wput (uaecptr addr, uae_u32 value)
{
                
}

void REGPARAM2 clock_bput (uaecptr addr, uae_u32 value)
{
    switch (addr & 0x3f) {
    case 0x37: clock_control_d = value; break;
    case 0x3b: clock_control_e = value; break;
    case 0x3f: clock_control_f = value; break;
    }
}


                                       

uae_u8 *restore_cia (int num, uae_u8 *src)
{
    uae_u8 b;
    uae_u16 w;
    uae_u32 l;

                       
    b = restore_u8 ();					           
    if (num) ciabpra = b; else ciaapra = b;
    b = restore_u8 ();					           
    if (num) ciabprb = b; else ciaaprb = b;
    b = restore_u8 ();					            
    if (num) ciabdra = b; else ciaadra = b;
    b = restore_u8 ();					            
    if (num) ciabdrb = b; else ciaadrb = b;
    w = restore_u16 ();					          
    if (num) ciabta = w; else ciaata = w;
    w = restore_u16 ();					          
    if (num) ciabtb = w; else ciaatb = w;
    l = restore_u8 ();					               
    l |= restore_u8 () << 8;
    l |= restore_u8 () << 16;
    if (num) ciabtod = l; else ciaatod = l;
    restore_u8 ();						              
    b = restore_u8 ();					           
    if (num) ciabsdr = b; else ciaasdr = b;
    b = restore_u8 ();					                                   
    if (num) ciabicr = b; else ciaaicr = b;
    b = restore_u8 ();					           
    if (num) ciabcra = b; else ciaacra = b;
    b = restore_u8 ();					           
    if (num) ciabcrb = b; else ciaacrb = b;

                       

    b = restore_u8 ();					              
    if (num) ciabimask = b; else ciaaimask = b;
    w = restore_u8 ();					                   
    w |= restore_u8 () << 8;
    if (num) ciabla = w; else ciaala = w;
    w = restore_u8 ();					                   
    w |= restore_u8 () << 8;
    if (num) ciablb = w; else ciaalb = w;
    w = restore_u8 ();					                       
    w |= restore_u8 () << 8;
    w |= restore_u8 () << 16;
    if (num) ciabtol = w; else ciaatol = w;
    l = restore_u8 ();					           
    l |= restore_u8 () << 8;
    l |= restore_u8 () << 16;
    if (num) ciabalarm = l; else ciaaalarm = l;
    b = restore_u8 ();
    if (num) ciabtlatch = b & 1; else ciaatlatch = b & 1;	                     
    if (num) ciabtodon = b & 2; else ciaatodon = b & 2;		                     
    if (num) {
	div10 = CYCLE_UNIT * restore_u8 ();
    }
    return src;
}

uae_u8 *save_cia (int num, int *len)
{
    uae_u8 *dstbak,*dst, b;
    uae_u16 t;

    dstbak = dst = (uae_u8 *)malloc (16 + 12 + 1);

    compute_passed_time ();

                       

    b = num ? ciabpra : ciaapra;				           
    save_u8 (b);
    b = num ? ciabprb : ciaaprb;				           
    save_u8 (b);
    b = num ? ciabdra : ciaadra;				            
    save_u8 (b); 
    b = num ? ciabdrb : ciaadrb;				            
    save_u8 (b);
    t = (num ? ciabta - ciabta_passed : ciaata - ciaata_passed);          
    save_u16 (t);
    t = (num ? ciabtb - ciabtb_passed : ciaatb - ciaatb_passed);          
    save_u16 (t);
    b = (num ? ciabtod : ciaatod);			            
    save_u8 (b);
    b = (num ? ciabtod >> 8 : ciaatod >> 8);		            
    save_u8 (b);
    b = (num ? ciabtod >> 16 : ciaatod >> 16);		            
    save_u8 (b);
    save_u8 (0);						              
    b = num ? ciabsdr : ciaasdr;				           
    save_u8 (b);
    b = num ? ciabicr : ciaaicr;				                                   
    save_u8 (b);
    b = num ? ciabcra : ciaacra;				           
    save_u8 (b);
    b = num ? ciabcrb : ciaacrb;				           
    save_u8 (b);

                           

    save_u8 (num ? ciabimask : ciaaimask);			         
    b = (num ? ciabla : ciaala);			                      
    save_u8 (b);
    b = (num ? ciabla >> 8 : ciaala >> 8);		                      
    save_u8 (b);
    b = (num ? ciablb : ciaalb);			                      
    save_u8 (b);
    b = (num ? ciablb >> 8 : ciaalb >> 8);		                      
    save_u8 (b);
    b = (num ? ciabtol : ciaatol);			                    
    save_u8 (b);
    b = (num ? ciabtol >> 8 : ciaatol >> 8);		                     
    save_u8 (b);
    b = (num ? ciabtol >> 16 : ciaatol >> 16);		                    
    save_u8 (b);
    b = (num ? ciabalarm : ciaaalarm);			              
    save_u8 (b);
    b = (num ? ciabalarm >> 8 : ciaaalarm >>8 );	               
    save_u8 (b);
    b = (num ? ciabalarm >> 16 : ciaaalarm >> 16);	              
    save_u8 (b);
    b = 0;
    if (num)
	b |= ciabtlatch ? 1 : 0;
    else
	b |= ciaatlatch ? 1 : 0;                      
    if (num)
	b |= ciabtodon ? 2 : 0;
    else
	b |= ciaatodon ? 2 : 0;                     
    save_u8 (b);
	                                  
	save_u8 (div10 / CYCLE_UNIT);
    *len = dst - dstbak;
    return dstbak;
}
