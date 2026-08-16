#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sysconfig.h"
#include "sysdeps.h"
#include "config.h"
#include "memory-uae.h"
#include "akiko.h"

uae_u8 akiko_buffer[8192];
uae_u32 akiko_bplcon = 0;
static uae_u32 akiko_c2p_buffer[8];
static uae_u32 akiko_c2p_out[8];
static int akiko_c2p_pos = 0;

void akiko_init(void)
{
    memset(akiko_buffer, 0, sizeof(akiko_buffer));
    memset(akiko_c2p_buffer, 0, sizeof(akiko_c2p_buffer));
    memset(akiko_c2p_out, 0, sizeof(akiko_c2p_out));
    akiko_c2p_pos = 0;
}

void akiko_reset(void)
{
    akiko_init();
}

static void akiko_perform_c2p(void)
{
    int i, j;
    for (i = 0; i < 8; i++) {
        akiko_c2p_out[i] = 0;
    }
    for (i = 0; i < 32; i++) {
        uae_u32 val = 0;
        for (j = 0; j < 8; j++) {
            if (akiko_c2p_buffer[j] & (1 << (31 - i))) {
                val |= (1 << j);
            }
        }
        akiko_c2p_out[i / 4] |= (val << ((3 - (i % 4)) * 8));
    }
}

uae_u32 akiko_read(uaecptr addr)
{
    uaecptr offset = addr & 0xFFFF;
    if (offset >= 0x38 && offset < 0x58) {
        int idx = (offset - 0x38) / 4;
        return akiko_c2p_out[idx];
    }
    return 0;
}

void akiko_write(uaecptr addr, uae_u32 value)
{
    uaecptr offset = addr & 0xFFFF;
    if (offset >= 0x38 && offset < 0x58) {
        int idx = (offset - 0x38) / 4;
        akiko_c2p_buffer[idx] = value;
        if (idx == 7) {
            akiko_perform_c2p();
        }
    }
}

int akiko_bget(uaecptr addr)
{
    return (int)(akiko_read(addr) >> (24 - 8 * (addr & 3))) & 0xFF;
}

int akiko_wget(uaecptr addr)
{
    return (int)(akiko_read(addr) >> (16 - 8 * (addr & 2))) & 0xFFFF;
}

int akiko_lget(uaecptr addr)
{
    return (int)akiko_read(addr);
}

void akiko_bput(uaecptr addr, int val)
{
    akiko_write(addr, (uae_u32)val);
}

void akiko_wput(uaecptr addr, int val)
{
    akiko_write(addr, (uae_u32)val);
}

void akiko_lput(uaecptr addr, int val)
{
    akiko_write(addr, (uae_u32)val);
}
