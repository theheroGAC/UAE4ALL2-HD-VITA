   
                                 
   
                           
   
                                
    

static __inline__ char *raddr(uaecptr p)
{
    return p == 0 ? NULL : (char *)get_real_address(p);
}

extern void gfxlib_install(void);

                      

extern int GFX_WritePixel(uaecptr rp, int x, int y);

