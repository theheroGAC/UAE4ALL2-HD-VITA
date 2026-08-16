   
                                 
   
                                                          
   
                               
    
                                                       
                             
    

#include "sysconfig.h"
#include "sysdeps.h"

#include "config.h"
#include "thread.h"
#include "options.h"
#include "memory-uae.h"
#include "custom.h"
#include "m68k/m68k_intrf.h"
#include "disk.h"
#include "autoconf.h"
#include "filesys.h"
#include "execlib.h"
#include "native2amiga.h"

smp_comm_pipe native2amiga_pending;

  
                                            
   

void native2amiga_install (void)
{
    init_comm_pipe (&native2amiga_pending, 10, 2);
}

  
                                                                 
   
void native2amiga_startup (void)
{
}

#ifdef SUPPORT_THREADS
void uae_ReplyMsg(uaecptr msg)
{
    write_comm_pipe_int (&native2amiga_pending, 2, 0);
    write_comm_pipe_u32 (&native2amiga_pending, msg, 1);

    uae_int_requested = 1;
}

void uae_PutMsg(uaecptr port, uaecptr msg)
{
    uae_pt data;
    data.i = 1;
    write_comm_pipe_int (&native2amiga_pending, 1, 0);
    write_comm_pipe_u32 (&native2amiga_pending, port, 0);
    write_comm_pipe_u32 (&native2amiga_pending, msg, 1);

    uae_int_requested = 1;
}

void uae_Signal(uaecptr task, uae_u32 mask)
{
    write_comm_pipe_int (&native2amiga_pending, 0, 0);
    write_comm_pipe_u32 (&native2amiga_pending, task, 0);
    write_comm_pipe_int (&native2amiga_pending, mask, 1);
    
    uae_int_requested = 1;
}
#endif

void uae_NewList(uaecptr list)
{
    put_long (list, list + 4);
    put_long (list + 4, 0);
    put_long (list + 8, list);
}

uaecptr uae_AllocMem (uae_u32 size, uae_u32 flags)
{
    m68k_dreg (regs, 0) = size;
    m68k_dreg (regs, 1) = flags;
    return CallLib (get_long (4), -198);               
}

void uae_FreeMem (uaecptr memory, uae_u32 size)
{
    m68k_dreg (regs, 0) = size;
    m68k_areg (regs, 1) = memory;
    CallLib (get_long (4), -0xD2);              
}
