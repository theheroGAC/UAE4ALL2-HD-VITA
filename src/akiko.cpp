#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sysconfig.h"
#include "sysdeps.h"
#include "config.h"
#include "memory-uae.h"
#include "custom.h"
#include "savestate.h"
#include "cdrom.h"
#include "akiko.h"

#define AKIKO_INT_SUBCODE       0x80000000
#define AKIKO_INT_PIO_TX_DONE   0x40000000
#define AKIKO_INT_PIO_RX_READY  0x20000000
#define AKIKO_INT_RX_DMA_DONE   0x10000000
#define AKIKO_INT_TX_DMA_DONE   0x08000000
#define AKIKO_INT_DATA_DONE     0x04000000
#define AKIKO_INT_OVERFLOW      0x02000000

#define AKIKO_FLAG_SUBCODE      0x80000000
#define AKIKO_FLAG_TX_DMA       0x40000000
#define AKIKO_FLAG_RX_DMA       0x20000000
#define AKIKO_FLAG_MEMORY_MODE  0x10000000
#define AKIKO_FLAG_DATA_DMA     0x08000000
#define AKIKO_FLAG_ENABLE       0x04000000
#define AKIKO_FLAG_RAW          0x02000000

#define AKIKO_DATA_BLOCK_SIZE 0x1000
#define AKIKO_COMMAND_BUFFER_SIZE 32
#define AKIKO_RESULT_BUFFER_SIZE 64

uae_u8 akiko_buffer[8192];
uae_u32 akiko_bplcon = 0;

static uae_u32 akiko_c2p_buffer[8];
static uae_u32 akiko_c2p_out[8];
static int akiko_c2p_read_offset;

static uae_u32 cd_intreq;
static uae_u32 cd_intena;
static uae_u32 cd_data_address;
static uae_u32 cd_misc_address;
static uae_u32 cd_flags;
static uae_u16 cd_data_blocks;
static uae_u8 cd_subcode_offset;
static uae_u8 cd_tx_index;
static uae_u8 cd_tx_end;
static uae_u8 cd_rx_index;
static uae_u8 cd_rx_end;
static uae_u8 cd_command[AKIKO_COMMAND_BUFFER_SIZE];
static int cd_command_length;
static int cd_command_expected;
static uae_u8 cd_result[AKIKO_RESULT_BUFFER_SIZE];
static int cd_result_length;
static int cd_result_offset;
static int cd_current_sector;
static int cd_sectors_remaining;
static int cd_irq_asserted;

static void akiko_update_irq(void)
{
    int active = (cd_intreq & cd_intena) != 0;
    if (active && !cd_irq_asserted) {
        INTREQ(0x8000 | 0x2000);
        cd_irq_asserted = 1;
    } else if (!active && cd_irq_asserted) {
        INTREQ(0x2000);
        cd_irq_asserted = 0;
    }
}

static int bcd_to_int(uae_u8 value)
{
    return ((value >> 4) * 10) + (value & 0x0f);
}

static int command_size(uae_u8 command)
{
    switch (command & 0x0f) {
        case 1: return 1;
        case 2: return 1;
        case 3: return 1;
        case 4: return 12;
        case 5: return 2;
        case 6: return 1;
        case 7: return 1;
        default: return 1;
    }
}

static int command_msf_to_lba(const uae_u8 *command)
{
    int minutes = bcd_to_int(command[0]);
    int seconds = bcd_to_int(command[1]);
    int frames = bcd_to_int(command[2]);
    int lba = (minutes * 60 * 75) + (seconds * 75) + frames;
    return lba >= 150 ? lba - 150 : lba;
}

static void akiko_perform_c2p(void)
{
    int i, j;
    for (i = 0; i < 8; i++) akiko_c2p_out[i] = 0;
    for (i = 0; i < 32; i++) {
        uae_u32 value = 0;
        for (j = 0; j < 8; j++) {
            if (akiko_c2p_buffer[j] & (1u << (31 - i)))
                value |= 1u << j;
        }
        akiko_c2p_out[i / 4] |= value << ((3 - (i & 3)) * 8);
    }
    akiko_c2p_read_offset = 0;
}

static void akiko_c2p_write_byte(uaecptr offset, uae_u8 value)
{
    int relative = (int)offset - 0x38;
    int index;
    int shift;
    if (relative < 0 || relative >= 0x20) return;
    index = relative / 4;
    shift = (3 - (relative & 3)) * 8;
    if ((relative & 3) == 0)
        akiko_c2p_buffer[index] = 0;
    akiko_c2p_buffer[index] |= (uae_u32)value << shift;
    if (index == 7 && (relative & 3) == 3)
        akiko_perform_c2p();
}

