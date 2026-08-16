
#ifndef h_pnd_io_evdev_h
#define h_pnd_io_evdev_h

                                                                                
                                                                                
                                  
  
                                                                                
                                                                                          
                                      
  
                                                                           
  

                                                                   
                                                                                    

typedef enum {
  pnd_evdev_dpads = 0,                                 
  pnd_evdev_nub1,
  pnd_evdev_nub2,
  pnd_evdev_power,
  pnd_evdev_max
} pnd_evdev_e;

unsigned char pnd_evdev_open ( pnd_evdev_e device );                                  
void pnd_evdev_close ( pnd_evdev_e device );
void pnd_evdev_closeall ( void );
int pnd_evdev_get_fd ( unsigned char handle );                                
int pnd_evdev_open_by_name ( char *devname );                                                        

typedef enum {
  pnd_evdev_left = (1<<0),                                                              
  pnd_evdev_right = 1<<1,
  pnd_evdev_up = 1<<2,
  pnd_evdev_down = 1<<3,
  pnd_evdev_x = 1<<4,
  pnd_evdev_y = 1<<5,
  pnd_evdev_a = 1<<6,
  pnd_evdev_b = 1<<7,
  pnd_evdev_ltrigger = 1<<8,
  pnd_evdev_rtrigger = 1<<9,
  pnd_evdev_start = 1<<10,
  pnd_evdev_select = 1<<11,
  pnd_evdev_pandora = 1<<12
} pnd_evdev_dpad_e;

typedef struct {
  int x;
  int y;
} pnd_nubstate_t;

                                          
                                                                                              
                                                                          
unsigned char pnd_evdev_catchup ( unsigned char blockp );                            

                                                                   
                               
unsigned int pnd_evdev_dpad_state ( pnd_evdev_e device );                                       

                                               
                                                                                                   
                                     
                                   
int pnd_evdev_nub_state ( pnd_evdev_e nubdevice, pnd_nubstate_t *r_nubstate );

#endif
