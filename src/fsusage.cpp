                                                         
                                                                

                                                                       
                                                                       
                                                                      
                     

                                                                  
                                                                 
                                                                
                                               

                                                                    
                                                                          
                                                                     

#include "sysconfig.h"
#include "sysdeps.h"
#include "config.h"

#include <stdlib.h>
#include <sys/types.h>

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include "fsusage.h"

                                                  
                                                        
                                                                       

static long
adjust_blocks
	(long blocks,
	int fromsize, int tosize)
{
  if (tosize <= 0)
    abort ();
  if (fromsize <= 0)
    return -1;

  if (fromsize == tosize)	                           
    return blocks;
  else if (fromsize > tosize)	                            
    return blocks * (fromsize / tosize);
  else				                           
    return (blocks + (blocks < 0 ? -1 : 1)) / (tosize / fromsize);
}


int statfs ();

#if HAVE_UNISTD_H
# include <unistd.h>
#endif

#if HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif

#if HAVE_SYS_MOUNT_H
# include <sys/mount.h>
#endif

#if HAVE_SYS_VFS_H
#ifndef AROS
# include <sys/vfs.h>
#endif
#endif

#if HAVE_SYS_FS_S5PARAM_H	                   
# include <sys/fs/s5param.h>
#endif

#if defined (HAVE_SYS_FILSYS_H) && !defined (_CRAY)
# include <sys/filsys.h>	          
#endif

#if HAVE_FCNTL_H
# include <fcntl.h>
#endif

#if HAVE_SYS_STATFS_H
#ifndef AROS
# include <sys/statfs.h>
#endif
#endif

#if HAVE_DUSTAT_H		              
# include <sys/dustat.h>
#endif

#if HAVE_SYS_STATVFS_H		          
# include <sys/statvfs.h>
int statvfs ();
#endif

                                                                       
                                                                    
                    

int
safe_read
      (int desc,
      char *ptr,
      int len)
{
  int n_chars;

  if (len <= 0)
    return len;

#ifdef EINTR
  do
    {
      n_chars = read (desc, ptr, len);
    }
  while (n_chars < 0 && errno == EINTR);
#else
  n_chars = read (desc, ptr, len);
#endif

  return n_chars;
}

#if defined(__PSP2__) || defined(__SWITCH__)
#ifdef __PSP2__                  
#include <psp2/io/devctl.h>
#endif

  
                
                      
                       
                          
              
               
  

int
get_fs_usage
	(const char *path,
	const char *disk,
	struct fs_usage *fsp)
{
	fsp->fsu_blocks = 507289;
	fsp->fsu_bfree = 3435973;
	fsp->fsu_bavail = 507289 / 2;
	fsp->fsu_files = 3435973;
	fsp->fsu_ffree = 3435973;

  
                   
                                        
                                                                          
                
  
  
	return 0;
}
#else

                                                                   
                                        
                                                                 
                                
                                                                     
                                                                
                                                  
int
get_fs_usage
	(const char *path,
	const char *disk,
	struct fs_usage *fsp)
{
	                                                      
	                 
	fsp->fsu_blocks = 507289;
	fsp->fsu_bfree = 3435973;
	fsp->fsu_bavail = 507289 / 2;
	fsp->fsu_files = 3435973;
	fsp->fsu_ffree = 3435973;
		
#ifdef STAT_STATFS3_OSF1
# define CONVERT_BLOCKS(B) adjust_blocks ((B), fsd.f_fsize, 512)

  struct statfs fsd;

  if (statfs (path, &fsd, sizeof (struct statfs)) != 0)
    return -1;

#endif                        

#ifdef STAT_STATFS2_FS_DATA	            
# define CONVERT_BLOCKS(B) adjust_blocks ((B), 1024, 512)

  struct fs_data fsd;

