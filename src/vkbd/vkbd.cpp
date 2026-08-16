#include<stdio.h>
#include<stdlib.h>
#include<SDL.h>

#ifdef USE_SDL2
#include "sdl2_to_sdl1.h"
#endif

#include "vkbd.h"

#include "keyboard.h"

#define MIN(a,b) ((a) < (b) ? (a) : (b))

#define VKBD_MIN_HOLDING_TIME 200
#define VKBD_MOVE_DELAY 50

extern int keycode2amiga(SDL_keysym *prKeySym);

extern int mainMenu_displayHires;
extern int mainMenu_displayedLines;
extern int mainMenu_shader;
extern int presetModeId;
extern int visibleAreaWidth;
extern int mainMenu_vkbdLanguage;
extern int mainMenu_vkbdStyle;

static int vkbd_x=VKBD_X;
static int vkbd_y=VKBD_Y;
static int vkbd_transparency=255;
static int vkbd_just_blinked=0;
static Uint32 vkbd_last_press_time=0;
static Uint32 vkbd_last_move_time=0;

                                     
t_vkbd_sticky_key vkbd_sticky_key[] = 
{
	{ AK_LSH, false, true, 70},
	{ AK_RSH, false, true, 81},
	{ AK_CTRL, false, true, 54},
	{ AK_LALT, false, true, 85},
	{ AK_RALT, false, true, 89},
	{ AK_LAMI, false, true, 86},
	{ AK_RAMI, false, true, 88}
};

int vkbd_let_go_of_direction=0;
int vkbd_mode=0;
int vkbd_move=0;
float vkbd_touch_x=-1;
float vkbd_touch_y=-1;
int vkbd_key=KEYCODE_NOTHING;
SDLKey vkbd_button2=(SDLKey)0;                  
int vkbd_keysave=KEYCODE_NOTHING;

#if !defined (DREAMCAST) && !defined (GP2X) && !defined (PSP) && !defined (GIZMONDO) 

int vkbd_init(void) { return 0; }
void vkbd_init_button2(void) { }
void vkbd_quit(void) { }
int vkbd_process(void) { return KEYCODE_NOTHING; }
void vkbd_displace_up(void) { };
void vkbd_displace_down(void) { };
void vkbd_transparency_up(void) { };
void vkbd_transparency_down(void) { };
#else

extern SDL_Surface *prSDLScreen;

static SDL_Surface *ksur;
static SDL_Surface *ksurHires;
static SDL_Surface *canvas;                                                      
static SDL_Surface *canvasHires;                                                      
static SDL_Surface *ksurShift;
static SDL_Surface *ksurShiftHires;

static int vkbd_actual=0, vkbd_color=0;

#ifdef GP2X
extern char launchDir [256];
#endif

typedef struct{
	SDL_Rect rect;
	unsigned char up,down,left,right;
	int key;
} t_vkbd_rect;

static t_vkbd_rect *vkbd_rect;

                                                                
                                                               
