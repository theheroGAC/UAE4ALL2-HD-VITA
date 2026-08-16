#ifndef STANDALONE_CONFIG_H
#define STANDALONE_CONFIG_H

#include <SDL.h>
#include <stdbool.h>

#define GAME_PATH "ux0:/data/MioGiocoAmiga/"
#define ROM_NAME "kick3.rom"
#define DISK1_NAME "disk1.adf"
#define DISK2_NAME "disk2.adf"

#define PATH_KICK3 GAME_PATH ROM_NAME
#define PATH_DISK1 GAME_PATH DISK1_NAME
#define PATH_DISK2 GAME_PATH DISK2_NAME

#ifdef __cplusplus
extern "C" {
#endif

   
                                             
   
bool FileExists(const char *path);

   
                                                                                                         
   
void ShowErrorAndExit(const char *line1, const char *line2, const char *line3, const char *line4);

   
                                                                                    
   
bool Standalone_CheckBootFiles(void);

   
                                                                                         
   
void Standalone_ConfigureEmulator(void);

#ifdef __cplusplus
}
#endif

#endif                       
