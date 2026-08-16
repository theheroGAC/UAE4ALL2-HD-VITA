#include <string.h>
#include <stdbool.h>

#include <switch.h>
#include "switch_kbd.h"

void kbdswitch_get(char *title, const char *initial_text, int maxLen, int multiline, char *buf) {
	
	Result rc=0;
	
	SwkbdConfig kbd;
	
	rc = swkbdCreate(&kbd, 0);
	
	if (R_SUCCEEDED(rc)) {
		                                  
		swkbdConfigMakePresetDefault(&kbd);
		                                      
		                                      
		                                          
		
		                                                    
		                                             
		                                                 
		                                                  
		                                           
		                                     
		                                         
		
		                                                                                           
		
		                                      
		swkbdConfigSetInitialText(&kbd, initial_text);
		
		                                               
		
		rc = swkbdShow(&kbd, buf, maxLen);
		
		swkbdClose(&kbd);
	}
}