static t_vkbd_rect vkbd_rect_US[]= 
{
	{{  1,  1, 29, 11 },85,17,16, 1, AK_ESC},                   
	{{ 31,  1, 14, 11 },86,18, 0, 2, AK_F1},	    
	{{ 46,  1, 14, 11 },87,19, 1, 3, AK_F2},	    
	{{ 61,  1, 14, 11 },87,20, 2, 4, AK_F3},	    
	{{ 76,  1, 14, 11 },87,21, 3, 5, AK_F4},	    
	{{ 91,  1, 14, 11 },87,22, 4, 6, AK_F5},	     
	{{106,  1, 14, 11 },87,23, 5, 7, AK_F6},	    
	{{121,  1, 14, 11 },87,24, 6, 8, AK_F7},	     
	{{136,  1, 14, 11 },87,25, 7, 9, AK_F8},	     
	{{151,  1, 14, 11 },87,26, 8,10, AK_F9},	    
	{{166,  1, 14, 11 },87,27, 9,11, AK_F10},	     
	{{181,  1, 29, 11 },88,28,10,12, AK_DEL},	     
	{{211,  1, 29, 11 },90,30,11,13, AK_HELP},	     
	{{241,  1, 14, 11 },92,32,12,14, AK_NPLPAREN},	     
	{{256,  1, 14, 11 },69,33,13,15, AK_NPRPAREN},	     
	{{271,  1, 14, 11 },69,34,14,16, AK_NPDIV},	     
	{{286,  1, 14, 11 },69,35,15,0, AK_NPMUL},	     
	
	{{  1, 13, 29, 11 }, 0,36,35,18, AK_BACKQUOTE},                    
	{{ 31, 13, 14, 11 }, 1,37,17,19, AK_1},	     
	{{ 46, 13, 14, 11 }, 2,38,18,20, AK_2},	     
	{{ 61, 13, 14, 11 }, 3,39,19,21, AK_3},	     
	{{ 76, 13, 14, 11 }, 4,40,20,22, AK_4},	     
	{{ 91, 13, 14, 11 }, 5,41,21,23, AK_5},	     
	{{106, 13, 14, 11 }, 6,42,22,24, AK_6},	     
	{{121, 13, 14, 11 }, 7,43,23,25, AK_7},	      
	{{136, 13, 14, 11 }, 8,44,24,26, AK_8},	     
	{{151, 13, 14, 11 }, 9,45,25,27, AK_9},	     
	{{166, 13, 14, 11 },10,46,26,28, AK_0},	     
	{{181, 13, 14, 11 },11,47,27,29, AK_MINUS},	     
	{{196, 13, 14, 11 },11,48,28,30, AK_EQUAL},	     
	{{211, 13, 14, 11 },12,49,29,31, AK_BACKSLASH},	     
	{{226, 13, 14, 11 },12,49,30,32, AK_BS},	     
	{{241, 13, 14, 11 },13,50,31,33, AK_NP7},	     
	{{256, 13, 14, 11 },14,51,32,34, AK_NP8},	     
	{{271, 13, 14, 11 },15,52,33,35, AK_NP9},	     
	{{286, 13, 14, 11 },16,53,34,17, AK_NPSUB},	     
	
	{{  1, 25, 29, 11 }, 17,54,53,37, AK_TAB},                    
	{{ 31, 25, 14, 11 }, 18,55,36,38, AK_Q},	     
	{{ 46, 25, 14, 11 }, 19,56,37,39, AK_W},	     
	{{ 61, 25, 14, 11 }, 20,57,38,40, AK_E},	     
	{{ 76, 25, 14, 11 }, 21,58,39,41, AK_R},	     
	{{ 91, 25, 14, 11 }, 22,59,40,42, AK_T},	     
	{{106, 25, 14, 11 }, 23,60,41,43, AK_Y},	     
	{{121, 25, 14, 11 }, 24,61,42,44, AK_U},	      
	{{136, 25, 14, 11 }, 25,62,43,45, AK_I},	     
	{{151, 25, 14, 11 }, 26,63,44,46, AK_O},	     
	{{166, 25, 14, 11 }, 27,64,45,47, AK_P},	     
	{{181, 25, 14, 11 }, 28,65,46,48, AK_LBRACKET},	     
	{{196, 25, 14, 11 }, 29,49,47,49, AK_RBRACKET},	     
	{{211, 25, 29, 23 }, 30,81,48,50, AK_RET},	     
	{{241, 25, 14, 11 }, 32,66,49,51, AK_NP4},	     
	{{256, 25, 14, 11 }, 33,67,50,52, AK_NP5},	     
	{{271, 25, 14, 11 }, 34,68,51,53, AK_NP6},	     
	{{286, 25, 14, 11 }, 35,69,52,36, AK_NPADD},	     
	
	{{  1, 37, 29, 11 }, 36,70,69,55, AK_CTRL},                    
	{{ 31, 37, 14, 11 }, 37,70,54,56, AK_A},	     
	{{ 46, 37, 14, 11 }, 38,71,55,57, AK_S},	     
	{{ 61, 37, 14, 11 }, 39,72,56,58, AK_D},	     
	{{ 76, 37, 14, 11 }, 40,73,57,59, AK_F},	     
	{{ 91, 37, 14, 11 }, 41,74,58,60, AK_G},	     
	{{106, 37, 14, 11 }, 42,75,59,61, AK_H},	     
	{{121, 37, 14, 11 }, 43,76,60,62, AK_J},	      
	{{136, 37, 14, 11 }, 44,77,61,63, AK_K},	     
	{{151, 37, 14, 11 }, 45,78,62,64, AK_L},	     
	{{166, 37, 14, 11 }, 46,79,63,65, AK_SEMICOLON},	     
	{{181, 37, 14, 11 }, 47,80,64,49, AK_QUOTE},	     
	{{241, 37, 14, 11 }, 50,83,49,67, AK_NP1},	     
	{{256, 37, 14, 11 }, 51,83,66,68, AK_NP2},	     
	{{271, 37, 14, 11 }, 52,84,67,69, AK_NP3},	     
	{{286, 37, 14, 34 }, 53,16,68,54, AK_ENT},	     
	
	{{  1, 49, 44, 11 }, 54,85,84,71, AK_LSH},                    
	{{ 46, 49, 14, 11 }, 56,87,70,72, AK_Z},	     
	{{ 61, 49, 14, 11 }, 57,87,71,73, AK_X},	     
	{{ 76, 49, 14, 11 }, 58,87,72,74, AK_C},	     
	{{ 91, 49, 14, 11 }, 59,87,73,75, AK_V},	     
	{{106, 49, 14, 11 }, 60,87,74,76, AK_B},	     
	{{121, 49, 14, 11 }, 61,87,75,77, AK_N},	      
	{{136, 49, 14, 11 }, 62,87,76,78, AK_M},	     
	{{151, 49, 14, 11 }, 63,87,77,79, AK_COMMA},	     
	{{166, 49, 14, 11 }, 64,87,78,80, AK_PERIOD},	     
	{{181, 49, 14, 11 }, 65,88,79,81, AK_SLASH},	     
	{{196, 49, 29, 11 }, 49,89,80,82, AK_RSH},	     
	{{226, 49, 14, 11 }, 49,91,81,83, AK_UP},	     
	{{241, 49, 29, 11 }, 66,92,82,84, AK_NP0},	     
	{{271, 49, 14, 11 }, 67,69,83,69, AK_NPDEL},	     
	
	{{  1, 61, 29, 11 }, 70,0,69,86, AK_LALT},                    
	{{ 31, 61, 14, 11 }, 70,1,85,87, AK_LAMI},	     
	{{ 46, 61,134, 11 }, 71,2,86,88, AK_SPC},	     
	{{181, 61, 14, 11 }, 80,11,87,89, AK_RAMI},	     
	{{196, 61, 14, 11 }, 81,11,88,90, AK_RALT},	     
	{{211, 61, 14, 11 }, 81,12,89,91, AK_LF},	     
	{{226, 61, 14, 11 }, 82,12,90,92, AK_DN},	      
	{{241, 61, 14, 11 }, 83,13,91,69, AK_RT},	     
	{{0, 0, 0, 0 }, 0,0,0,0, 0},      
	{{0, 0, 0, 0 }, 0,0,0,0, 0},      
};

             
static t_vkbd_rect vkbd_rect_UK[]= 
{
	{{  1,  1, 29, 11 },85,17,16, 1, AK_ESC},                   
	{{ 31,  1, 14, 11 },86,18, 0, 2, AK_F1},	    
	{{ 46,  1, 14, 11 },87,19, 1, 3, AK_F2},	    
	{{ 61,  1, 14, 11 },87,20, 2, 4, AK_F3},	    
	{{ 76,  1, 14, 11 },87,21, 3, 5, AK_F4},	    
	{{ 91,  1, 14, 11 },87,22, 4, 6, AK_F5},	     
	{{106,  1, 14, 11 },87,23, 5, 7, AK_F6},	    
	{{121,  1, 14, 11 },87,24, 6, 8, AK_F7},	     
	{{136,  1, 14, 11 },87,25, 7, 9, AK_F8},	     
	{{151,  1, 14, 11 },87,26, 8,10, AK_F9},	    
	{{166,  1, 14, 11 },87,27, 9,11, AK_F10},	     
	{{181,  1, 29, 11 },88,28,10,12, AK_DEL},	     
	{{211,  1, 29, 11 },90,30,11,13, AK_HELP},	     
	{{241,  1, 14, 11 },92,32,12,14, AK_NPLPAREN},	     
	{{256,  1, 14, 11 },69,33,13,15, AK_NPRPAREN},	     
	{{271,  1, 14, 11 },69,34,14,16, AK_NPDIV},	     
	{{286,  1, 14, 11 },69,35,15,0, AK_NPMUL},	     
	
	{{  1, 13, 29, 11 }, 0,36,35,18, AK_BACKQUOTE},                    
	{{ 31, 13, 14, 11 }, 1,37,17,19, AK_1},	     
	{{ 46, 13, 14, 11 }, 2,38,18,20, AK_2},	     
	{{ 61, 13, 14, 11 }, 3,39,19,21, AK_3},	     
	{{ 76, 13, 14, 11 }, 4,40,20,22, AK_4},	     
	{{ 91, 13, 14, 11 }, 5,41,21,23, AK_5},	     
	{{106, 13, 14, 11 }, 6,42,22,24, AK_6},	     
	{{121, 13, 14, 11 }, 7,43,23,25, AK_7},	      
	{{136, 13, 14, 11 }, 8,44,24,26, AK_8},	     
	{{151, 13, 14, 11 }, 9,45,25,27, AK_9},	     
	{{166, 13, 14, 11 },10,46,26,28, AK_0},	     
	{{181, 13, 14, 11 },11,47,27,29, AK_MINUS},	     
	{{196, 13, 14, 11 },11,48,28,30, AK_EQUAL},	     
	{{211, 13, 14, 11 },12,49,29,31, AK_BACKSLASH},	     
	{{226, 13, 14, 11 },12,49,30,32, AK_BS},	     
	{{241, 13, 14, 11 },13,50,31,33, AK_NP7},	     
	{{256, 13, 14, 11 },14,51,32,34, AK_NP8},	     
	{{271, 13, 14, 11 },15,52,33,35, AK_NP9},	     
	{{286, 13, 14, 11 },16,53,34,17, AK_NPSUB},	     
	
	{{  1, 25, 29, 11 }, 17,54,53,37, AK_TAB},                    
	{{ 31, 25, 14, 11 }, 18,55,36,38, AK_Q},	     
	{{ 46, 25, 14, 11 }, 19,56,37,39, AK_W},	     
	{{ 61, 25, 14, 11 }, 20,57,38,40, AK_E},	     
	{{ 76, 25, 14, 11 }, 21,58,39,41, AK_R},	     
	{{ 91, 25, 14, 11 }, 22,59,40,42, AK_T},	     
	{{106, 25, 14, 11 }, 23,60,41,43, AK_Y},	     
	{{121, 25, 14, 11 }, 24,61,42,44, AK_U},	      
	{{136, 25, 14, 11 }, 25,62,43,45, AK_I},	     
	{{151, 25, 14, 11 }, 26,63,44,46, AK_O},	     
	{{166, 25, 14, 11 }, 27,64,45,47, AK_P},	     
	{{181, 25, 14, 11 }, 28,65,46,48, AK_LBRACKET},	     
	{{196, 25, 14, 11 }, 29,49,47,49, AK_RBRACKET},	     
	{{211, 25, 29, 23 }, 30,81,48,50, AK_RET},	     
	{{241, 25, 14, 11 }, 32,66,49,51, AK_NP4},	     
	{{256, 25, 14, 11 }, 33,67,50,52, AK_NP5},	     
	{{271, 25, 14, 11 }, 34,68,51,53, AK_NP6},	     
	{{286, 25, 14, 11 }, 35,69,52,36, AK_NPADD},	     
	
	{{  1, 37, 29, 11 }, 36,70,69,55, AK_CTRL},                    
	{{ 31, 37, 14, 11 }, 37,93,54,56, AK_A},	       
	{{ 46, 37, 14, 11 }, 38,71,55,57, AK_S},	     
	{{ 61, 37, 14, 11 }, 39,72,56,58, AK_D},	     
	{{ 76, 37, 14, 11 }, 40,73,57,59, AK_F},	     
	{{ 91, 37, 14, 11 }, 41,74,58,60, AK_G},	     
	{{106, 37, 14, 11 }, 42,75,59,61, AK_H},	     
	{{121, 37, 14, 11 }, 43,76,60,62, AK_J},	      
	{{136, 37, 14, 11 }, 44,77,61,63, AK_K},	     
	{{151, 37, 14, 11 }, 45,78,62,64, AK_L},	     
	{{166, 37, 14, 11 }, 46,79,63,65, AK_SEMICOLON},	     
	{{181, 37, 14, 11 }, 47,80,64,49, AK_QUOTE},	     
	{{241, 37, 14, 11 }, 50,83,49,67, AK_NP1},	     
	{{256, 37, 14, 11 }, 51,83,66,68, AK_NP2},	     
	{{271, 37, 14, 11 }, 52,84,67,69, AK_NP3},	     
	{{286, 37, 14, 34 }, 53,16,68,54, AK_ENT},	     
	
	{{  1, 49, 29, 11 }, 54,85,84,93, AK_LSH},                      
	{{ 46, 49, 14, 11 }, 56,87,93,72, AK_Z},	       
	{{ 61, 49, 14, 11 }, 57,87,71,73, AK_X},	     
	{{ 76, 49, 14, 11 }, 58,87,72,74, AK_C},	     
	{{ 91, 49, 14, 11 }, 59,87,73,75, AK_V},	     
	{{106, 49, 14, 11 }, 60,87,74,76, AK_B},	     
	{{121, 49, 14, 11 }, 61,87,75,77, AK_N},	      
	{{136, 49, 14, 11 }, 62,87,76,78, AK_M},	     
	{{151, 49, 14, 11 }, 63,87,77,79, AK_COMMA},	     
	{{166, 49, 14, 11 }, 64,87,78,80, AK_PERIOD},	     
	{{181, 49, 14, 11 }, 65,88,79,81, AK_SLASH},	     
	{{196, 49, 29, 11 }, 49,89,80,82, AK_RSH},	     
	{{226, 49, 14, 11 }, 49,91,81,83, AK_UP},	     
	{{241, 49, 29, 11 }, 66,92,82,84, AK_NP0},	     
	{{271, 49, 14, 11 }, 67,69,83,69, AK_NPDEL},	     
	
	{{  1, 61, 29, 11 }, 70,0,69,86, AK_LALT},                    
	{{ 31, 61, 14, 11 }, 93,1,85,87, AK_LAMI},	       
	{{ 46, 61,134, 11 }, 71,2,86,88, AK_SPC},	     
	{{181, 61, 14, 11 }, 80,11,87,89, AK_RAMI},	     
	{{196, 61, 14, 11 }, 81,11,88,90, AK_RALT},	     
	{{211, 61, 14, 11 }, 81,12,89,91, AK_LF},	     
	{{226, 61, 14, 11 }, 82,12,90,92, AK_DN},	      
	{{241, 61, 14, 11 }, 83,13,91,69, AK_RT},	     
	                
	{{31, 49, 14, 11 }, 55,86,70,71, AK_LTGT},	       
	{{0, 0, 0, 0 }, 0,0,0,0, 0},      
};

                 
static t_vkbd_rect vkbd_rect_GER[]= 
{
	{{  1,  1, 29, 11 },85,17,16, 1, AK_ESC},                   
	{{ 31,  1, 14, 11 },86,18, 0, 2, AK_F1},	    
	{{ 46,  1, 14, 11 },87,19, 1, 3, AK_F2},	    
	{{ 61,  1, 14, 11 },87,20, 2, 4, AK_F3},	    
	{{ 76,  1, 14, 11 },87,21, 3, 5, AK_F4},	    
	{{ 91,  1, 14, 11 },87,22, 4, 6, AK_F5},	     
	{{106,  1, 14, 11 },87,23, 5, 7, AK_F6},	    
	{{121,  1, 14, 11 },87,24, 6, 8, AK_F7},	     
	{{136,  1, 14, 11 },87,25, 7, 9, AK_F8},	     
	{{151,  1, 14, 11 },87,26, 8,10, AK_F9},	    
	{{166,  1, 14, 11 },87,27, 9,11, AK_F10},	     
	{{181,  1, 29, 11 },88,28,10,12, AK_DEL},	     
	{{211,  1, 29, 11 },90,30,11,13, AK_HELP},	     
	{{241,  1, 14, 11 },92,32,12,14, AK_NPLPAREN},	     
	{{256,  1, 14, 11 },69,33,13,15, AK_NPRPAREN},	     
	{{271,  1, 14, 11 },69,34,14,16, AK_NPDIV},	     
	{{286,  1, 14, 11 },69,35,15,0, AK_NPMUL},	     
	
	{{  1, 13, 29, 11 }, 0,36,35,18, AK_BACKQUOTE},                    
	{{ 31, 13, 14, 11 }, 1,37,17,19, AK_1},	     
	{{ 46, 13, 14, 11 }, 2,38,18,20, AK_2},	     
	{{ 61, 13, 14, 11 }, 3,39,19,21, AK_3},	     
	{{ 76, 13, 14, 11 }, 4,40,20,22, AK_4},	     
	{{ 91, 13, 14, 11 }, 5,41,21,23, AK_5},	     
	{{106, 13, 14, 11 }, 6,42,22,24, AK_6},	     
	{{121, 13, 14, 11 }, 7,43,23,25, AK_7},	      
	{{136, 13, 14, 11 }, 8,44,24,26, AK_8},	     
	{{151, 13, 14, 11 }, 9,45,25,27, AK_9},	     
	{{166, 13, 14, 11 },10,46,26,28, AK_0},	     
	{{181, 13, 14, 11 },11,47,27,29, AK_MINUS},	     
	{{196, 13, 14, 11 },11,48,28,30, AK_EQUAL},	     
	{{211, 13, 14, 11 },12,49,29,31, AK_BACKSLASH},	     
	{{226, 13, 14, 11 },12,49,30,32, AK_BS},	     
	{{241, 13, 14, 11 },13,50,31,33, AK_NP7},	     
	{{256, 13, 14, 11 },14,51,32,34, AK_NP8},	     
	{{271, 13, 14, 11 },15,52,33,35, AK_NP9},	     
	{{286, 13, 14, 11 },16,53,34,17, AK_NPSUB},	     
	
	{{  1, 25, 29, 11 }, 17,54,53,37, AK_TAB},                    
	{{ 31, 25, 14, 11 }, 18,55,36,38, AK_Q},	     
	{{ 46, 25, 14, 11 }, 19,56,37,39, AK_W},	     
	{{ 61, 25, 14, 11 }, 20,57,38,40, AK_E},	     
	{{ 76, 25, 14, 11 }, 21,58,39,41, AK_R},	     
	{{ 91, 25, 14, 11 }, 22,59,40,42, AK_T},	     
	{{106, 25, 14, 11 }, 23,60,41,43, AK_Y},	     
	{{121, 25, 14, 11 }, 24,61,42,44, AK_U},	      
	{{136, 25, 14, 11 }, 25,62,43,45, AK_I},	     
	{{151, 25, 14, 11 }, 26,63,44,46, AK_O},	     
	{{166, 25, 14, 11 }, 27,64,45,47, AK_P},	     
	{{181, 25, 14, 11 }, 28,65,46,48, AK_LBRACKET},	     
	{{196, 25, 14, 11 }, 29,94,47,49, AK_RBRACKET},	       
	{{211, 25, 29, 23 }, 30,81,48,50, AK_RET},	     
	{{241, 25, 14, 11 }, 32,66,49,51, AK_NP4},	     
	{{256, 25, 14, 11 }, 33,67,50,52, AK_NP5},	     
	{{271, 25, 14, 11 }, 34,68,51,53, AK_NP6},	     
	{{286, 25, 14, 11 }, 35,69,52,36, AK_NPADD},	     
	
	{{  1, 37, 29, 11 }, 36,70,69,55, AK_CTRL},                    
	{{ 31, 37, 14, 11 }, 37,93,54,56, AK_A},	       
	{{ 46, 37, 14, 11 }, 38,71,55,57, AK_S},	     
	{{ 61, 37, 14, 11 }, 39,72,56,58, AK_D},	     
	{{ 76, 37, 14, 11 }, 40,73,57,59, AK_F},	     
	{{ 91, 37, 14, 11 }, 41,74,58,60, AK_G},	     
	{{106, 37, 14, 11 }, 42,75,59,61, AK_H},	     
	{{121, 37, 14, 11 }, 43,76,60,62, AK_J},	      
	{{136, 37, 14, 11 }, 44,77,61,63, AK_K},	     
	{{151, 37, 14, 11 }, 45,78,62,64, AK_L},	     
	{{166, 37, 14, 11 }, 46,79,63,65, AK_SEMICOLON},	     
	{{181, 37, 14, 11 }, 47,80,64,94, AK_QUOTE},	       
	{{241, 37, 14, 11 }, 50,83,49,67, AK_NP1},	     
	{{256, 37, 14, 11 }, 51,83,66,68, AK_NP2},	     
	{{271, 37, 14, 11 }, 52,84,67,69, AK_NP3},	     
	{{286, 37, 14, 34 }, 53,16,68,54, AK_ENT},	     
	
	{{  1, 49, 29, 11 }, 54,85,84,93, AK_LSH},                      
	{{ 46, 49, 14, 11 }, 56,87,93,72, AK_Z},	       
	{{ 61, 49, 14, 11 }, 57,87,71,73, AK_X},	     
	{{ 76, 49, 14, 11 }, 58,87,72,74, AK_C},	     
	{{ 91, 49, 14, 11 }, 59,87,73,75, AK_V},	     
	{{106, 49, 14, 11 }, 60,87,74,76, AK_B},	     
	{{121, 49, 14, 11 }, 61,87,75,77, AK_N},	      
	{{136, 49, 14, 11 }, 62,87,76,78, AK_M},	     
	{{151, 49, 14, 11 }, 63,87,77,79, AK_COMMA},	     
	{{166, 49, 14, 11 }, 64,87,78,80, AK_PERIOD},	     
	{{181, 49, 14, 11 }, 65,88,79,81, AK_SLASH},	     
	{{196, 49, 29, 11 }, 49,89,80,82, AK_RSH},	     
	{{226, 49, 14, 11 }, 49,91,81,83, AK_UP},	     
	{{241, 49, 29, 11 }, 66,92,82,84, AK_NP0},	     
	{{271, 49, 14, 11 }, 67,69,83,69, AK_NPDEL},	     
	
	{{  1, 61, 29, 11 }, 70,0,69,86, AK_LALT},                    
	{{ 31, 61, 14, 11 }, 93,1,85,87, AK_LAMI},	       
	{{ 46, 61,134, 11 }, 71,2,86,88, AK_SPC},	     
	{{181, 61, 14, 11 }, 80,11,87,89, AK_RAMI},	     
	{{196, 61, 14, 11 }, 81,11,88,90, AK_RALT},	     
	{{211, 61, 14, 11 }, 81,12,89,91, AK_LF},	     
	{{226, 61, 14, 11 }, 82,12,90,92, AK_DN},	      
	{{241, 61, 14, 11 }, 83,13,91,69, AK_RT},	      
	                   
	{{31, 49, 14, 11 }, 55,86,70,71, AK_LTGT},	       
	{{196, 37, 14, 11 }, 48,81,65,49, AK_NUMBERSIGN},         
};

                 
static t_vkbd_rect vkbd_rect_FR[]= 
{
	{{  1,  1, 29, 11 },85,17,16, 1, AK_ESC},                   
	{{ 31,  1, 14, 11 },86,18, 0, 2, AK_F1},	    
	{{ 46,  1, 14, 11 },87,19, 1, 3, AK_F2},	    
	{{ 61,  1, 14, 11 },87,20, 2, 4, AK_F3},	    
	{{ 76,  1, 14, 11 },87,21, 3, 5, AK_F4},	    
	{{ 91,  1, 14, 11 },87,22, 4, 6, AK_F5},	     
	{{106,  1, 14, 11 },87,23, 5, 7, AK_F6},	    
	{{121,  1, 14, 11 },87,24, 6, 8, AK_F7},	     
	{{136,  1, 14, 11 },87,25, 7, 9, AK_F8},	     
	{{151,  1, 14, 11 },87,26, 8,10, AK_F9},	    
	{{166,  1, 14, 11 },87,27, 9,11, AK_F10},	     
	{{181,  1, 29, 11 },88,28,10,12, AK_DEL},	     
	{{211,  1, 29, 11 },90,30,11,13, AK_HELP},	     
	{{241,  1, 14, 11 },92,32,12,14, AK_NPLPAREN},	     
	{{256,  1, 14, 11 },69,33,13,15, AK_NPRPAREN},	     
	{{271,  1, 14, 11 },69,34,14,16, AK_NPDIV},	     
	{{286,  1, 14, 11 },69,35,15,0, AK_NPMUL},	     
	
	{{  1, 13, 29, 11 }, 0,36,35,18, AK_BACKQUOTE},                    
	{{ 31, 13, 14, 11 }, 1,37,17,19, AK_1},	     
	{{ 46, 13, 14, 11 }, 2,38,18,20, AK_2},	     
	{{ 61, 13, 14, 11 }, 3,39,19,21, AK_3},	     
	{{ 76, 13, 14, 11 }, 4,40,20,22, AK_4},	     
	{{ 91, 13, 14, 11 }, 5,41,21,23, AK_5},	     
	{{106, 13, 14, 11 }, 6,42,22,24, AK_6},	     
	{{121, 13, 14, 11 }, 7,43,23,25, AK_7},	      
	{{136, 13, 14, 11 }, 8,44,24,26, AK_8},	     
	{{151, 13, 14, 11 }, 9,45,25,27, AK_9},	     
	{{166, 13, 14, 11 },10,46,26,28, AK_0},	     
	{{181, 13, 14, 11 },11,47,27,29, AK_MINUS},	     
	{{196, 13, 14, 11 },11,48,28,30, AK_EQUAL},	     
	{{211, 13, 14, 11 },12,49,29,31, AK_BACKSLASH},	     
	{{226, 13, 14, 11 },12,49,30,32, AK_BS},	     
	{{241, 13, 14, 11 },13,50,31,33, AK_NP7},	     
	{{256, 13, 14, 11 },14,51,32,34, AK_NP8},	     
	{{271, 13, 14, 11 },15,52,33,35, AK_NP9},	     
	{{286, 13, 14, 11 },16,53,34,17, AK_NPSUB},	     
	
	{{  1, 25, 29, 11 }, 17,54,53,37, AK_TAB},                    
	{{ 31, 25, 14, 11 }, 18,55,36,38, AK_Q},	     
	{{ 46, 25, 14, 11 }, 19,56,37,39, AK_W},	     
	{{ 61, 25, 14, 11 }, 20,57,38,40, AK_E},	     
	{{ 76, 25, 14, 11 }, 21,58,39,41, AK_R},	     
	{{ 91, 25, 14, 11 }, 22,59,40,42, AK_T},	     
	{{106, 25, 14, 11 }, 23,60,41,43, AK_Y},	     
	{{121, 25, 14, 11 }, 24,61,42,44, AK_U},	      
	{{136, 25, 14, 11 }, 25,62,43,45, AK_I},	     
	{{151, 25, 14, 11 }, 26,63,44,46, AK_O},	     
	{{166, 25, 14, 11 }, 27,64,45,47, AK_P},	     
	{{181, 25, 14, 11 }, 28,65,46,48, AK_LBRACKET},	     
	{{196, 25, 14, 11 }, 29,94,47,49, AK_RBRACKET},	       
	{{211, 25, 29, 23 }, 30,81,48,50, AK_RET},	     
	{{241, 25, 14, 11 }, 32,66,49,51, AK_NP4},	     
	{{256, 25, 14, 11 }, 33,67,50,52, AK_NP5},	     
	{{271, 25, 14, 11 }, 34,68,51,53, AK_NP6},	     
	{{286, 25, 14, 11 }, 35,69,52,36, AK_NPADD},	     
	
	{{  1, 37, 29, 11 }, 36,70,69,55, AK_CTRL},                    
	{{ 31, 37, 14, 11 }, 37,93,54,56, AK_A},	       
	{{ 46, 37, 14, 11 }, 38,71,55,57, AK_S},	     
	{{ 61, 37, 14, 11 }, 39,72,56,58, AK_D},	     
	{{ 76, 37, 14, 11 }, 40,73,57,59, AK_F},	     
	{{ 91, 37, 14, 11 }, 41,74,58,60, AK_G},	     
	{{106, 37, 14, 11 }, 42,75,59,61, AK_H},	     
	{{121, 37, 14, 11 }, 43,76,60,62, AK_J},	      
	{{136, 37, 14, 11 }, 44,77,61,63, AK_K},	     
	{{151, 37, 14, 11 }, 45,78,62,64, AK_L},	     
	{{166, 37, 14, 11 }, 46,79,63,65, AK_SEMICOLON},	     
	{{181, 37, 14, 11 }, 47,80,64,94, AK_QUOTE},	       
	{{241, 37, 14, 11 }, 50,83,49,67, AK_NP1},	     
	{{256, 37, 14, 11 }, 51,83,66,68, AK_NP2},	     
	{{271, 37, 14, 11 }, 52,84,67,69, AK_NP3},	     
	{{286, 37, 14, 34 }, 53,16,68,54, AK_ENT},	     
	
	{{  1, 49, 29, 11 }, 54,85,84,93, AK_LSH},                      
	{{ 46, 49, 14, 11 }, 56,87,93,72, AK_Z},	       
	{{ 61, 49, 14, 11 }, 57,87,71,73, AK_X},	     
	{{ 76, 49, 14, 11 }, 58,87,72,74, AK_C},	     
	{{ 91, 49, 14, 11 }, 59,87,73,75, AK_V},	     
	{{106, 49, 14, 11 }, 60,87,74,76, AK_B},	     
	{{121, 49, 14, 11 }, 61,87,75,77, AK_N},	      
	{{136, 49, 14, 11 }, 62,87,76,78, AK_M},	     
	{{151, 49, 14, 11 }, 63,87,77,79, AK_COMMA},	     
	{{166, 49, 14, 11 }, 64,87,78,80, AK_PERIOD},	     
	{{181, 49, 14, 11 }, 65,88,79,81, AK_SLASH},	     
	{{196, 49, 29, 11 }, 49,89,80,82, AK_RSH},	     
	{{226, 49, 14, 11 }, 49,91,81,83, AK_UP},	     
	{{241, 49, 29, 11 }, 66,92,82,84, AK_NP0},	     
	{{271, 49, 14, 11 }, 67,69,83,69, AK_NPDEL},	     
	
	{{  1, 61, 29, 11 }, 70,0,69,86, AK_LALT},                    
	{{ 31, 61, 14, 11 }, 93,1,85,87, AK_LAMI},	       
	{{ 46, 61,134, 11 }, 71,2,86,88, AK_SPC},	     
	{{181, 61, 14, 11 }, 80,11,87,89, AK_RAMI},	     
	{{196, 61, 14, 11 }, 81,11,88,90, AK_RALT},	     
	{{211, 61, 14, 11 }, 81,12,89,91, AK_LF},	     
	{{226, 61, 14, 11 }, 82,12,90,92, AK_DN},	      
	{{241, 61, 14, 11 }, 83,13,91,69, AK_RT},	      
	                   
	{{31, 49, 14, 11 }, 55,86,70,71, AK_LTGT},	       
	{{196, 37, 14, 11 }, 48,81,65,49, AK_NUMBERSIGN},         
};

