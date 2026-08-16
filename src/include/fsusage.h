                                                            
                                                          

                                                                       
                                                                       
                                                                      
                     

                                                                  
                                                                 
                                                                
                                               

                                                                    
                                                                          
                                                                     

                                                                    
struct fs_usage
{
  long fsu_blocks;		                   
  long fsu_bfree;		                                         
  long fsu_bavail;		                                             
  long fsu_files;		                       
  long fsu_ffree;		                      
};

#ifndef __P
#if defined (__GNUC__) || (defined (__STDC__) && __STDC__)
#define __P(args) args
#else
#define __P(args) ()
#endif             
#endif                 

int get_fs_usage __P ((const char *path, const char *disk,
		       struct fs_usage *fsp));