static uae_u8 akiko_c2p_read_byte(uaecptr offset)
{
    int relative = (int)offset - 0x38;
    int index;
    int shift;
    if (relative < 0 || relative >= 0x20) return 0;
    index = akiko_c2p_read_offset;
    shift = (3 - (relative & 3)) * 8;
    if (index < 0 || index >= 8) return 0;
    if ((relative & 3) == 3)
        akiko_c2p_read_offset = (akiko_c2p_read_offset + 1) & 7;
    return (uae_u8)(akiko_c2p_out[index] >> shift);
}

static void akiko_set_result(const uae_u8 *data, int length)
{
    int i;
    uae_u8 checksum = 0;
    if (!data || length <= 0) return;
    if (length > AKIKO_RESULT_BUFFER_SIZE - 1)
        length = AKIKO_RESULT_BUFFER_SIZE - 1;
    memcpy(cd_result, data, length);
    for (i = 0; i < length; i++) checksum = (uae_u8)(checksum + cd_result[i]);
    cd_result[length++] = (uae_u8)(0xff - checksum);
    cd_result_length = length;
    cd_result_offset = 0;
    if (cd_flags & AKIKO_FLAG_RX_DMA)
        cd_intreq |= AKIKO_INT_RX_DMA_DONE;
    else
        cd_intreq |= AKIKO_INT_PIO_RX_READY;
    akiko_update_irq();
}

static void akiko_command_complete(void)
{
    uae_u8 response[32];
    int length = 2;
    int command = cd_command[0] & 0x0f;
    int i;

    memset(response, 0, sizeof(response));
    response[0] = cd_command[0];

    switch (command) {
        case 1:
            cd_sectors_remaining = 0;
            cdrom_audio_stop();
            response[1] = cdrom_is_inserted ? 1 : 0xf8;
            break;
        case 2:
            cdrom_audio_pause(1);
            response[1] = cdrom_is_inserted ? 1 : 0xf8;
            break;
        case 3:
            cdrom_audio_pause(0);
            response[1] = cdrom_is_inserted ? 1 : 0xf8;
            break;
        case 4:
            if (!cdrom_is_inserted) {
                response[1] = 0xf8;
            } else {
                int start = command_msf_to_lba(cd_command + 1);
                int end = command_msf_to_lba(cd_command + 4);
                if (start < 0) start = 0;
                if (end <= start) end = start + 1;
                if ((uae_u32)start >= cdrom_get_capacity()) {
                    response[1] = 0xa0;
                } else {
                    if ((uae_u32)end > cdrom_get_capacity()) end = (int)cdrom_get_capacity();
                    cd_current_sector = start;
                    cd_sectors_remaining = end - start;
                    if (cd_command[7] & 0x80)
                        cdrom_audio_stop();
                    else {
                        cd_sectors_remaining = 0;
                        cdrom_audio_start((uae_u32)start, (uae_u32)end);
                    }
                    response[1] = 1 | (cdrom_is_audio_lba((uae_u32)start) ? 0x08 : 0);
                }
            }
            break;
        case 5:
            response[1] = (cd_command[1] & 1) ? 1 : 0;
            length = (cd_command[1] & 0x80) ? 2 : 0;
            break;
        case 6:
            response[1] = cdrom_is_inserted ? 1 : 0xf8;
            length = 14;
            if (!cdrom_get_subcode((uae_u32)cd_current_sector, response + 2))
                memset(response + 2, 0, 12);
            break;
        case 7:
            response[1] = cdrom_is_inserted ? 1 : 0;
            memcpy(response + 2, "CHINON O-658-2 24", 17);
            length = 20;
            break;
        default:
            response[1] = 0x80;
            break;
    }

    cd_command_length = 0;
    cd_command_expected = 0;
    if (length > 0) akiko_set_result(response, length);
    else {
        cd_result_length = 0;
        cd_result_offset = 0;
        cd_intreq &= ~AKIKO_INT_PIO_RX_READY;
        akiko_update_irq();
    }
    if (cd_flags & AKIKO_FLAG_TX_DMA)
        cd_intreq |= AKIKO_INT_TX_DMA_DONE;
    else
        cd_intreq |= AKIKO_INT_PIO_TX_DONE;
    akiko_update_irq();
}

static void akiko_receive_command_byte(uae_u8 value)
{
    if (cd_command_length >= AKIKO_COMMAND_BUFFER_SIZE)
        cd_command_length = 0;
    cd_command[cd_command_length++] = value;
    if (cd_command_length == 1)
        cd_command_expected = command_size(value) + 1;
    if (cd_command_expected > 0 && cd_command_length >= cd_command_expected)
        akiko_command_complete();
}