void vkbd_init_button2(void)
{
	vkbd_button2=(SDLKey)0;
}

void vkbd_reset_sticky_keys(void) 
{
	for (int i=0; i<NUM_STICKY; i++)
	{
		vkbd_sticky_key[i].can_switch=true;
		vkbd_sticky_key[i].stuck=false;
	}
}

int vkbd_init(void)
{
	int i;
	char tmpchar[256];
	char tmpchar2[256];
	char vkbdFileName[256];
	char vkbdHiresFileName[256];
	char vkbdShiftFileName[256];
	char vkbdShiftHiresFileName[256];
	char vkdbStyleString[10];
	char vkbdLanguageString[10];
	switch (mainMenu_vkbdStyle) {
		case 1:
			snprintf(vkdbStyleString, 10, "Warm");
			break;
		case 2:
			snprintf(vkdbStyleString, 10, "Cool");
			break;
		case 3:
			snprintf(vkdbStyleString, 10, "Dark");
			break;
		default:
			snprintf(vkdbStyleString, 10, "Orig");
			break;
	}
	switch (mainMenu_vkbdLanguage) {
		case 1:
			snprintf(vkbdLanguageString, 10, "UK");
			vkbd_rect=vkbd_rect_UK;
			break;
		case 2:
			snprintf(vkbdLanguageString, 10, "Ger");
			vkbd_rect=vkbd_rect_GER;
			break;
		case 3:
			snprintf(vkbdLanguageString, 10, "FR");
			vkbd_rect=vkbd_rect_FR;
			break;
		default:
			snprintf(vkbdLanguageString, 10, "US");
			vkbd_rect=vkbd_rect_US;
			break;
	}
	snprintf(vkbdFileName, 256, "vkbd%s%sLarge.bmp", vkdbStyleString, vkbdLanguageString );
	snprintf(vkbdHiresFileName, 256, "vkbd%s%sLargeHires.bmp", vkdbStyleString, vkbdLanguageString);
	snprintf(vkbdShiftFileName, 256, "vkbd%s%sLargeShift.bmp", vkdbStyleString, vkbdLanguageString);
	snprintf(vkbdShiftHiresFileName, 256, "vkbd%s%sLargeShiftHires.bmp", vkdbStyleString, vkbdLanguageString);

#if defined(__PSP2__) || defined(__SWITCH__)
	snprintf(tmpchar, 256, "%s%s", DATA_PREFIX, vkbdFileName);
	snprintf(tmpchar2, 256, "%s%s", DATA_PREFIX, vkbdHiresFileName);
#else
#ifdef GP2X
	snprintf(tmpchar, 256, "%s/data/%s", launchDir, vkbdFileName);
	snprintf(tmpchar2, 256, "%s/data/%s", launchDir, vkbdHiresFileName);
#else
#ifdef GIZMONDO
	snprintf(tmpchar, 256, "%s", "\\SD Card\\uae4all\\data\\%s",vkbdFileName);
	snprintf(tmpchar2, 256, "%s", "\\SD Card\\uae4all\\data\\%s",vkbdHiresFileName);
#else
	snprintf(tmpchar, 256, "%s%s", DATA_PREFIX, vkbdFileName);
	snprintf(tmpchar2, 256, "%s%s", DATA_PREFIX, vkbdHiresFileName);
#endif
#endif
#endif           

	SDL_Surface *tmp = SDL_LoadBMP(tmpchar);

	if (tmp==NULL)
	{
		printf("Virtual Keyboard Bitmap Error: %s\n",SDL_GetError());
		return -1;
	}
	ksur=SDL_DisplayFormat(tmp);
	SDL_FreeSurface(tmp);

	tmp = SDL_LoadBMP(tmpchar2);
	
	if (tmp==NULL)
	{
		printf("Virtual Keyboard Bitmap Error: %s\n",SDL_GetError());
		return -1;
	}
	ksurHires=SDL_DisplayFormat(tmp);
	SDL_FreeSurface(tmp);

	                                                    
	canvas=SDL_DisplayFormat(ksur);
	canvasHires=SDL_DisplayFormat(ksurHires);

	                                                                               
#if defined(__PSP2__) || defined(__SWITCH__)
	snprintf(tmpchar, 256, "%s%s", DATA_PREFIX, vkbdShiftFileName);
	snprintf(tmpchar2, 256, "%s%s", DATA_PREFIX, vkbdShiftHiresFileName);
#else
#ifdef GP2X
	snprintf(tmpchar, 256, "%s/data/%s", launchDir, vkbdShiftFileName);
	snprintf(tmpchar2, 256, "%s/data/%s", launchDir, vkbdShiftHiresFileName);
#else
#ifdef GIZMONDO
	snprintf(tmpchar, 256, "%s", "\\SD Card\\uae4all\\data\\%s",vkbdShiftFileName);
	snprintf(tmpchar2, 256, "%s", "\\SD Card\\uae4all\\data\\%s",vkbdShiftHiresFileName);
#else
	snprintf(tmpchar, 256, "%s%s", DATA_PREFIX, vkbdShiftFileName);
	snprintf(tmpchar2, 256, "%s%s", DATA_PREFIX, vkbdShiftHiresFileName);
#endif
#endif
#endif           
	
	tmp = SDL_LoadBMP(tmpchar);

	if (tmp==NULL)
	{
		printf("Virtual Keyboard Bitmap Error: %s\n",SDL_GetError());
		return -1;
	}
	ksurShift=SDL_DisplayFormat(tmp);
	SDL_FreeSurface(tmp);
	
	tmp = SDL_LoadBMP(tmpchar2);

	if (tmp==NULL)
	{
		printf("Virtual Keyboard Bitmap Error: %s\n",SDL_GetError());
		return -1;
	}
	ksurShiftHires=SDL_DisplayFormat(tmp);
	SDL_FreeSurface(tmp);

	vkbd_transparency=128;                                           
	SDL_SetAlpha(canvas, SDL_SRCALPHA | SDL_RLEACCEL, vkbd_transparency);
	SDL_SetAlpha(canvasHires, SDL_SRCALPHA | SDL_RLEACCEL, vkbd_transparency);

	vkbd_actual=0;
#if !defined(__PSP2__) && !defined(__SWITCH__)                                            
	vkbd_redraw();
#endif
	for (int i=0; i<NUM_STICKY; i++)
	{
		vkbd_sticky_key[i].stuck=false;
	}
	vkbd_x=(prSDLScreen->w-ksur->w)/2;
	vkbd_y=prSDLScreen->h-ksur->h;
	vkbd_mode=0;
	vkbd_move=0;
	vkbd_last_press_time=0;
	vkbd_last_move_time=0;
	vkbd_key=KEYCODE_NOTHING;
	vkbd_button2=(SDLKey)0;
	vkbd_keysave=KEYCODE_NOTHING;
	return 0;
}

