  

                                        

                                                                       
                                                                          
                                                                         
                                                                        
                                                                     
                                                                    

                                                                          
                                                   

                                                                          
                                                                        
                                                                        
                                                                          
                                                                       
                                                                   
                         

  

#ifndef _PSP2_DIRENT_H_
#define	_PSP2_DIRENT_H_

#include <sys/types.h>
#include <sys/time.h>

#include <psp2/io/dirent.h>

#ifdef __cplusplus
extern "C" {
#endif

#define F_opendir 1
#define F_readdir 1
#define F_closedir 1

#define DT_DIR 0
#define DT_REG 1

struct dirent
{
	                   
	SceIoStat	d_stat;
	                 
	char	d_name[256];
	                            
	void	*d_private;
	int	dummy;
	int d_type;
};

struct DIR_;
typedef struct DIR_ DIR;

int            closedir(DIR *);
DIR           *opendir(const char *);
struct dirent *readdir(DIR *);
int            readdir_r(DIR *, struct dirent *, struct dirent **);
void           rewinddir(DIR *);
void           seekdir(DIR *, long int);
long int       telldir(DIR *);

#ifdef __cplusplus
}
#endif

#endif                      
