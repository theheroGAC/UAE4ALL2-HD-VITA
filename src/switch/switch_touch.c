  
                                  

#if defined(__vita__)
#include <psp2/kernel/processmgr.h>
#define displayWidth 960.0
#define displayHeight 544.0
#endif

#if defined(__SWITCH__)
#define SCE_TOUCH_PORT_MAX_NUM 2
#define displayWidth 1280.0
#define displayHeight 720.0
#endif

#include "switch_touch.h"

#include "math.h"

typedef int bool;
#include "vkbd.h"

static void initTouch();
static void preprocessEvents(SDL_Event *event);
static void preprocessFingerDown(SDL_Event *event);
static void preprocessFingerUp(SDL_Event *event);
static void preprocessFingerMotion(SDL_Event *event);

static int psp2RearTouch = 0;                                     
extern int lastmx;
extern int lastmy;
static int touch_initialized = 0;
unsigned int _simulatedClickStartTime[SCE_TOUCH_PORT_MAX_NUM][2];                                                                            
static int hiresdx = 0;
static int hiresdy = 0;

enum {
	MAX_NUM_FINGERS = 3,                                        
	MAX_TAP_TIME = 250,                                                               
	MAX_TAP_MOTION_DISTANCE = 10,                                                                           
	SIMULATED_CLICK_DURATION = 50,                                                        
};                                 

typedef struct {
	int id;                
	Uint32 timeLastDown;
	int lastX;                                 
	int lastY;                                 
	float lastDownX;                                                
	float lastDownY;                                                
} Touch;

Touch _finger[SCE_TOUCH_PORT_MAX_NUM][MAX_NUM_FINGERS];                               

typedef enum DraggingType {
	DRAG_NONE = 0,
	DRAG_TWO_FINGER,
	DRAG_THREE_FINGER,
} DraggingType;

DraggingType _multiFingerDragging[SCE_TOUCH_PORT_MAX_NUM];                                                         

typedef int bool;
#define false 0;
#define true 1;

static void initTouch() {
	for (int port = 0; port < SCE_TOUCH_PORT_MAX_NUM; port++) {
		for (int i = 0; i < MAX_NUM_FINGERS; i++) {
			_finger[port][i].id = -1;
		}
		_multiFingerDragging[port] = DRAG_NONE;
	}

	for (int port = 0; port < SCE_TOUCH_PORT_MAX_NUM; port++) {
		for (int i = 0; i < 2; i++) {
			_simulatedClickStartTime[port][i] = 0;
		}
	}
}

void SWITCH_HandleTouch(SDL_Event *event) {
	if (!touch_initialized) {
		initTouch();
		touch_initialized = 1;
	}
	preprocessEvents(event);
}

static void preprocessEvents(SDL_Event *event) {

	                            
	                                            
	                                                                              
	                                     
	                                              
	                                                 
	if (event->type == SDL_FINGERDOWN || event->type == SDL_FINGERUP || event->type == SDL_FINGERMOTION) {
		                               
		SDL_TouchID port = event->tfinger.touchId;
		if (port < SCE_TOUCH_PORT_MAX_NUM && port >= 0) {
			if (port == 1) {
				switch (event->type) {
				case SDL_FINGERDOWN:
					preprocessFingerDown(event);
					break;
				case SDL_FINGERUP:
					preprocessFingerUp(event);
					break;
				case SDL_FINGERMOTION:
					preprocessFingerMotion(event);
					break;
				}
			}
		}
	}
}