void vkbd_quit(void)
{
	int i;
	SDL_FreeSurface(ksurShift);
	SDL_FreeSurface(ksurShiftHires);
	SDL_FreeSurface(ksur);
	SDL_FreeSurface(ksurHires);
	vkbd_mode=0;
	for (int i=0; i<NUM_STICKY; i++)
	{
		vkbd_sticky_key[i].stuck=false;
	}
}

void vkbd_redraw(void)
{
	SDL_Rect r;
	SDL_Surface *toDraw;
	SDL_Surface *myCanvas;
	if (mainMenu_displayHires)
	{
		if (vkbd_sticky_key[0].stuck || vkbd_sticky_key[1].stuck)			
			toDraw=ksurShiftHires;
		else
			toDraw=ksurHires;
			myCanvas=canvasHires;
	}
	else
	{
		if (vkbd_sticky_key[0].stuck || vkbd_sticky_key[1].stuck)
			toDraw=ksurShift;
		else
			toDraw=ksur;
			myCanvas=canvas;
	}

	                                
	r.x=0;
	r.y=0;
	r.w=toDraw->w;
	r.h=toDraw->h;
	SDL_BlitSurface(toDraw,NULL,myCanvas,&r);

	                                                          
	                                                                   
	Uint32 sticky_key_color=SDL_MapRGB(myCanvas->format, 0, 255, 0);
	for (int i=0; i<NUM_STICKY; i++) {
		if (vkbd_sticky_key[i].stuck==true) {
			int index = vkbd_sticky_key[i].index;
			if (mainMenu_displayHires)
			{
				r.x=2*vkbd_rect[index].rect.x+2;
				r.w=6;
			}
			else
			{
				r.x=vkbd_rect[index].rect.x+1;
				r.w=3;
			}
			r.y=vkbd_rect[index].rect.y+1;
			r.h=3;
			SDL_FillRect(myCanvas,&r,sticky_key_color);
		}
	}

	if (vkbd_y>prSDLScreen->h-myCanvas->h) 
		vkbd_y=prSDLScreen->h-myCanvas->h;
		
	vkbd_x=(prSDLScreen->w-myCanvas->w)/2;
	
	r.x=vkbd_x;	
	r.y=vkbd_y;	
	r.w=myCanvas->w;
	r.h=myCanvas->h;

	SDL_BlitSurface(myCanvas,NULL,prSDLScreen,&r);
}

