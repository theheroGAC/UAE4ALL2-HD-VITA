                                                              
                                                                                    

                      
                                        
                                                          
#ifndef _ALL_SOURCE
                        
#endif

                                                    
                  

                                                 
#define HAVE_GETMNTENT 1

                                                
                           

                                                                        
#define HAVE_UTIME_NULL 1

                                                                 
                   

                                                       
                   

                                                        
                  

                                                       
                  

                                                                        
                          

                                                                  
#define RETSIGTYPE void

                                                  
#define STDC_HEADERS 1

                                                                       
#define TIME_WITH_SYS_TIME 1

                                                      
                           

                                                                  
                              

                                                 
                

                                                                  
                                                                            
                          

                                                                      
                                                                       
                                                
                                

                                                                     
                                                         
                              

                                                                     
                                     
                           

                                                                     
                                                                     
                                          
#define MOUNTED_GETMNTENT1 1

                                                                        
                                                                          
                               

                                                                      
                                        
                               

                                                                     
                                            
                               

                                                                      
                                                                     
                                            
                           

                                                                  
                              

                                                                        
                                                                       
                             

                                                                             
                                         
#define STAT_STATFS2_BSIZE 1

                                                                             
                       
                               

                                                            
                                     
                                 

                                                                   
                         

                                                           
                         

                                                                        
                                                             
                                                                       
                                             
                                          

                                        
#define SIZEOF___INT64 8

                                     
#define SIZEOF_CHAR 1

                                    
#define SIZEOF_INT 4

                                     
#define SIZEOF_LONG 4

                                          
#define SIZEOF_LONG_LONG 8

                                      
#define SIZEOF_SHORT 2

                                             
#define HAVE_BCOPY 1

                                                 
                           

                                                
#define HAVE_ENDGRENT 1

                                                
#define HAVE_ENDPWENT 1

                                              
#define HAVE_FCHDIR 1

                                             
#define HAVE_FTIME 1

                                                 
#define HAVE_FTRUNCATE 1

                                              
#define HAVE_GETCWD 1

                                                  
                            

                                              
#define HAVE_GETOPT 1

                                                    
                               

                                               
#define HAVE_ISASCII 1

                                              
#define HAVE_LCHOWN 1

                                                  
                            

                                              
#define HAVE_MEMCPY 1

                                             
#define HAVE_MKDIR 1

                                              
#define HAVE_MKFIFO 1

                                                 
                           

                                             
#define HAVE_RMDIR 1

                                              
#define HAVE_SELECT 1

                                                 
#define HAVE_SETITIMER 1

                                                 
#define HAVE_SIGACTION 1

                                              
#define HAVE_STRCHR 1

                                              
#define HAVE_STRDUP 1

                                                
#define HAVE_STRERROR 1

                                               
#define HAVE_STRRCHR 1

                                              
#define HAVE_STRSTR 1

                                                 
#define HAVE_TCGETATTR 1

                                                
#define HAVE_VFPRINTF 1

                                               
#define HAVE_VPRINTF 1

                                                
#define HAVE_VSPRINTF 1

                                                      
                           

                                                     
#define HAVE_CURSES_H 1

                                                                        
                                             

                                                    
                         

                                                          
                               

                                                     
#define HAVE_DIRENT_H 1

                                                           
                                

                                                    
#define HAVE_FCNTL_H 1

                                                       
#define HAVE_FEATURES_H 1

                                                     
#define HAVE_GETOPT_H 1

                                                                      
                                           

                                                               
                                    

                                                                
                                     

                                                     
#define HAVE_MNTENT_H 1

                                                     
                          

                                                      
#define HAVE_NCURSES_H 1

                                                   
                        

                                                        
                             

                                                     
#define HAVE_STRING_H 1

                                                      
#define HAVE_STRINGS_H 1

                                                          
                               

                                                          
                               

                                                      
                           

                                                         
                              

                                                             
                                  

                                                           
                                

                                                        
                             

                                                        
#define HAVE_SYS_IOCTL_H 1

                                                      
#define HAVE_SYS_IPC_H 1

                                                        
#define HAVE_SYS_MOUNT_H 1

                                                       
                            

                                                        
#define HAVE_SYS_PARAM_H 1

                                                      
#define HAVE_SYS_SHM_H 1

                                                            
#define HAVE_SYS_SOUNDCARD_H 1

                                                       
#define HAVE_SYS_STAT_H 1

                                                         
#define HAVE_SYS_STATFS_H 1

                                                          
                               

                                                          
#define HAVE_SYS_TERMIOS_H 1

                                                       
#define HAVE_SYS_TIME_H 1

                                                        
#define HAVE_SYS_TYPES_H 1

                                                        
                             

                                                      
#define HAVE_SYS_VFS_H 1

                                                     
#define HAVE_UNISTD_H 1

                                                    
#define HAVE_UTIME_H 1

                                                     
                          

                                                      
                           

#ifdef USE_FAME_CORE
	                                                              
                                      
    
#	define uae_malloc(len) malloc((len + 1) & ~1);
#endif

#if defined(__PSP2__) || defined(__SWITCH__)
#undef HAVE_SYS_MOUNT_H
#undef HAVE_SYS_VFS_H
#undef HAVE_SYS_STATFS_H
#ifdef __PSP2__                  
#include <psp2/power.h>
#endif
#include "psp2_input.h"
#ifdef DEBUG_UAE4ALL
void vita_write_log (const char *, ...);
#endif
#endif

#ifdef __SWITCH__
#define MAX_NUM_CONTROLLERS 8
#else
#define MAX_NUM_CONTROLLERS 4
#endif

#define MAX_NUM_CUSTOM_PRESETS 6