static void preprocessFingerDown(SDL_Event *event) {
	                               
	SDL_TouchID port = event->tfinger.touchId;
	                      
	SDL_FingerID id = event->tfinger.fingerId;

	if (vkbd_mode) {
		if (port == 1) {
			vkbd_touch_x = event->tfinger.x;
			vkbd_touch_y = event->tfinger.y;
		}
		return;
	}

	int x = lastmx;
	int y = lastmy;

	                                                            
	for (int i = 0; i < MAX_NUM_FINGERS; i++) {
		if (_finger[port][i].id == id) {
			_finger[port][i].id = -1;
		}
	}

	                                                                                   
	                       
	                                                                              
	for (int i = 0; i < MAX_NUM_FINGERS; i++) {
		if (_finger[port][i].id == -1) {
			_finger[port][i].id = id;
			_finger[port][i].timeLastDown = event->tfinger.timestamp;
			_finger[port][i].lastDownX = event->tfinger.x;
			_finger[port][i].lastDownY = event->tfinger.y;
			_finger[port][i].lastX = x;
			_finger[port][i].lastY = y;
			break;
		}
	}
}

static void preprocessFingerUp(SDL_Event *event) {
	                              
	SDL_TouchID port = event->tfinger.touchId;
	                      
	SDL_FingerID id = event->tfinger.fingerId;

	if (vkbd_mode) {
		if (port == 1) {
			vkbd_touch_x = -1;
			vkbd_touch_y = -1;
			vkbd_move &= ~VKBD_BUTTON;
		}
		return;
	}

	                                                        
	int numFingersDown = 0;
	for (int i = 0; i < MAX_NUM_FINGERS; i++) {
		if (_finger[port][i].id >= 0) {
			numFingersDown++;
		}
	}

	int x = lastmx;
	int y = lastmy;

	for (int i = 0; i < MAX_NUM_FINGERS; i++) {
		if (_finger[port][i].id == id) {
			_finger[port][i].id = -1;
			if (!_multiFingerDragging[port]) {
				if ((event->tfinger.timestamp - _finger[port][i].timeLastDown) <= MAX_TAP_TIME) {
					                                                                                                            
					                                                                                                            
					float xrel = ((event->tfinger.x * displayWidth) - (_finger[port][i].lastDownX * displayWidth));
					float yrel = ((event->tfinger.y * displayHeight) - (_finger[port][i].lastDownY * displayHeight));
					float maxRSquared = (float) (MAX_TAP_MOTION_DISTANCE * MAX_TAP_MOTION_DISTANCE);
					if ((xrel * xrel + yrel * yrel) < maxRSquared) {
						if (numFingersDown == 2 || numFingersDown == 1) {
							Uint8 simulatedButton = 0;
							if (numFingersDown == 2) {
								simulatedButton = SDL_BUTTON_RIGHT;
								                                 
								_simulatedClickStartTime[port][1] = event->tfinger.timestamp;
							} else if (numFingersDown == 1) {
								simulatedButton = SDL_BUTTON_LEFT;
								                                 
								_simulatedClickStartTime[port][0] = event->tfinger.timestamp;
							}

							event->type = SDL_MOUSEBUTTONDOWN;
							event->button.button = simulatedButton;
							event->button.state = SDL_PRESSED;
							event->button.x = x;
							event->button.y = y;
						}
					}
				}
			} else if (numFingersDown == 1) {
				                                                                 
				Uint8 simulatedButton = 0;
				if (_multiFingerDragging[port] == DRAG_THREE_FINGER)
					simulatedButton = SDL_BUTTON_RIGHT;
				else {
					simulatedButton = SDL_BUTTON_LEFT;
				}
				event->type = SDL_MOUSEBUTTONUP;
				event->button.button = simulatedButton;
				event->button.state = SDL_RELEASED;
				event->button.x = x;
				event->button.y = y;
				_multiFingerDragging[port] = DRAG_NONE;
			}
		}
	}
}

