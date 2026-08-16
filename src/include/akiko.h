#ifndef AKIKO_H
#define AKIKO_H

#include "sysconfig.h"
#include "sysdeps.h"

extern uae_u8 akiko_buffer[8192];
extern uae_u32 akiko_bplcon;

void akiko_init(void);
void akiko_reset(void);
uae_u32 akiko_read(uaecptr addr);
void akiko_write(uaecptr addr, uae_u32 value);
int akiko_bget(uaecptr addr);
int akiko_wget(uaecptr addr);
int akiko_lget(uaecptr addr);
void akiko_bput(uaecptr addr, int val);
void akiko_wput(uaecptr addr, int val);
void akiko_lput(uaecptr addr, int val);

#endif
