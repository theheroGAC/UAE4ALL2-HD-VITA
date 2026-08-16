  
                                                    
  
                                      
  
                                                                        
                                                                  
                                                                        
                                                                      
                                                                     
                                                                        
                            
  
                                                                          
                                                         
  
                                                                            
                                                             
                                                                         
                                                                    
                                                                        
                                                                        
                                  
  
  
                                         
                                                                         
                                                                        
                                    
  
                                          
                                                                     
                                                            
  
                                         
                                                                       
                                                                      
                                                               
  
                                                                        
                                                                     
  
                                          
                                              
  
                                          
                                                                       
                                                                         
                                                  
                                                      
  
                                                                        
                                                                           
                   
  
                                                                          
                                                             
  
                                             
                                  
  
                                         
                                                                       
                                                                  
                                                                            
                                       
  
                           
                                                                      
                                                                    
                           
  
                            
                                                                    
                                                                   
                                                                  
                                       
  
                            
                                                                         
                                  
  
                           
                                                                      
                                                                            
                                                                         
                      
  
                           
                 
                                                                               
#ifndef DIRENT_H
#define DIRENT_H

#if !defined(_68K_) && !defined(_MPPC_) && !defined(_X86_) && !defined(_IA64_) && !defined(_AMD64_) && defined(_M_IX86)
#   define _X86_
#endif
#include <stdio.h>
#include <stdarg.h>
#include <windef.h>
#include <winbase.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

                                                                  
#define _DIRENT_HAVE_D_TYPE

                                                                    
#define _DIRENT_HAVE_D_NAMLEN

                                   
#if !defined(FILE_ATTRIBUTE_DEVICE)
#   define FILE_ATTRIBUTE_DEVICE 0x40
#endif

                                               
#if !defined(S_IFMT)
#   define S_IFMT   _S_IFMT                                         
#endif
#if !defined(S_IFDIR)
#   define S_IFDIR  _S_IFDIR                                   
#endif
#if !defined(S_IFCHR)
#   define S_IFCHR  _S_IFCHR                                          
#endif
#if !defined(S_IFFIFO)
#   define S_IFFIFO _S_IFFIFO                             
#endif
#if !defined(S_IFREG)
#   define S_IFREG  _S_IFREG                                      
#endif
#if !defined(S_IREAD)
#   define S_IREAD  _S_IREAD                                         
#endif
#if !defined(S_IWRITE)
#   define S_IWRITE _S_IWRITE                                         
#endif
#if !defined(S_IEXEC)
#   define S_IEXEC  _S_IEXEC                                            
#endif
#if !defined(S_IFIFO)
#   define S_IFIFO _S_IFIFO                               
#endif
#if !defined(S_IFBLK)
#   define S_IFBLK   0                                            
#endif
#if !defined(S_IFLNK)
#   define S_IFLNK   0                                    
#endif
#if !defined(S_IFSOCK)
#   define S_IFSOCK  0                                      
#endif

#if defined(_MSC_VER)
#   define S_IRUSR  S_IREAD                                    
#   define S_IWUSR  S_IWRITE                                    
#   define S_IXUSR  0                                             
#   define S_IRGRP  0                                           
#   define S_IWGRP  0                                            
#   define S_IXGRP  0                                              
#   define S_IROTH  0                                            
#   define S_IWOTH  0                                             
#   define S_IXOTH  0                                               
#endif

                                 
#if !defined(PATH_MAX)
#   define PATH_MAX MAX_PATH
#endif
#if !defined(FILENAME_MAX)
#   define FILENAME_MAX MAX_PATH
#endif
#if !defined(NAME_MAX)
#   define NAME_MAX FILENAME_MAX
#endif

                                
#define DT_UNKNOWN  0
#define DT_REG      S_IFREG
#define DT_DIR      S_IFDIR
#define DT_FIFO     S_IFIFO
#define DT_SOCK     S_IFSOCK
#define DT_CHR      S_IFCHR
#define DT_BLK      S_IFBLK

                                                      
#define IFTODT(mode) ((mode) & S_IFMT)
#define DTTOIF(type) (type)

  
                                                                          
                                                                            
                                                                           
              
   
