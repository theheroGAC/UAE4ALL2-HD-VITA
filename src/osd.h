#ifndef OSD_H
#define OSD_H

#include <SDL.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

   
                                                 
                                               
                                                             
   
void OSD_TriggerDiskSwap(int disk_number, bool is_error);

   
                                                                              
                                                                                   
   
void OSD_Render(SDL_Surface *surface);

#ifdef __cplusplus
}
#endif

#endif         
