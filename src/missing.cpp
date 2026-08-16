   
                                 
   
                                       
   
                                
    

#include "sysconfig.h"
#include "sysdeps.h"
#include "config.h"

#include "uae.h"

#ifndef HAVE_STRDUP

char *my_strdup (const char *s)
{
                                                                       
    char *x = (char*)xmalloc(strlen((char *)s) + 1);
    strcpy(x, (char *)s);
    return x;
}

#endif


void *xmalloc (size_t n)
{
    void *a = uae_malloc (n);
    if (a == NULL) {
	write_log ("virtual memory exhausted\n");
	return NULL;
    }
    return a;
}

void *xcalloc (size_t n, size_t size)
{
    void *a = calloc (n, size);
    if (a == NULL) {
	write_log ("virtual memory exhausted\n");
	return NULL;
    }
    return a;
}