void vkbd_transparency_up(void)
{
	switch (vkbd_transparency) 
	{
		case 255:
			vkbd_transparency=192;
			break;
		case 192:
			vkbd_transparency=128;
			break;
		case 128: 
			vkbd_transparency=64;
			break;
		case 64: 
			vkbd_transparency=255;
			break;
		default:
			vkbd_transparency=64;
			break;
	}		
	if (vkbd_transparency != 255)
	{
		SDL_SetAlpha(canvas, SDL_SRCALPHA | SDL_RLEACCEL, vkbd_transparency);		
		SDL_SetAlpha(canvasHires, SDL_SRCALPHA | SDL_RLEACCEL, vkbd_transparency);		
	}
	else               
	{
	 	SDL_SetAlpha(canvas, 0, 255);
	 	SDL_SetAlpha(canvasHires, 0, 255);
	}
}	

void vkbd_transparency_down(void)
{
	switch (vkbd_transparency) 
	{
		case 255:
			vkbd_transparency=64;
			break;
		case 192:
			vkbd_transparency=255;
			break;
		case 128: 
			vkbd_transparency=192;
			break;
	 	case 64:
			vkbd_transparency=128;
			break;
		default:
			vkbd_transparency=255;
			break;
	}		
	if (vkbd_transparency != 255)
	{
		SDL_SetAlpha(canvas, SDL_SRCALPHA | SDL_RLEACCEL, vkbd_transparency);		
		SDL_SetAlpha(canvasHires, SDL_SRCALPHA | SDL_RLEACCEL, vkbd_transparency);		
	}
	else               
	{
	 	SDL_SetAlpha(canvas, 0, 255);
	 	SDL_SetAlpha(canvasHires, 0, 255);				
	}
}	


