   
                                 
   
                                                                         
                                          
   
                                                                        
                                                                      
                                                                      
                                        
   
                                      
    

#ifndef SYSDEPS_H
#define SYSDEPS_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <limits.h>

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

#if defined(__PSP2__)
#define SDL_SetVideoModeScaling SDL_VITA_SetVideoModeScaling
#define SDL_SetVideoModeBilinear SDL_VITA_SetVideoModeBilinear
#define SDL_SetVideoModeSync SDL_VITA_SetVideoModeSync
#endif


#ifdef _GCCRES_
#undef _GCCRES_
#endif

#ifdef UAE4ALL_NO_USE_RESTRICT
#define _GCCRES_
#else
#define _GCCRES_ __restrict__
#endif

#ifndef __STDC__
#error "Your compiler is not ANSI. Get a real one."
#endif

#include <stdarg.h>

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_VALUES_H
#include <values.h>
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_UTIME_H
#include <utime.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#if HAVE_DIRENT_H
#if defined(__PSP2__)                  
# include "psp2-dirent.h"
#else
# include <dirent.h>
#endif
#else
# define dirent direct
# if HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif
# if HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif
# if HAVE_NDIR_H
#  include <ndir.h>
# endif
#endif

#ifdef HAVE_SYS_UTIME_H
# include <sys/utime.h>
#endif

#include <errno.h>
#include <assert.h>

#if EEXIST == ENOTEMPTY
#define BROKEN_OS_PROBABLY_AIX
#endif

#ifdef __NeXT__
#define S_IRUSR S_IREAD
#define S_IWUSR S_IWRITE
#define S_IXUSR S_IEXEC
#define S_ISDIR(val) (S_IFDIR & val)
struct utimbuf
{
    time_t actime;
    time_t modtime;
};
#endif

#if defined(__GNUC__) && defined(AMIGA)
                                                            
                                                            
                                                            
#define REGPARAM2 REGPARAM
#else                       
#define REGPARAM2
#endif

                                                         
#if defined(__SASC) && defined(AMIGA)
#define REGPARAM2 
#define REGPARAM
#define S_IRUSR S_IREAD
#define S_IWUSR S_IWRITE
#define S_IXUSR S_IEXECUTE
#define S_ISDIR(val) (S_IFDIR & val)
#define mkdir(x,y) mkdir(x)
#define truncate(x,y) 0
#define creat(x,y) open("T:creat",O_CREAT|O_TEMP|O_RDWR)                       
#define strcasecmp stricmp
#define utime(file,time) 0
struct utimbuf
{
    time_t actime;
    time_t modtime;
};
#endif

#if defined(WARPUP)
#include "devices/timer.h"
#include "osdep/posixemu.h"
#define REGPARAM
#define REGPARAM2
#define RETSIGTYPE
                   
#define strcasecmp stricmp
#define memcpy q_memcpy
#define memset q_memset
#define strdup my_strdup
                      
#define creat(x,y) open("T:creat",O_CREAT|O_RDWR|O_TRUNC,777)
extern void* q_memset(void*,int,size_t);
extern void* q_memcpy(void*,const void*,size_t);
#endif

#ifdef __DOS__
#include <pc.h>
#include <io.h>
#endif

                          
#ifdef ACORN

#define S_IRUSR S_IREAD
#define S_IWUSR S_IWRITE
#define S_IXUSR S_IEXEC

#define strcasecmp stricmp

#endif

#ifndef L_tmpnam
#define L_tmpnam 128                       
#endif


                                               
typedef unsigned char uae_u8;
typedef signed char uae_s8;

typedef struct { uae_u8 RGB[3]; } RGB;

#ifdef __64BIT__                                                                          
typedef uint16_t uae_u16;
typedef int16_t uae_s16;

typedef uint32_t uae_u32;
typedef int32_t uae_s32;

#undef uae_s64
#undef uae_u64

#define uae_s64 int64_t
#define uae_u64 uint64_t
#define VAL64(a) (a)
#define UVAL64(a) (a)

typedef uae_u32 uaecptr;
typedef uae_u64 hostptr;
                                          
                            

#else

#if SIZEOF_SHORT == 2
typedef unsigned short uae_u16;
typedef short uae_s16;
#elif SIZEOF_INT == 2
typedef unsigned int uae_u16;
typedef int uae_s16;
#else
#error No 2 byte type, you lose.
#endif

#if SIZEOF_INT == 4
typedef unsigned int uae_u32;
typedef int uae_s32;
#elif SIZEOF_LONG == 4
typedef unsigned long uae_u32;
typedef long uae_s32;
#else
#error No 4 byte type, you lose.
#endif

#undef uae_s64
#undef uae_u64

#if SIZEOF_LONG_LONG == 8
#define uae_s64 long long
#define uae_u64 unsigned long long
#define VAL64(a) (a ## LL)
#define UVAL64(a) (a ## uLL)
#elif SIZEOF___INT64 == 8
#define uae_s64 __int64
#define uae_u64 unsigned __int64
#define VAL64(a) (a)
#define UVAL64(a) (a)
#elif SIZEOF_LONG == 8
#define uae_s64 long;
#define uae_u64 unsigned long;
#define VAL64(a) (a ## l)
#define UVAL64(a) (a ## ul)
#endif

typedef uae_u32 uaecptr;
typedef uae_u32 hostptr;

#endif

#ifdef HAVE_STRDUP
#define my_strdup strdup
#else
extern char *my_strdup (const char*s);
#endif

extern void *xmalloc(size_t);
extern void *xcalloc(size_t, size_t);

                                                                           
                                                                       
#ifdef __GNUC__
#define ENUMDECL typedef enum
#define ENUMNAME(name) name

  
                                                                   
                                                                 
                                                                        
  
                                                 
   

