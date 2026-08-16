   
                                 
   
                                                                
                            
   
                               
    
                                                       
                                        
    

  
                                                             
                                                                  
                                            
   
#ifdef SUPPORT_THREADS
void uae_ReplyMsg(uaecptr msg);
void uae_PutMsg(uaecptr port, uaecptr msg);
void uae_Signal(uaecptr task, uae_u32 mask);
#endif
void uae_NewList(uaecptr list);

  
                                                    
                                                    
                                                   
                                               
                                                    
   
uaecptr uae_AllocMem (uae_u32 size, uae_u32 flags);


  
                                            
   
void native2amiga_install (void);

  
                                                                 
   
void native2amiga_startup (void);

                          
#ifdef SUPPORT_THREADS
                                                
                                                
                                                                
                                           
                                                               
extern smp_comm_pipe native2amiga_pending;
#endif