static void preprocessFingerMotion(SDL_Event *event) {
	if (vkbd_mode)
		return;
	
	                               
	SDL_TouchID port = event->tfinger.touchId;
	                      
	SDL_FingerID id = event->tfinger.fingerId;

	                                                        
	int numFingersDown = 0;
	for (int i = 0; i < MAX_NUM_FINGERS; i++) {
		if (_finger[port][i].id >= 0) {
			numFingersDown++;
		}
	}

	if (numFingersDown >= 1) {
		int x = lastmx;
		int y = lastmy;
		int xrel = (int)(event->tfinger.dx * displayWidth);
		int yrel = (int)(event->tfinger.dy * displayHeight);

		x = lastmx + (int)(event->tfinger.dx * displayWidth);
		y = lastmy + (int)(event->tfinger.dy * displayHeight);

		                                                                   
		for (int i = 0; i < MAX_NUM_FINGERS; i++) {
			if (_finger[port][i].id == id) {
				_finger[port][i].lastX = x;
				_finger[port][i].lastY = y;
			}
		}

		                                                                              
		if (numFingersDown >= 2) {
			if (!_multiFingerDragging[port]) {
				                                                                                    
				int numFingersDownLong = 0;
				for (int i = 0; i < MAX_NUM_FINGERS; i++) {
					if (_finger[port][i].id >= 0) {
						if (event->tfinger.timestamp - _finger[port][i].timeLastDown > MAX_TAP_TIME) {
							numFingersDownLong++;
						}
					}
				}
				if (numFingersDownLong >= 2) {
					Uint8 simulatedButton = 0;
					if (numFingersDownLong == 2) {
						simulatedButton = SDL_BUTTON_LEFT;
						_multiFingerDragging[port] = DRAG_TWO_FINGER;
					} else {
						simulatedButton = SDL_BUTTON_RIGHT;
						_multiFingerDragging[port] = DRAG_THREE_FINGER;
					}
					int mouseDownX = lastmx;
					int mouseDownY = lastmy;
					SDL_Event ev;
					ev.type = SDL_MOUSEBUTTONDOWN;
					ev.button.button = simulatedButton;
					ev.button.state = SDL_PRESSED;
					ev.button.x = mouseDownX;
					ev.button.y = mouseDownY;
					SDL_PushEvent(&ev);
				}
			}
		}

		                                                                                                                
		bool updatePointer = true;
		if (numFingersDown > 1) {
			for (int i = 0; i < MAX_NUM_FINGERS; i++) {
				if (_finger[port][i].id == id) {
					for (int j = 0; j < MAX_NUM_FINGERS; j++) {
						if (_finger[port][j].id >= 0 && (i != j) ) {
							if (_finger[port][j].timeLastDown < _finger[port][i].timeLastDown) {
								updatePointer = false;
							}
						}
					}
				}
			}
		}
		if (updatePointer) {
			const int slowdown = 16;
			hiresdx += (int)(event->tfinger.dx * displayWidth * slowdown);
			hiresdy += (int)(event->tfinger.dy * displayHeight * slowdown);
			int xrel = hiresdx / slowdown;
			int yrel = hiresdy / slowdown;

			if (xrel || yrel) {
				int x = lastmx + xrel;
				int y = lastmy + yrel;
				event->type = SDL_MOUSEMOTION;
				event->motion.x = x;
				event->motion.y = y;
				event->motion.xrel = xrel;
				event->motion.yrel = yrel;
				hiresdx %= slowdown;
				hiresdy %= slowdown;
			}
		}
	}
}

void SWITCH_FinishSimulatedMouseClicks() {
	for (int port = 0; port < SCE_TOUCH_PORT_MAX_NUM; port++) {
		for (int i = 0; i < 2; i++) {
			if (_simulatedClickStartTime[port][i] != 0) {
				Uint32 currentTime = SDL_GetTicks();
				if (currentTime - _simulatedClickStartTime[port][i] >= SIMULATED_CLICK_DURATION) {
					int simulatedButton;
					if (i == 0) {
						simulatedButton = SDL_BUTTON_LEFT;
					} else {
						simulatedButton = SDL_BUTTON_RIGHT;
					}
					SDL_Event ev;
					ev.type = SDL_MOUSEBUTTONUP;
					ev.button.button = simulatedButton;
					ev.button.state = SDL_RELEASED;
					ev.button.x = lastmx;
					ev.button.y = lastmy;
					SDL_PushEvent(&ev);

					_simulatedClickStartTime[port][i] = 0;
				}
			}
		}
	}
}
