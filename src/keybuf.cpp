   
                                 
   
                                                                          
                      
   
                                                                          
                                                                           
                                                                         
                                                      
   
                                      
    

#include "sysconfig.h"
#include "sysdeps.h"
#include <assert.h>

#include "config.h"
#include "uae.h"
#include "options.h"
#include "keybuf.h"
#include "keyboard.h"
#include "joystick.h"
#include "custom.h"

static int kpb_first, kpb_last;

static int keybuf[256];

int keys_available (void)
{
    int val;
    val = kpb_first != kpb_last;
    return val;
}

int get_next_key (void)
{
    int key;
    assert (kpb_first != kpb_last);

    key = keybuf[kpb_last];
    if (++kpb_last == 256)
	kpb_last = 0;
    return key;
}

void record_key (int kc)
{
    int kpb_next = kpb_first + 1;

    if (kpb_next == 256)
	kpb_next = 0;
    if (kpb_next == kpb_last) {
	write_log ("Keyboard buffer overrun. Congratulations.\n");
	return;
    }
    if ((kc >> 1) == AK_RCTRL) {
	kc ^= AK_RCTRL << 1;
	kc ^= AK_CTRL << 1;
    }
    keybuf[kpb_first] = kc;
    kpb_first = kpb_next;
}

void keybuf_init (void)
{
    kpb_first = kpb_last = 0;
}
