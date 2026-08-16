    
                                 
    
                           
   
                                
    

#ifndef MACCESS_UAE_H
#define MACCESS_UAE_H

static uae_u16 do_get_mem_word(uae_u16 *_GCCRES_ a);
static void do_put_mem_word(uae_u16 *_GCCRES_ a, uae_u16 v);

static __inline__ uae_u32 do_get_mem_long(uae_u32 *_GCCRES_ a)
{
#ifdef USE_FAME_CORE
   uae_u32 b;
                                                                        
              
                                                                                   
                                                                                      
      
   b  = do_get_mem_word((uae_u16 *)a) << 16;
   b |= do_get_mem_word((uae_u16 *)((hostptr)a + 2));
   return b;
#else
    uae_u8 *b = (uae_u8 *)a;
    
    return (*b << 24) | (*(b+1) << 16) | (*(b+2) << 8) | (*(b+3));
#endif
}

static __inline__ uae_u16 do_get_mem_word(uae_u16 *_GCCRES_ a)
{
#ifdef USE_FAME_CORE
                                                                        
   if ((hostptr)a & 1) {
                 
                                                                        
                                                                     
         
      uae_u32 b;
      b = *((uae_u32 *)((hostptr)a ^ 1));
      b = (b << 8) | (b >> 24);
      return (uae_u16)b;
   }
   else {
      return (*a);
   }
#else
    uae_u8 *b = (uae_u8 *)a;
    
    return (*b << 8) | (*(b+1));
#endif
}


static __inline__ uae_u8 do_get_mem_byte(uae_u8 *_GCCRES_ a)
{
#ifdef USE_FAME_CORE
    a= (uae_u8 *)(((hostptr)a)^1);
#endif

    return *a;
}

static __inline__ void do_put_mem_long(uae_u32 *_GCCRES_ a, uae_u32 v)
{
#ifdef USE_FAME_CORE
                                                                        
              
                                                                          
                                                                
      
   do_put_mem_word((uae_u16 *)a, v >> 16);
   do_put_mem_word((uae_u16 *)((hostptr)a + 2), v);
#else
    uae_u8 *b = (uae_u8 *)a;
    
    *b = v >> 24;
    *(b+1) = v >> 16;    
    *(b+2) = v >> 8;
    *(b+3) = v;
#endif
}

static __inline__ void do_put_mem_word(uae_u16 *_GCCRES_ a, uae_u16 v)
{
#ifdef USE_FAME_CORE
                                                                        
   if ((hostptr)a & 1) {
                 
                                                                         
                                                     
         
      *((uae_u8 *)a - 1) = v >> 8;
      *((uae_u8 *)a + 2) = v;
   }
   else {
      (*a)=v;
   }
#else
    uae_u8 *b = (uae_u8 *)a;
    
    *b = v >> 8;
    *(b+1) = v;
#endif
}

static __inline__ void do_put_mem_byte(uae_u8 *_GCCRES_ a, uae_u8 v)
{
#ifdef USE_FAME_CORE
     a= (uae_u8 *)(((hostptr)a)^1);
#endif

    *a = v;
}

#define call_mem_get_func(func, addr) ((*func)(addr))
#define call_mem_put_func(func, addr, v) ((*func)(addr, v))

#undef MD_HAVE_MEM_1_FUNCS

#endif
