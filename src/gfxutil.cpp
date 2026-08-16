   
                                 
   
                                                           
   
                                                      
    

#include "sysconfig.h"
#include "sysdeps.h"

#include "config.h"
#include "uae.h"
#include "options.h"
#include "thread.h"
#include "memory-uae.h"
#include "custom.h"
#include "keyboard.h"
#include "xwin.h"
#include "keybuf.h"

#define	RED 	0
#define	GRN	1
#define	BLU	2


unsigned long doMask (int p, int bits, int shift)
{
                                                    
                                                                       

    unsigned long val = p * 0x11111111UL;
    val >>= (32 - bits);
    val <<= shift;

    return val;
}


static unsigned int doColor(int i, int bits, int shift)
{
    int shift2;
    if(bits >= 8) shift2 = 0; else shift2 = 8 - bits;
    return (i >> shift2) << shift;
}

void alloc_colors64k (int rw, int gw, int bw, int rs, int gs, int bs)
{
	int i;
	for (i = 0; i < 4096; i++) {
		int r = i >> 8;
		int g = (i >> 4) & 0xF;
		int b = i & 0xF;
		xcolors[i] = doMask(r, rw, rs) | doMask(g, gw, gs) | doMask(b, bw, bs);
	}
	                             
	for(i=0; i<256; i++) {
		xredcolors[i] = doColor(i, rw, rs);
		xgreencolors[i] = doColor(i, gw, gs);
		xbluecolors[i] = doColor(i, bw, bs);
	}
}