static void akiko_process_result_dma(void)
{
    if (!(cd_flags & AKIKO_FLAG_RX_DMA) || cd_result_offset >= cd_result_length)
        return;
    while (cd_result_offset < cd_result_length && cd_rx_index != cd_rx_end) {
        put_byte(cd_misc_address + cd_rx_index, cd_result[cd_result_offset++]);
        cd_rx_index++;
    }
    if (cd_result_offset >= cd_result_length) {
        cd_result_length = 0;
        cd_result_offset = 0;
        cd_intreq &= ~AKIKO_INT_RX_DMA_DONE;
    }
    if (cd_rx_index == cd_rx_end) {
        cd_intreq |= AKIKO_INT_RX_DMA_DONE;
        akiko_update_irq();
    }
}

static void akiko_process_command_dma(void)
{
    if (!(cd_flags & AKIKO_FLAG_TX_DMA) || cd_tx_index == cd_tx_end)
        return;
    while (cd_tx_index != cd_tx_end) {
        akiko_receive_command_byte((uae_u8)get_byte(cd_misc_address + cd_tx_index));
        cd_tx_index++;
    }
    cd_intreq |= AKIKO_INT_TX_DMA_DONE;
    akiko_update_irq();
}

static void akiko_process_data_dma(void)
{
    int slot;
    uae_u8 raw[2352];
    if (!(cd_flags & AKIKO_FLAG_ENABLE) || !(cd_flags & AKIKO_FLAG_DATA_DMA))
        return;
    if (!cdrom_is_inserted || cd_sectors_remaining <= 0)
        return;
    for (slot = 15; slot >= 0; slot--) {
        if (cd_data_blocks & (1u << slot)) break;
    }
    if (slot < 0) return;
    if (!cdrom_read_raw_sector((uae_u32)cd_current_sector, raw)) {
        cd_intreq |= AKIKO_INT_OVERFLOW;
        akiko_update_irq();
        return;
    }
    raw[0] = raw[1] = raw[2] = 0;
    raw[3] = (uae_u8)((cd_current_sector) & 31);
    for (int i = 0; i < 2352; i++)
        put_byte(cd_data_address + (uaecptr)slot * AKIKO_DATA_BLOCK_SIZE + i, raw[i]);
    cd_data_blocks &= (uae_u16)~(1u << slot);
    cd_current_sector++;
    cd_sectors_remaining--;
    cd_intreq |= AKIKO_INT_DATA_DONE;
    akiko_update_irq();
}

static void akiko_process(void)
{
    akiko_process_command_dma();
    akiko_process_result_dma();
    akiko_process_data_dma();
}

static uae_u8 akiko_reg_read_byte(uaecptr address)
{
    uae_u32 value = 0;
    int offset = (int)(address & 0x3f);
    switch (offset) {
        case 0x00: return 0xc0;
        case 0x01: return 0xca;
        case 0x02: return 0xca;
        case 0x03: return 0xfe;
        case 0x04: case 0x05: case 0x06: case 0x07:
            value = cd_intreq; return (uae_u8)(value >> ((3 - (offset - 4)) * 8));
        case 0x08: case 0x09: case 0x0a: case 0x0b:
            value = cd_intena; return (uae_u8)(value >> ((3 - (offset - 8)) * 8));
        case 0x10: case 0x11: case 0x12: case 0x13:
            value = cd_data_address; return (uae_u8)(value >> ((3 - (offset - 0x10)) * 8));
        case 0x14: case 0x15: case 0x16: case 0x17:
            value = cd_misc_address; return (uae_u8)(value >> ((3 - (offset - 0x14)) * 8));
        case 0x18: return cd_subcode_offset;
        case 0x1d: return cd_tx_index;
        case 0x1e: return cd_rx_index;
        case 0x20: return (uae_u8)(cd_data_blocks >> 8);
        case 0x21: return (uae_u8)cd_data_blocks;
        case 0x24: case 0x25: case 0x26: case 0x27:
            value = cd_flags; return (uae_u8)(value >> ((3 - (offset - 0x24)) * 8));
        case 0x28:
            if (cd_result_offset < cd_result_length)
                value = cd_result[cd_result_offset++];
            if (cd_result_offset >= cd_result_length) {
                cd_result_offset = 0;
                cd_result_length = 0;
                cd_intreq &= ~AKIKO_INT_PIO_RX_READY;
                akiko_update_irq();
            }
            return (uae_u8)value;
        case 0x38: case 0x39: case 0x3a: case 0x3b:
            return akiko_c2p_read_byte((uaecptr)offset);
        default:
            return 0;
    }
}