  if (statfs (path, &fsd) != 1)
    return -1;
  fsp->fsu_blocks = CONVERT_BLOCKS (fsd.fd_req.btot);
  fsp->fsu_bfree = CONVERT_BLOCKS (fsd.fd_req.bfree);
  fsp->fsu_bavail = CONVERT_BLOCKS (fsd.fd_req.bfreen);
  fsp->fsu_files = fsd.fd_req.gtot;
  fsp->fsu_ffree = fsd.fd_req.gfree;

#endif                           

#ifdef STAT_READ_FILSYS		          
# ifndef SUPERBOFF
#  define SUPERBOFF (SUPERB * 512)
# endif
# define CONVERT_BLOCKS(B) \
    adjust_blocks ((B), (fsd.s_type == Fs2b ? 1024 : 512), 512)

  struct filsys fsd;
  int fd;

  if (! disk)
    {
      errno = 0;
      return -1;
    }

  fd = open (disk, O_RDONLY);
  if (fd < 0)
    return -1;
  lseek (fd, (long) SUPERBOFF, 0);
  if (safe_read (fd, (char *) &fsd, sizeof fsd) != sizeof fsd)
    {
      close (fd);
      return -1;
    }
  close (fd);
  fsp->fsu_blocks = CONVERT_BLOCKS (fsd.s_fsize);
  fsp->fsu_bfree = CONVERT_BLOCKS (fsd.s_tfree);
  fsp->fsu_bavail = CONVERT_BLOCKS (fsd.s_tfree);
  fsp->fsu_files = (fsd.s_isize - 2) * INOPB * (fsd.s_type == Fs2b ? 2 : 1);
  fsp->fsu_ffree = fsd.s_tinode;

#endif                       

#ifdef STAT_STATFS2_BSIZE	                                 
# define CONVERT_BLOCKS(B) adjust_blocks ((B), fsd.f_bsize, 512)

  struct statfs fsd;

  if (statfs (path, &fsd) < 0)
    return -1;

# ifdef STATFS_TRUNCATES_BLOCK_COUNTS

                                                                 
                                                                      
                                                                     
                                                                    
                                   
  if (fsd.f_blocks == 0x1fffff && fsd.f_spare[0] > 0)
    {
      fsd.f_blocks = fsd.f_spare[0];
      fsd.f_bfree = fsd.f_spare[1];
      fsd.f_bavail = fsd.f_spare[2];
    }
# endif                                    

#endif                         

#ifdef STAT_STATFS2_FSIZE	            
# define CONVERT_BLOCKS(B) adjust_blocks ((B), fsd.f_fsize, 512)

  struct statfs fsd;

  if (statfs (path, &fsd) < 0)
    return -1;

#endif                         

#ifdef STAT_STATFS4		                            
# if _AIX || defined(_CRAY)
#  define CONVERT_BLOCKS(B) adjust_blocks ((B), fsd.f_bsize, 512)
#  ifdef _CRAY
#   define f_bavail f_bfree
#  endif
# else
#  define CONVERT_BLOCKS(B) (B)
#  ifndef _SEQUENT_		                            
#   ifndef DOLPHIN		                                        
#    define f_bavail f_bfree
#   endif
#  endif
# endif

  struct statfs fsd;

  if (statfs (path, &fsd, sizeof fsd, 0) < 0)
    return -1;
                                                                
                                                           
                                          

#endif                   

#ifdef STAT_STATVFS		          
# define CONVERT_BLOCKS(B) \
    adjust_blocks ((B), fsd.f_frsize ? fsd.f_frsize : fsd.f_bsize, 512)

  struct statvfs fsd;

  if (statvfs (path, &fsd) < 0)
    return -1;
                                                   

#endif                   

#if !defined(STAT_STATFS2_FS_DATA) && !defined(STAT_READ_FILSYS) && !defined(__SYMBIAN32__)
				                      

  fsp->fsu_blocks = CONVERT_BLOCKS (fsd.f_blocks);
  fsp->fsu_bfree = CONVERT_BLOCKS (fsd.f_bfree);
  fsp->fsu_bavail = CONVERT_BLOCKS (fsd.f_bavail);
  fsp->fsu_files = fsd.f_files;
  fsp->fsu_ffree = fsd.f_ffree;

#endif                                                       

  return 0;
}
#endif            