void vkbd_displace_up(void)
{
	if (vkbd_y>3)
		vkbd_y-=4;
	else
		vkbd_y=0;
}

void vkbd_displace_down(void)
{
	if (vkbd_y<prSDLScreen->h-ksur->h-3)
		vkbd_y+=4;
	else
		vkbd_y=prSDLScreen->h-ksur->h;
}		

int vkbd_touch_xy_to_actual(float touch_x, float touch_y)
{
	int x = 0;
	int y = 0;
	int x_offset = 0;
	int y_offset = 0;
	float scaled_width = 0;
	float scaled_height = 0;
#ifdef __PSP2__
	int display_width = 960;
	int display_height = 544;
	                                                                           
                                                                           
                                          
	int preset_variant = presetModeId % 10;
	if (preset_variant == 7)
	{
		scaled_width = (float)display_width;
		scaled_height = (float)display_height;
		x_offset = 0;
		y_offset = 0;
	}
	else if (mainMenu_shader != 0)
	{
		scaled_height = (float)display_height;
		scaled_width = scaled_height * (4.0f / 3.0f);
		x_offset = (display_width - scaled_width) / 2;
		y_offset = 0;
	}
	else
	{
		scaled_width = 720.0f;
		scaled_height = 540.0f;
		x_offset = (display_width - scaled_width) / 2;
		y_offset = (display_height - scaled_height) / 2;
	}
#else
	int display_width = 1280;
	int display_height = 720;
	if (mainMenu_shader == 0) {
		                  
		int screen_width;
		int screen_height;
		screen_width = visibleAreaWidth;
		if (mainMenu_displayHires)
			screen_height = 2 * mainMenu_displayedLines;			
		else
			screen_height = mainMenu_displayedLines;
		int scale_factor = MIN(display_height / screen_height, display_width / screen_width);
		scaled_height = scale_factor * screen_height;
		scaled_width = scale_factor * screen_width;
	} else {
		                                      
		scaled_height = display_height;
		if (mainMenu_displayHires)
				scaled_width = ((visibleAreaWidth * display_height) / (float) (2 * mainMenu_displayedLines));
		else
				scaled_width = ((visibleAreaWidth * display_height) / (float) (mainMenu_displayedLines));
	}
	            
	x_offset = (display_width - scaled_width) / 2;
	y_offset = (display_height - scaled_height) / 2;
#endif
	x = (((touch_x * display_width) - x_offset) * visibleAreaWidth) / scaled_width;
	y = (((touch_y * display_height) - y_offset) * mainMenu_displayedLines) / scaled_height;
	x -= vkbd_x;
	y -= vkbd_y;
	if (mainMenu_displayHires)
		x /= 2;

	for (int i = 0; i < 95; i++)
	{
		int x_min = vkbd_rect[i].rect.x;
		int y_min = vkbd_rect[i].rect.y;
		int x_max = x_min + vkbd_rect[i].rect.w;
		int y_max = y_min + vkbd_rect[i].rect.h;
		if (x_min != 0 && y_min != 0 && x >= x_min-1 && x <= x_max && y >= y_min-1 && y <= y_max)
			return i;
	}
	return -1;
}