static void akiko_reg_write_byte(uaecptr address, uae_u8 value)
{
    int offset = (int)(address & 0x3f);
    uae_u32 mask;
    switch (offset) {
        case 0x08: case 0x09: case 0x0a: case 0x0b:
            mask = 0xffu << ((3 - (offset - 8)) * 8);
            cd_intena = (cd_intena & ~mask) | ((uae_u32)value << ((3 - (offset - 8)) * 8));
            akiko_update_irq();
            break;
        case 0x10: case 0x11: case 0x12: case 0x13:
            mask = 0xffu << ((3 - (offset - 0x10)) * 8);
            cd_data_address = (cd_data_address & ~mask) | ((uae_u32)value << ((3 - (offset - 0x10)) * 8));
            cd_data_address &= 0x00fff000;
            break;
        case 0x14: case 0x15: case 0x16: case 0x17:
            mask = 0xffu << ((3 - (offset - 0x14)) * 8);
            cd_misc_address = (cd_misc_address & ~mask) | ((uae_u32)value << ((3 - (offset - 0x14)) * 8));
            cd_misc_address &= 0x00fffc00;
            break;
        case 0x18:
            cd_intreq &= ~AKIKO_INT_SUBCODE;
            cd_subcode_offset = 0;
            akiko_update_irq();
            break;
        case 0x1d:
            cd_intreq &= ~AKIKO_INT_TX_DMA_DONE;
            cd_tx_end = value;
            akiko_update_irq();
            break;
        case 0x1f:
            cd_intreq &= ~AKIKO_INT_RX_DMA_DONE;
            cd_rx_end = value;
            akiko_update_irq();
            break;
        case 0x20:
            if (cd_flags & AKIKO_FLAG_ENABLE) cd_data_blocks |= (uae_u16)value << 8;
            cd_intreq &= ~AKIKO_INT_DATA_DONE;
            akiko_update_irq();
            break;
        case 0x21:
            if (cd_flags & AKIKO_FLAG_ENABLE) cd_data_blocks |= value;
            cd_intreq &= ~AKIKO_INT_DATA_DONE;
            akiko_update_irq();
            break;
        case 0x24: case 0x25: case 0x26: case 0x27:
            mask = 0xffu << ((3 - (offset - 0x24)) * 8);
            cd_flags = (cd_flags & ~mask) | ((uae_u32)value << ((3 - (offset - 0x24)) * 8));
            break;
        case 0x28:
            if (!(cd_flags & AKIKO_FLAG_TX_DMA))
                akiko_receive_command_byte(value);
            cd_intreq &= ~AKIKO_INT_PIO_TX_DONE;
            akiko_update_irq();
            break;
        case 0x38: case 0x39: case 0x3a: case 0x3b:
            akiko_c2p_write_byte((uaecptr)offset, value);
            break;
        default:
            break;
    }
    akiko_process();
}

void akiko_init(void)
{
    memset(akiko_buffer, 0, sizeof(akiko_buffer));
    memset(akiko_c2p_buffer, 0, sizeof(akiko_c2p_buffer));
    memset(akiko_c2p_out, 0, sizeof(akiko_c2p_out));
    memset(cd_command, 0, sizeof(cd_command));
    memset(cd_result, 0, sizeof(cd_result));
    akiko_c2p_read_offset = 0;
    cd_intreq = 0;
    cd_intena = 0;
    cd_data_address = 0;
    cd_misc_address = 0;
    cd_flags = 0;
    cd_data_blocks = 0;
    cd_subcode_offset = 0;
    cd_tx_index = cd_tx_end = 0;
    cd_rx_index = cd_rx_end = 0;
    cd_command_length = 0;
    cd_command_expected = 0;
    cd_result_length = 0;
    cd_result_offset = 0;
    cd_current_sector = 0;
    cd_sectors_remaining = 0;
    cd_irq_asserted = 0;
}

void akiko_reset(void)
{
    akiko_init();
}

void akiko_hsync_handler(void)
{
    akiko_process();
}

uae_u32 akiko_read(uaecptr addr)
{
    return akiko_lget(addr);
}

void akiko_write(uaecptr addr, uae_u32 value)
{
    akiko_lput(addr, value);
}

int akiko_bget(uaecptr addr)
{
    return akiko_reg_read_byte(addr);
}

int akiko_wget(uaecptr addr)
{
    return (akiko_reg_read_byte(addr) << 8) | akiko_reg_read_byte(addr + 1);
}

