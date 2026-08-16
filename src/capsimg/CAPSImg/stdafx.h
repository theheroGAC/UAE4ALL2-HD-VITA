                                                             
                                                                  
                           
  

#pragma once

                  
                                            

#define WIN32_LEAN_AND_MEAN                                                              

                   
                                           

                                      
#define _CRTDBG_MAP_ALLOC

                          
#include <stdlib.h>
                                          
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <vector>
                                      
                                          
#include <dirent.h>

                  
#include <stddef.h>			           
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>			            
#define MAX_PATH ( 260 )
#ifndef __cdecl
#define __cdecl
#endif
#define _lrotl(x,n) (((x) << (n)) | ((x) >> (sizeof(x)*8-(n))))
#define _lrotr(x,n) (((x) >> (n)) | ((x) << (sizeof(x)*8-(n))))
typedef const char *LPCSTR;
typedef const char *LPCTSTR;
                  


#define INTEL
#define MAX_FILENAMELEN (MAX_PATH*2)

                       
#include "CommonTypes.h"

                  
#include "BaseFile.h"
#include "DiskFile.h"
#include "MemoryFile.h"
#include "CRC.h"
#include "BitBuffer.h"

                                 
#include "CapsLibAll.h"

         
#include "DiskEncoding.h"
#include "CapsDefinitions.h"
#include "CTRawCodec.h"

               
#include "CapsFile.h"
#include "DiskImage.h"
#include "CapsLoader.h"
#include "CapsImageStd.h"
#include "CapsImage.h"
#include "StreamImage.h"
#include "StreamCueImage.h"
#include "DiskImageFactory.h"

                
#include "C2Comm.h"

         
#include "CapsCore.h"
#include "CapsFDCEmulator.h"
#include "CapsFormatMFM.h"


                  
#define _access access
#ifndef __MINGW32__
#define _mkdir(x) mkdir(x,0)
#else
#define _mkdir(x) mkdir(x)
#endif
#define d_namlen d_reclen
#define __assume(cond) do { if (!(cond)) __builtin_unreachable(); } while (0)
#define min(x, y) (((x) < (y)) ? (x) : (y))

typedef struct _SYSTEMTIME {
        WORD wYear;
        WORD wMonth;
        WORD wDayOfWeek;
        WORD wDay;
        WORD wHour;
        WORD wMinute;
        WORD wSecond;
        WORD wMilliseconds;
} SYSTEMTIME, *LPSYSTEMTIME;
extern "C" void GetLocalTime(LPSYSTEMTIME lpSystemTime);
                  


