  
                                 
   
                                                                             
   
                                
    

#ifndef _RPT_H_
#define _RPT_H_

#if defined(__PSP2__)                  
#include <psp2/kernel/processmgr.h>
#endif

#if defined(__SWITCH__)
#include <switch.h>
#endif

typedef unsigned long frame_time_t;

extern int64_t g_uae_epoch;

static __inline__ frame_time_t read_processor_time (void)
{
  int64_t time;
#if defined(__PSP2__)                  
  time = sceKernelGetProcessTimeWide();
#elif defined(__SWITCH__)
  time = (int64_t) ((svcGetSystemTick() * 1000000) / 19200000);
#else
  struct timespec ts;
  
  clock_gettime (CLOCK_MONOTONIC, &ts);

  time = (((int64_t) ts.tv_sec) * 1000000) + (ts.tv_nsec / 1000);
#endif
  return time - g_uae_epoch;
}

#endif              