int vkbd_process(void)
{
	Uint32 now=SDL_GetTicks();
	SDL_Rect r;
	
	vkbd_redraw();

	if (vkbd_touch_x != -1 && vkbd_touch_y != -1) {
		int new_vkbd_actual = vkbd_touch_xy_to_actual(vkbd_touch_x, vkbd_touch_y);
		if (new_vkbd_actual != -1)
		{
			vkbd_actual = new_vkbd_actual;
			vkbd_move = VKBD_BUTTON;
			                              
			if (mainMenu_displayHires)
			{
				r.x=vkbd_x+2*vkbd_rect[vkbd_actual].rect.x;
				r.w=2*vkbd_rect[vkbd_actual].rect.w;
			}
			else
			{
				r.x=vkbd_x+vkbd_rect[vkbd_actual].rect.x;
				r.w=vkbd_rect[vkbd_actual].rect.w;
			}
			r.y=vkbd_y+vkbd_rect[vkbd_actual].rect.y;
			r.h=vkbd_rect[vkbd_actual].rect.h;
			SDL_FillRect(prSDLScreen,&r,vkbd_color);
		}
	}

	if (vkbd_move&VKBD_BUTTON)
	{
		vkbd_move=0;
		int amigaKeyCode=vkbd_rect[vkbd_actual].key;
		                      
		for (int i=0; i<NUM_STICKY; i++) 
		{
			if (amigaKeyCode == vkbd_sticky_key[i].code) 
			{
				if (vkbd_sticky_key[i].can_switch) 
				{
					vkbd_sticky_key[i].stuck=!vkbd_sticky_key[i].stuck;
					vkbd_sticky_key[i].can_switch=false;
					return amigaKeyCode;
				}
				else return (KEYCODE_NOTHING);
			}
		}
		return amigaKeyCode;
	}
	
	if (vkbd_move&VKBD_BUTTON_BACKSPACE)
	{
		vkbd_move=0;
		return AK_BS;
	}
	if (vkbd_move&VKBD_BUTTON_SHIFT)
	{
		vkbd_move=0;
		                                                            
		if (vkbd_sticky_key[0].can_switch)
		{
			vkbd_sticky_key[0].stuck=!vkbd_sticky_key[0].stuck;
			vkbd_sticky_key[0].can_switch=false;
			return(AK_LSH);
		} else
			return(KEYCODE_NOTHING);
	}
	if (vkbd_move&VKBD_BUTTON_RESET_STICKY)
	{
		vkbd_move=0;
		                                          
		vkbd_reset_sticky_keys();
		return(KEYCODE_STICKY_RESET);                                               
	}
	if (vkbd_move&VKBD_LEFT || vkbd_move&VKBD_RIGHT || vkbd_move&VKBD_UP || vkbd_move&VKBD_DOWN) 
	{
		if (vkbd_let_go_of_direction)                     
			vkbd_last_press_time=now;
		if (
				(
				now-vkbd_last_press_time>VKBD_MIN_HOLDING_TIME 
				&& now-vkbd_last_move_time>VKBD_MOVE_DELAY
				) 
				|| vkbd_let_go_of_direction
			) 
		{
			vkbd_last_move_time=now;
			if (vkbd_move&VKBD_LEFT)
				vkbd_actual=vkbd_rect[vkbd_actual].left;
			else if (vkbd_move&VKBD_RIGHT)
				vkbd_actual=vkbd_rect[vkbd_actual].right;
			if (vkbd_move&VKBD_UP)
				vkbd_actual=vkbd_rect[vkbd_actual].up;
			else if (vkbd_move&VKBD_DOWN)
				vkbd_actual=vkbd_rect[vkbd_actual].down;
		}
		vkbd_let_go_of_direction=0;
	}
	else
		vkbd_let_go_of_direction=1;
		
	if (mainMenu_displayHires)
	{
		r.x=vkbd_x+2*vkbd_rect[vkbd_actual].rect.x;
		r.w=2*vkbd_rect[vkbd_actual].rect.w;
	}
	else
	{
		r.x=vkbd_x+vkbd_rect[vkbd_actual].rect.x;
		r.w=vkbd_rect[vkbd_actual].rect.w;
	}
	r.y=vkbd_y+vkbd_rect[vkbd_actual].rect.y;
	r.h=vkbd_rect[vkbd_actual].rect.h;
	if (!vkbd_just_blinked)
	{	
		SDL_FillRect(prSDLScreen,&r,vkbd_color);
		vkbd_just_blinked=1;
		                           
	}
	else
	{
		vkbd_just_blinked=0;
	}
	return KEYCODE_NOTHING;                                  
}
#endif
