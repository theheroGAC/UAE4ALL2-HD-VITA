   
                                 
   
                         
   
                                
    

extern void do_start_program (void);
extern void do_leave_program (void);
extern void start_program (void);
extern void leave_program (void);
extern void real_main (int, char **);
extern void usage (void);

#define NUM_DRIVES 4


extern void uae_reset (void);
extern void uae_quit (void);
extern void reset_all_systems (void);

extern int quit_program;

extern char warning_buffer[256];

                                                                     
                                            
                                                                        
                     
                     
                                                   
                                                              
                   
   
struct bstring {
    const char *data;
    int val;
};