#define	S_ISFIFO(mode) (((mode) & S_IFMT) == S_IFIFO)
#define	S_ISDIR(mode)  (((mode) & S_IFMT) == S_IFDIR)
#define	S_ISREG(mode)  (((mode) & S_IFMT) == S_IFREG)
#define	S_ISLNK(mode)  (((mode) & S_IFMT) == S_IFLNK)
#define	S_ISSOCK(mode) (((mode) & S_IFMT) == S_IFSOCK)
#define	S_ISCHR(mode)  (((mode) & S_IFMT) == S_IFCHR)
#define	S_ISBLK(mode)  (((mode) & S_IFMT) == S_IFBLK)

                                                                 
#define _D_EXACT_NAMLEN(p) ((p)->d_namlen)

                                                     
#define _D_ALLOC_NAMLEN(p) (PATH_MAX + 1)


#ifdef __cplusplus
extern "C" {
#endif


                            
struct _wdirent {
    long d_ino;                                                  
    unsigned short d_reclen;                                        
    size_t d_namlen;                                                           
    int d_type;                                                
    wchar_t d_name[PATH_MAX + 1];                              
};
typedef struct _wdirent _wdirent;

struct _WDIR {
    struct _wdirent ent;                                                     
    WIN32_FIND_DATAW data;                                             
    int cached;                                                            
    HANDLE handle;                                                       
    wchar_t *patt;                                                          
};
typedef struct _WDIR _WDIR;

static _WDIR *_wopendir (const wchar_t *dirname);
static struct _wdirent *_wreaddir (_WDIR *dirp);
static int _wclosedir (_WDIR *dirp);
static void _wrewinddir (_WDIR* dirp);


                                    
#define wdirent _wdirent
#define WDIR _WDIR
#define wopendir _wopendir
#define wreaddir _wreaddir
#define wclosedir _wclosedir
#define wrewinddir _wrewinddir


                                   
struct dirent {
    long d_ino;                                                  
    unsigned short d_reclen;                                        
    size_t d_namlen;                                                           
    int d_type;                                                
    char d_name[PATH_MAX + 1];                                 
};
typedef struct dirent dirent;

struct DIR {
    struct dirent ent;
    struct _WDIR *wdirp;
};
typedef struct DIR DIR;

static DIR *opendir (const char *dirname);
static struct dirent *readdir (DIR *dirp);
static int closedir (DIR *dirp);
static void rewinddir (DIR* dirp);


                                
static WIN32_FIND_DATAW *dirent_first (_WDIR *dirp);
static WIN32_FIND_DATAW *dirent_next (_WDIR *dirp);

static int dirent_mbstowcs_s(
    size_t *pReturnValue,
    wchar_t *wcstr,
    size_t sizeInWords,
    const char *mbstr,
    size_t count);

static int dirent_wcstombs_s(
    size_t *pReturnValue,
    char *mbstr,
    size_t sizeInBytes,
    const wchar_t *wcstr,
    size_t count);

static void dirent_set_errno (int error);

  
                                                                     
                                                                      
           
   
static _WDIR*
_wopendir(
    const wchar_t *dirname)
{
    _WDIR *dirp = NULL;
    int error;

                                  
    if (dirname == NULL  ||  dirname[0] == '\0') {
        dirent_set_errno (ENOENT);
        return NULL;
    }

                                      
    dirp = (_WDIR*) malloc (sizeof (struct _WDIR));
    if (dirp != NULL) {
        DWORD n;

                                   
        dirp->handle = INVALID_HANDLE_VALUE;
        dirp->patt = NULL;
        dirp->cached = 0;

                                                                  
        n = GetFullPathNameW (dirname, 0, NULL, NULL);

                                                                          
        dirp->patt = (wchar_t*) malloc (sizeof (wchar_t) * n + 16);
        if (dirp->patt) {

              
                                                                        
                                                                         
                                                                              
               
            n = GetFullPathNameW (dirname, n, dirp->patt, NULL);
            if (n > 0) {
                wchar_t *p;

                                                                    
                p = dirp->patt + n;
                if (dirp->patt < p) {
                    switch (p[-1]) {
                    case '\\':
                    case '/':
                    case ':':
                                                                             
                               ;
                        break;

                    default:
                                                                          
                        *p++ = '\\';
                    }
                }
                *p++ = '*';
                *p = '\0';

                                                                        
                if (dirent_first (dirp)) {
                                                              
                    error = 0;
                } else {
                                                     
                    error = 1;
                    dirent_set_errno (ENOENT);
                }

            } else {
                                                    
                dirent_set_errno (ENOENT);
                error = 1;
            }

        } else {
                                                           
            error = 1;
        }

    } else {
                                             
        error = 1;
    }

                                   
    if (error  &&  dirp) {
        _wclosedir (dirp);
        dirp = NULL;
    }

    return dirp;
}

  
                                                                        
                                                                           
                                                                           
                                                                        
   
static struct _wdirent*
_wreaddir(
    _WDIR *dirp)
{
    WIN32_FIND_DATAW *datap;
    struct _wdirent *entp;

                                   
    datap = dirent_next (dirp);
    if (datap) {
        size_t n;
        DWORD attr;
        
                                                  
        entp = &dirp->ent;

           
                                                                            
                                                                            
                                                                
           
        n = 0;
        while (n < PATH_MAX  &&  datap->cFileName[n] != 0) {
            entp->d_name[n] = datap->cFileName[n];
            n++;
        }
        dirp->ent.d_name[n] = 0;

                                                           
        entp->d_namlen = n;

                       
        attr = datap->dwFileAttributes;
        if ((attr & FILE_ATTRIBUTE_DEVICE) != 0) {
            entp->d_type = DT_CHR;
        } else if ((attr & FILE_ATTRIBUTE_DIRECTORY) != 0) {
            entp->d_type = DT_DIR;
        } else {
            entp->d_type = DT_REG;
        }

                                
        entp->d_ino = 0;
        entp->d_reclen = sizeof (struct _wdirent);

    } else {

                                       
        entp = NULL;

    }

    return entp;
}

  
                                                                             
                                                                  
               
   
static int
_wclosedir(
    _WDIR *dirp)
{
    int ok;
    if (dirp) {

                                   
        if (dirp->handle != INVALID_HANDLE_VALUE) {
            FindClose (dirp->handle);
            dirp->handle = INVALID_HANDLE_VALUE;
        }

                                    
        if (dirp->patt) {
            free (dirp->patt);
            dirp->patt = NULL;
        }

                                         
        free (dirp);
        ok =            0;

    } else {
                                      
        dirent_set_errno (EBADF);
        ok =            -1;
    }
    return ok;
}

  
                                                                       
                   
   
static void
_wrewinddir(
    _WDIR* dirp)
{
    if (dirp) {
                                            
        if (dirp->handle != INVALID_HANDLE_VALUE) {
            FindClose (dirp->handle);
        }

                                    
        dirent_first (dirp);
    }
}

                                          
static WIN32_FIND_DATAW*
dirent_first(
    _WDIR *dirp)
{
    WIN32_FIND_DATAW *datap;

                                                     
    dirp->handle = FindFirstFileW (dirp->patt, &dirp->data);
    if (dirp->handle != INVALID_HANDLE_VALUE) {

                                                        
        datap = &dirp->data;
        dirp->cached = 1;

    } else {

                                                                       
        dirp->cached = 0;
        datap = NULL;

    }
    return datap;
}

                                         
static WIN32_FIND_DATAW*
dirent_next(
    _WDIR *dirp)
{
    WIN32_FIND_DATAW *p;

                                  
    if (dirp->cached != 0) {

                                                       
        p = &dirp->data;
        dirp->cached = 0;

    } else if (dirp->handle != INVALID_HANDLE_VALUE) {

                                                      
        if (FindNextFileW (dirp->handle, &dirp->data) != FALSE) {
                            
            p = &dirp->data;
        } else {
                                                                            
            FindClose (dirp->handle);
            dirp->handle = INVALID_HANDLE_VALUE;
            p = NULL;
        }

    } else {

                                             
        p = NULL;

    }

    return p;
}

   
                                                  
   
static DIR*
opendir(
    const char *dirname) 
{
    struct DIR *dirp;
    int error;

                                  
    if (dirname == NULL  ||  dirname[0] == '\0') {
        dirent_set_errno (ENOENT);
        return NULL;
    }

                                           
    dirp = (DIR*) malloc (sizeof (struct DIR));
    if (dirp) {
        wchar_t wname[PATH_MAX + 1];
        size_t n;

                                                             
        error = dirent_mbstowcs_s(
            &n, wname, PATH_MAX + 1, dirname, PATH_MAX);
        if (!error) {

                                                                 
            dirp->wdirp = _wopendir (wname);
            if (dirp->wdirp) {
                                             
                error = 0;
            } else {
                                                     
                error = 1;
            }

        } else {
               
                                                                       
                                                                            
                                                                      
                      
               
            error = 1;
        }

    } else {
                                           
        error = 1;
    }

                                   
    if (error  &&  dirp) {
        free (dirp);
        dirp = NULL;
    }

    return dirp;
}

  
                             
  
                                                                           
                                                                              
                                                                        
                                                                               
                                                                            
                                                                     
                                                                           
                                                                          
                    
   
static struct dirent*
readdir(
    DIR *dirp) 
{
    WIN32_FIND_DATAW *datap;
    struct dirent *entp;

                                   
    datap = dirent_next (dirp->wdirp);
    if (datap) {
        size_t n;
        int error;

                                                               
        error = dirent_wcstombs_s(
            &n, dirp->ent.d_name, MAX_PATH + 1, datap->cFileName, MAX_PATH);

           
                                                                         
                                                                          
                                                                 
                                                                           
          
                                                                       
                                                              
                                                     
           
        if (error  &&  datap->cAlternateFileName[0] != '\0') {
            error = dirent_wcstombs_s(
                &n, dirp->ent.d_name, MAX_PATH + 1, datap->cAlternateFileName,
                sizeof (datap->cAlternateFileName) / 
                    sizeof (datap->cAlternateFileName[0]));
        }

        if (!error) {
            DWORD attr;

                                                       
            entp = &dirp->ent;

                                                               
            entp->d_namlen = n - 1;

                                 
            attr = datap->dwFileAttributes;
            if ((attr & FILE_ATTRIBUTE_DEVICE) != 0) {
                entp->d_type = DT_CHR;
            } else if ((attr & FILE_ATTRIBUTE_DIRECTORY) != 0) {
                entp->d_type = DT_DIR;
            } else {
                entp->d_type = DT_REG;
            }

                                    
            entp->d_ino = 0;
            entp->d_reclen = sizeof (struct dirent);

        } else {
               
                                                                         
                                                                       
                                                                      
                                               
               
            entp = &dirp->ent;
            entp->d_name[0] = '?';
            entp->d_name[1] = '\0';
            entp->d_namlen = 1;
            entp->d_type = DT_UNKNOWN;
            entp->d_ino = 0;
            entp->d_reclen = 0;
        }

    } else {
                                       
        entp = NULL;
    }

    return entp;
}

  
                          
   
static int
closedir(
    DIR *dirp) 
{
    int ok;
    if (dirp) {

                                                   
        ok = _wclosedir (dirp->wdirp);
        dirp->wdirp = NULL;

                                                  
        free (dirp);

    } else {

                                      
        dirent_set_errno (EBADF);
        ok =            -1;

    }
    return ok;
}

  
                                        
   
static void
rewinddir(
    DIR* dirp) 
{
                                                       
    _wrewinddir (dirp->wdirp);
}

                                                        
static int
dirent_mbstowcs_s(
    size_t *pReturnValue,
    wchar_t *wcstr,
    size_t sizeInWords,
    const char *mbstr,
    size_t count)
{
    int error;

#if defined(_MSC_VER)  &&  _MSC_VER >= 1400

                                               
    error = mbstowcs_s (pReturnValue, wcstr, sizeInWords, mbstr, count);

#else

                                                       
    size_t n;

                                          
    n = mbstowcs (wcstr, mbstr, count);
    if (n < sizeInWords) {

                                          
        if (wcstr) {
            wcstr[n] = 0;
        }

                                                                       
        if (pReturnValue) {
            *pReturnValue = n + 1;
        }

                     
        error = 0;

    } else {

                                      
        error = 1;

    }

#endif

    return error;
}

                                                        
static int
dirent_wcstombs_s(
    size_t *pReturnValue,
    char *mbstr,
    size_t sizeInBytes,
    const wchar_t *wcstr,
    size_t count)
{
    int error;

#if defined(_MSC_VER)  &&  _MSC_VER >= 1400

                                               
    error = wcstombs_s (pReturnValue, mbstr, sizeInBytes, wcstr, count);

#else

                                                       
    size_t n;

                                      
    n = wcstombs (mbstr, wcstr, count);
    if (n < sizeInBytes) {

                                          
        if (mbstr) {
            mbstr[n] = '\0';
        }

                                                                         
        if (pReturnValue) {
            *pReturnValue = n + 1;
        }

                     
        error = 0;

    } else {

                                   
        error = 1;

    }

#endif

    return error;
}

                        
static void
dirent_set_errno(
    int error)
{
#if defined(_MSC_VER)

                                 
    _set_errno (error);

#else

                                
    errno = error;

#endif
}


#ifdef __cplusplus
}
#endif
#endif             

