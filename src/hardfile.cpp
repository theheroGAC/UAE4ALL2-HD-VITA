   
                                 
   
                      
   
                                
    

#include "sysconfig.h"
#include "sysdeps.h"

#include "config.h"
#include "options.h"
#include "memory-uae.h"
#include "custom.h"
#include "m68k/m68k_intrf.h"
#include "disk.h"
#include "autoconf.h"
#include "filesys.h"
#include "execlib.h"
#include "gui.h"

static int opencount = 0;

static uae_u32 hardfile_open (void)
{
    uaecptr tmp1 = m68k_areg(regs, 1);            

                           
    if (get_hardfile_data (m68k_dreg (regs, 0))) {
	opencount++;
	put_word (m68k_areg(regs, 6)+32, get_word (m68k_areg(regs, 6)+32) + 1);
	put_long (tmp1 + 24, m68k_dreg (regs, 0));              
	put_byte (tmp1 + 31, 0);               
	put_byte (tmp1 + 8, 7);                            
	return 0;
    }

    put_long (tmp1 + 20, (uae_u32)-1);
    put_byte (tmp1 + 31, (uae_u8)-1);
    return (uae_u32)-1;
}

static uae_u32 hardfile_close (void)
{
    opencount--;
    put_word (m68k_areg(regs, 6) + 32, get_word (m68k_areg(regs, 6) + 32) - 1);

    return 0;
}

static uae_u32 hardfile_expunge (void)
{
    return 0;                                
}

static uae_u32 hardfile_beginio (void)
{
	uae_u32 tmp1, tmp2, dataptr, offset;
	uae_u32 retval = m68k_dreg(regs, 0);
	int unit;
	struct hardfiledata *hfd;
	
	tmp1 = m68k_areg(regs, 1);
	unit = get_long (tmp1 + 24);

	hfd = get_hardfile_data (unit);
	
	put_byte (tmp1+8, NT_MESSAGE);
	put_byte (tmp1+31, 0);                   
	tmp2 = get_word (tmp1+28);                 
	                                                        
	switch (tmp2) {
		case CMD_READ:
			gui_data.hdled = HDLED_READ;
			
			dataptr = get_long (tmp1 + 40);
			if (dataptr & 1)
				goto bad_command;
			offset = get_long (tmp1 + 44);
			if (offset & 511)
				goto bad_command;
			tmp2 = get_long (tmp1 + 36);                
			if (tmp2 & 511)
				goto bad_command;
			if (tmp2 + offset > (uae_u32)hfd->size)
				goto bad_command;
			
			put_long (tmp1 + 32, tmp2);                    
			fseek (hfd->fd, offset, SEEK_SET);
			while (tmp2) {
				int i;
				char buffer[512];
				                                  
				fread (buffer, 1, 512, hfd->fd);
				for (i = 0; i < 512; i++, dataptr++)
					put_byte(dataptr, buffer[i]);
				tmp2 -= 512;
			}
			break;
			
		case CMD_WRITE:
		case 11:             
			gui_data.hdled = HDLED_WRITE;
			
			dataptr = get_long (tmp1 + 40);
			if (dataptr & 1)
				goto bad_command;
			offset = get_long (tmp1 + 44);
			if (offset & 511)
				goto bad_command;
			tmp2 = get_long (tmp1 + 36);                
			if (tmp2 & 511)
				goto bad_command;
			if (tmp2 + offset > (uae_u32)hfd->size)
				goto bad_command;
			
			put_long (tmp1 + 32, tmp2);                    
			fseek (hfd->fd, offset, SEEK_SET);
			while (tmp2) {
				char buffer[512];
				int i;
				for (i=0; i < 512; i++, dataptr++)
					buffer[i] = get_byte(dataptr);
				fwrite (buffer, 1, 512, hfd->fd);
				tmp2 -= 512;
			}
			break;
			
			bad_command:
			break;
			
		case 18:                   
			printf ("Shouldn't happen\n");
			put_long (tmp1 + 32, 1);                                       
			break;
			
		case 19:                   
			printf ("Shouldn't happen 2\n");
			put_long (tmp1 + 32, 0);
			break;
			
			                                                        
		case CMD_UPDATE:
		case CMD_CLEAR:
		case 9:            
		case 10:           
		case 12:             
		case 13:                
		case 14:                   
		case 15:                 
		case 20:                   
		case 21:                   
			put_long (tmp1+32, 0);                
			retval = 0;
			break;
			
		default:
			                             
			put_byte (tmp1+31, (uae_u8)-3);               
			retval = 0;
			break;
	}
	return retval;
}

static uae_u32 hardfile_abortio (void)
{
    return (uae_u32)-3;
}

void hardfile_install (void)
{
    uae_u32 functable, datatable;
    uae_u32 initcode, openfunc, closefunc, expungefunc;
    uae_u32 beginiofunc, abortiofunc;

    ROM_hardfile_resname = ds ("uaehf.device");
    ROM_hardfile_resid = ds ("UAE hardfile.device 0.2");

                  
    initcode = filesys_initcode;

              
    openfunc = here ();
    calltrap (deftrap (hardfile_open)); dw (RTS);

               
    closefunc = here ();
    calltrap (deftrap (hardfile_close)); dw (RTS);

                 
    expungefunc = here ();
    calltrap (deftrap (hardfile_expunge)); dw (RTS);

                 
    beginiofunc = here ();
    calltrap (deftrap (hardfile_beginio));
    dw (0x48E7); dw (0x8002);                          
    dw (0x0829); dw (0); dw (30);                     
    dw (0x6608);               
    dw (0x2C78); dw (0x0004);                  
    dw (0x4EAE); dw (-378);                       
    dw (0x4CDF); dw (0x4001);                          
    dw (RTS);

                 
    abortiofunc = here ();
    calltrap (deftrap (hardfile_abortio)); dw (RTS);

                   
    functable = here ();
    dl (openfunc);           
    dl (closefunc);            
    dl (expungefunc);              
    dl (EXPANSION_nullfunc);           
    dl (beginiofunc);              
    dl (abortiofunc);              
    dl (0xFFFFFFFFul);                   

                   
    datatable = here ();
    dw (0xE000);               
    dw (0x0008);              
    dw (0x0300);                
    dw (0xC000);               
    dw (0x000A);              
    dl (ROM_hardfile_resname);
    dw (0xE000);               
    dw (0x000E);                
    dw (0x0600);                                  
    dw (0xD000);               
    dw (0x0014);                  
    dw (0x0004);          
    dw (0xD000);
    dw (0x0016);                   
    dw (0x0000);
    dw (0xC000);
    dw (0x0018);                   
    dl (ROM_hardfile_resid);
    dw (0x0000);                   

    ROM_hardfile_init = here ();
    dl (0x00000100);          
    dl (functable);
    dl (datatable);
    dl (initcode);
}