int akiko_lget(uaecptr addr)
{
    return ((uae_u32)akiko_reg_read_byte(addr) << 24) |
           ((uae_u32)akiko_reg_read_byte(addr + 1) << 16) |
           ((uae_u32)akiko_reg_read_byte(addr + 2) << 8) |
           (uae_u32)akiko_reg_read_byte(addr + 3);
}

void akiko_bput(uaecptr addr, int val)
{
    akiko_reg_write_byte(addr, (uae_u8)val);
}

void akiko_wput(uaecptr addr, int val)
{
    akiko_reg_write_byte(addr, (uae_u8)(val >> 8));
    akiko_reg_write_byte(addr + 1, (uae_u8)val);
}

void akiko_lput(uaecptr addr, int val)
{
    akiko_reg_write_byte(addr, (uae_u8)((uae_u32)val >> 24));
    akiko_reg_write_byte(addr + 1, (uae_u8)((uae_u32)val >> 16));
    akiko_reg_write_byte(addr + 2, (uae_u8)((uae_u32)val >> 8));
    akiko_reg_write_byte(addr + 3, (uae_u8)val);
}

uae_u8 *akiko_save_state(int *length)
{
    uae_u8 *buffer = (uae_u8 *)malloc(1024);
    uae_u8 *dst = buffer;
    uae_u32 audio_start;
    uae_u32 audio_end;
    uae_u32 audio_phase;
    int audio_playing;
    int audio_paused;
    int i;

    if (!buffer) return NULL;
    cdrom_audio_get_state(&audio_start, &audio_end, &audio_phase, &audio_playing, &audio_paused);
    save_u32(1);
    save_u32(cd_intreq);
    save_u32(cd_intena);
    save_u32(cd_data_address);
    save_u32(cd_misc_address);
    save_u32(cd_flags);
    save_u16(cd_data_blocks);
    save_u8(cd_subcode_offset);
    save_u8(cd_tx_index);
    save_u8(cd_tx_end);
    save_u8(cd_rx_index);
    save_u8(cd_rx_end);
    save_u32((uae_u32)cd_command_length);
    save_u32((uae_u32)cd_command_expected);
    save_u32((uae_u32)cd_result_length);
    save_u32((uae_u32)cd_result_offset);
    save_u32((uae_u32)cd_current_sector);
    save_u32((uae_u32)cd_sectors_remaining);
    for (i = 0; i < AKIKO_COMMAND_BUFFER_SIZE; i++) save_u8(cd_command[i]);
    for (i = 0; i < AKIKO_RESULT_BUFFER_SIZE; i++) save_u8(cd_result[i]);
    save_string(current_cd_image);
    save_u32(audio_start);
    save_u32(audio_end);
    save_u32(audio_phase);
    save_u8((uae_u8)audio_playing);
    save_u8((uae_u8)audio_paused);
    if (length) *length = (int)(dst - buffer);
    return buffer;
}

uae_u8 *akiko_restore_state(uae_u8 *source)
{
    uae_u8 *src = source;
    char *image;
    uae_u32 audio_start;
    uae_u32 audio_end;
    uae_u32 audio_phase;
    int audio_playing;
    int audio_paused;
    int i;

    if (!source || restore_u32() != 1) return source;
    cd_intreq = restore_u32();
    cd_intena = restore_u32();
    cd_data_address = restore_u32();
    cd_misc_address = restore_u32();
    cd_flags = restore_u32();
    cd_data_blocks = restore_u16();
    cd_subcode_offset = restore_u8();
    cd_tx_index = restore_u8();
    cd_tx_end = restore_u8();
    cd_rx_index = restore_u8();
    cd_rx_end = restore_u8();
    cd_command_length = (int)restore_u32();
    cd_command_expected = (int)restore_u32();
    cd_result_length = (int)restore_u32();
    cd_result_offset = (int)restore_u32();
    cd_current_sector = (int)restore_u32();
    cd_sectors_remaining = (int)restore_u32();
    for (i = 0; i < AKIKO_COMMAND_BUFFER_SIZE; i++) cd_command[i] = restore_u8();
    for (i = 0; i < AKIKO_RESULT_BUFFER_SIZE; i++) cd_result[i] = restore_u8();
    image = restore_string();
    if (image && image[0]) cdrom_open_image(image);
    else cdrom_close_image();
    free(image);
    audio_start = restore_u32();
    audio_end = restore_u32();
    audio_phase = restore_u32();
    audio_playing = restore_u8();
    audio_paused = restore_u8();
    cdrom_audio_set_state(audio_start, audio_end, audio_phase, audio_playing, audio_paused);
    akiko_update_irq();
    return src;
}