#undef DONT_HAVE_POSIX
#undef DONT_HAVE_REAL_POSIX                                                      
#undef DONT_HAVE_STDIO
#undef DONT_HAVE_MALLOC

#if defined(WARPUP)
#define DONT_HAVE_POSIX
#endif

#if defined _WIN32

#if defined __WATCOMC__

#define O_NDELAY 0
#include <direct.h>
#define dirent direct
#define mkdir(a,b) mkdir(a)
#define strcasecmp stricmp

#elif defined __MINGW32__

#define O_NDELAY 0
#define mkdir(a,b) mkdir(a)

#endif

#endif              

#ifdef DONT_HAVE_POSIX

#define access posixemu_access
extern int posixemu_access (const char *, int);
#define open posixemu_open
extern int posixemu_open (const char *, int, int);
#define close posixemu_close
extern void posixemu_close (int);
#define read posixemu_read
extern int posixemu_read (int, char *, int);
#define write posixemu_write
extern int posixemu_write (int, const char *, int);
#undef lseek
#define lseek posixemu_seek
extern int posixemu_seek (int, int, int);
#define stat(a,b) posixemu_stat ((a), (b))
extern int posixemu_stat (const char *, STAT *);
#define mkdir posixemu_mkdir
extern int mkdir (const char *, int);
#define rmdir posixemu_rmdir
extern int posixemu_rmdir (const char *);
#define unlink posixemu_unlink
extern int posixemu_unlink (const char *);
#define truncate posixemu_truncate
extern int posixemu_truncate (const char *, long int);
#define rename posixemu_rename
extern int posixemu_rename (const char *, const char *);
#define chmod posixemu_chmod
extern int posixemu_chmod (const char *, int);
#define tmpnam posixemu_tmpnam
extern void posixemu_tmpnam (char *);
#define utime posixemu_utime
extern int posixemu_utime (const char *, struct utimbuf *);
#define opendir posixemu_opendir
extern DIR * posixemu_opendir (const char *);
#define readdir posixemu_readdir
extern struct dirent* readdir (DIR *);
#define closedir posixemu_closedir
extern void closedir (DIR *);

                                                                             
                                                                           
                
extern long dos_errno (void);

#endif

#ifdef DONT_HAVE_STDIO

extern FILE *stdioemu_fopen (const char *, const char *);
#define fopen(a,b) stdioemu_fopen(a, b)
extern int stdioemu_fseek (FILE *, int, int);
#define fseek(a,b,c) stdioemu_fseek(a, b, c)
extern int stdioemu_fread (char *, int, int, FILE *);
#define fread(a,b,c,d) stdioemu_fread(a, b, c, d)
extern int stdioemu_fwrite (const char *, int, int, FILE *);
#define fwrite(a,b,c,d) stdioemu_fwrite(a, b, c, d)
extern int stdioemu_ftell (FILE *);
#define ftell(a) stdioemu_ftell(a)
extern int stdioemu_fclose (FILE *);
#define fclose(a) stdioemu_fclose(a)

#endif

#ifdef DONT_HAVE_MALLOC

#define malloc(a) mallocemu_malloc(a)
extern void *mallocemu_malloc (int size);
#define free(a) mallocemu_free(a)
extern void mallocemu_free (void *ptr);

#endif

#ifdef X86_ASSEMBLY
#define ASM_SYM_FOR_FUNC(a) __asm__(a)
#else
#define ASM_SYM_FOR_FUNC(a)
#endif

#if defined USE_COMPILER
#undef NO_PREFETCH_BUFFER
#undef NO_EXCEPTION_3
#define NO_EXCEPTION_3
#define NO_PREFETCH_BUFFER
#endif

#include "target.h"

#ifdef UAE_CONSOLE
#undef write_log
#if defined(__PSP2__)
#define write_log vita_write_log
#elif defined(__SWITCH__)
#define write_log psp2shell_print
#else
#define write_log write_log_standard
#endif
#else
#undef write_log
#define write_log(FORMATO, RESTO...)
#endif

#ifdef DEBUG_UAE4ALL
#if __GNUC__ - 1 > 1 || __GNUC_MINOR__ - 1 > 6
extern void write_log_standard (const char *, ...) __attribute__ ((format (printf, 1, 2)));
#else

extern void write_log_standard (const char *, ...);
extern void console_out (const char *, ...);
extern void console_flush (void);
extern int console_get (char *, int);
#endif
#else
#ifdef __PSP2__
extern void vita_write_log (const char *, ...);
#ifdef write_log
#undef write_log
#endif
#define write_log vita_write_log
#else
#ifdef write_log
#undef write_log
#endif
#define write_log(FORMATO, RESTO...)
#endif
#define write_log_standar(FORMATO, RESTO...)
#define console_out(FORMATO, RESTO...)
#define console_get(FORMATO, RESTO...)
#endif

#ifndef O_BINARY
#define O_BINARY 0
#endif

#ifndef STATIC_INLINE
#define STATIC_INLINE static __inline__
#endif

                                                
#define abort() \
  do { \
    write_log ("Internal error; file %s, line %d\n", __FILE__, __LINE__); \
    (abort) (); \
} while (0)
#else
#ifndef STATIC_INLINE
#define STATIC_INLINE static __inline__
#endif
#define ENUMDECL enum
#define ENUMNAME(name) ; typedef int name
#endif

                                                                           
                                                                            
                        
                                                                          
                                                   

                                                                          
          
#define CYCLE_UNIT 512

                                                                            
                                                   
#define OFFICIAL_CYCLE_UNIT 512

  
                                                                       
                                                                          
                                                           
                               
   
#define CPU_EMU_SIZE 0

#undef REGPARAM
#define REGPARAM

#endif
