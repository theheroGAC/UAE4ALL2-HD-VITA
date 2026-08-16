
#ifndef h_pnd_device_h
#define h_pnd_device_h

#ifdef __cplusplus
extern "C" {
#endif

                                                                       
                              

                                                                        
                  
                                          

                                                                                 
                                                         
   
#define PND_DEVICE_PROC_CLOCK "/proc/pandora/cpu_mhz_max"
#define PND_DEVICE_SYS_BACKLIGHT_BRIGHTNESS "/sys/class/backlight/gpio-backlight/brightness"
#define PND_DEVICE_FRAMEBUFFER "/dev/fb0"
#define PND_DEVICE_NUB1 "/dev/input/js1"
#define PND_DEVICE_NUB2 "/dev/input/js2"
#define PND_DEVICE_BATTERY_GAUGE_PERC "/sys/class/power_supply/bq27500-0/capacity"
#define PND_DEVICE_CHARGE_CURRENT "/sys/class/power_supply/bq27500-0/current_now"

#define PND_DEVICE_LED_CHARGER "/sys/class/leds/pandora::charger"
#define PND_DEVICE_LED_POWER   "/sys/class/leds/pandora::power"
#define PND_DEVICE_LED_SD1     "/sys/class/leds/pandora::sd1"
#define PND_DEVICE_LED_SD2     "/sys/class/leds/pandora::sd2"
#define PND_DEVICE_LED_WIFI    "/sys/class/leds/pandora::wifi"
#define PND_DEVICE_LED_BT      "/sys/class/leds/pandora::bluetooth"
#define PND_DEVICE_LED_SUFFIX_BRIGHTNESS "/brightness"

               
#define PND_EVDEV_NUB1    "nub0"               
#define PND_EVDEV_NUB2    "nub1"               
#define PND_EVDEV_KEYPAD  "keypad"                         
#define PND_EVDEV_GPIO    "gpio-keys"
#define PND_EVDEV_TS      "touchscreen"                          
#define PND_EVDEV_POWER   "power-button"                        

          
   
unsigned char pnd_device_open_write_close ( char *name, char *v );
unsigned char pnd_device_open_read_close ( char *name, char *r_buffer, unsigned int buffer_len );

                      
                                                                            
                                            
   
unsigned char pnd_device_set_clock ( unsigned int c );                         
unsigned int pnd_device_get_clock ( void );

                                                
                       
   
int pnd_device_get_battery_gauge_perc ( void );
unsigned char pnd_device_get_charge_current ( int *result );                                                     

                    

                    
unsigned char pnd_device_set_backlight ( unsigned int v );                          
unsigned int pnd_device_get_backlight ( void );

                          
unsigned char pnd_device_set_led_power_brightness ( unsigned char v );         
unsigned char pnd_device_set_led_charger_brightness ( unsigned char v );         

                        

#ifdef __cplusplus
}          
#endif

#endif
