   
                                 
   
          
                                                                     
                                                                         
             
   
                                     
    

#include "rpt.h"

extern void reset_frame_rate_hack (void);
extern int rpt_available;
extern frame_time_t syncbase;
extern unsigned long last_synctime;

extern unsigned long currcycle, nextevent;
extern unsigned long sample_evtime;
typedef void (*evfunc)(void);

struct ev
{
    int active;
    unsigned long int evtime, oldcycles;
    evfunc handler;
};

enum {
    ev_hsync, ev_copper, ev_audio, ev_cia, ev_blitter, ev_disk,
    ev_max
};

extern struct ev eventtab[ev_max];

static __inline__ void events_schedule (void)
{
    int i;

    unsigned long int mintime = ~0L;
    for (i = 0; i < ev_max; i++) {
	if (eventtab[i].active) {
	    unsigned long int eventtime = eventtab[i].evtime - currcycle;
	    if (eventtime < mintime)
		mintime = eventtime;
	}
    }
    nextevent = currcycle + mintime;
}

static __inline__ void do_cycles_slow (unsigned long cycles_to_add)
{
    while ((nextevent - currcycle) <= cycles_to_add) {
        int i;
        cycles_to_add -= (nextevent - currcycle);
        currcycle = nextevent;

        for (i = 0; i < ev_max; i++) {
	    if (eventtab[i].active && eventtab[i].evtime == currcycle) {
		(*eventtab[i].handler)();
	    }
	}
        events_schedule();
    }
    currcycle += cycles_to_add;
}

                                                                           
                                                                      
                                                                         
                                                      
static __inline__ void handle_active_events (void)
{
    int i;
    for (i = 0; i < ev_max; i++) {
	if (eventtab[i].active && eventtab[i].evtime == currcycle) {
	    (*eventtab[i].handler)();
	}
    }
}

static __inline__ unsigned long get_cycles (void)
{
    return currcycle;
}

extern void init_eventtab (void);


#define do_cycles do_cycles_slow


