   
                                 
   
                                                                      
                                                   
                             
   
                                
    

#include "sysconfig.h"
#include "sysdeps.h"
#include "config.h"

#include "fsdb.h"

                                                                            
int fsdb_name_invalid (const char *n)
{
    if (strcmp (n, FSDB_FILE) == 0)
	return 1;
    if (n[0] != '.')
	return 0;
    if (n[1] == '\0')
	return 1;
    return n[1] == '.' && n[2] == '\0';
}

                                                                           
                                                                
void fsdb_fill_file_attrs (a_inode *aino)
{
    struct stat statbuf;
                                          
    if (stat (aino->nname, &statbuf) == -1)
	return;
    aino->dir = S_ISDIR (statbuf.st_mode) ? 1 : 0;
    
    aino->amigaos_mode = ((S_IXUSR & statbuf.st_mode ? 0 : A_FIBF_EXECUTE)
    			  | (S_IWUSR & statbuf.st_mode ? 0 : A_FIBF_WRITE)
    			  | (S_IRUSR & statbuf.st_mode ? 0 : A_FIBF_READ));

#if defined(WIN32) || defined(ANDROIDSDL) || defined(__PSP2__) || defined(__SWITCH__)
                                            
    aino->amigaos_mode &= ~A_FIBF_EXECUTE;
    aino->amigaos_mode &= ~A_FIBF_READ;
#endif
}

int fsdb_set_file_attrs (a_inode *aino, int mask)
{
    struct stat statbuf;
    int mode;

    if (stat (aino->nname, &statbuf) == -1)
	return ERROR_OBJECT_NOT_AROUND;
	
    mode = statbuf.st_mode;
                                                          
    if (! aino->dir) {
	if (mask & A_FIBF_READ)
	    mode &= ~S_IRUSR;
	else
	    mode |= S_IRUSR;

	if (mask & A_FIBF_WRITE)
	    mode &= ~S_IWUSR;
	else
	    mode |= S_IWUSR;

	if (mask & A_FIBF_EXECUTE)
	    mode &= ~S_IXUSR;
	else
	    mode |= S_IXUSR;

#if !defined(__PSP2__) && !defined(__SWITCH__)
	chmod (aino->nname, mode);
#endif
    }

    aino->amigaos_mode = mask;
    aino->dirty = 1;
    return 0;
}

                                                                         
                                                       
int fsdb_mode_representable_p (const a_inode *aino)
{
    if (aino->dir)
	return aino->amigaos_mode == 0;
    return (aino->amigaos_mode & (A_FIBF_DELETE | A_FIBF_SCRIPT | A_FIBF_PURE)) == 0;
}

char *fsdb_create_unique_nname (a_inode *base, const char *suggestion)
{
    char tmp[256] = "__uae___";
    strncat (tmp, suggestion, 240);
    for (;;) {
	int i;
	char *p = build_nname (base->nname, tmp);
	if (access (p, R_OK) < 0 && errno == ENOENT) {
	    printf ("unique name: %s\n", p);
	    return p;
	}
	free (p);

	                                                                   
                                                         
	for (i = 0; i < 8; i++) {
#if WIN32 || __PSP2__ || __SWITCH__
	    tmp[i] = "_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"[rand () % 63];
#else
	    tmp[i] = "_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"[random () % 63];
#endif
	}
    }
}
