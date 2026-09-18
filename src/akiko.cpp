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
#include "gui.h"

#define CDINTERRUPT_SUBCODE     0x80000000
#define CDINTERRUPT_DRIVEXMIT   0x40000000
#define CDINTERRUPT_DRIVERECV   0x20000000
#define CDINTERRUPT_RXDMADONE   0x10000000
#define CDINTERRUPT_TXDMADONE   0x08000000
#define CDINTERRUPT_PBX         0x04000000
#define CDINTERRUPT_OVERFLOW    0x02000000

#define CDFLAG_SUBCODE          0x80000000
#define CDFLAG_TXD              0x40000000
#define CDFLAG_RXD              0x20000000
#define CDFLAG_CAS              0x10000000
#define CDFLAG_PBX              0x08000000
#define CDFLAG_ENABLE           0x04000000
#define CDFLAG_RAW              0x02000000
#define CDFLAG_MSB              0x01000000
#define CDFLAG_NTSC             0x00800000

uae_u8 akiko_buffer[8192];
uae_u32 akiko_bplcon = 0;

static uae_u32 akiko_c2p_buffer[8];
static uae_u32 akiko_c2p_out[8];
static int akiko_c2p_read_offset = -1;
static int akiko_c2p_write_offset = 0;

static uae_u32 cdrom_intreq;
static uae_u32 cdrom_intena;
static uae_u32 cdrom_addressdata;
static uae_u32 cdrom_addressmisc;
static uae_u32 subcode_address;
static uae_u32 cdrx_address;
static uae_u32 cdtx_address;
static uae_u32 cdrom_flags;
static uae_u16 cdrom_pbx;
static uae_u8 cdrom_subcodeoffset;

static uae_u8 cdcomtxinx;
static uae_u8 cdcomtxcmp;
static uae_u8 cdcomrxinx;
static uae_u8 cdcomrxcmp;

static uae_u8 cdrom_command_buffer[32];
static int cdrom_command_length;
static uae_u8 cdrom_command;
static int cdrom_checksum_error;
static int cdrom_unknown_command;

static uae_u8 cdrom_result_buffer[32];
static int cdrom_receive_length;
static int cdrom_receive_offset;
static uae_u8 cdrom_last_rx;

static int cdrom_data_offset;
static int cdrom_sector_counter;
static int cdrom_speed;
static int cdrom_seek_delay;
static int cdrom_toc_counter;
static int cd_initialized;
static int cd_irq_asserted;
static int cdrom_command_active;
static int cdrom_tx_dma_delay;
static int cdrom_rx_dma_delay;
static int cdrom_audiotimeout;
static int cdrom_playing;

static const int command_lengths[] = { 1, 2, 1, 1, 12, 2, 1, 1, 4, 1, 2, -1, -1, -1, -1, -1 };

static void akiko_update_irq(void)
{
    int active = (cdrom_intreq & cdrom_intena) != 0;
    if (active && !cd_irq_asserted) {
        INTREQ(0x8000 | 0x2000);
        cd_irq_asserted = 1;
    } else if (!active && cd_irq_asserted) {
        INTREQ(0x2000);
        cd_irq_asserted = 0;
    }
}

static void set_status(uae_u32 status)
{
    cdrom_intreq |= status;
    akiko_update_irq();
}

static int bcd_to_int(uae_u8 value)
{
    return ((value >> 4) * 10) + (value & 0x0f);
}

static uae_u8 int_to_bcd(int value)
{
    return (uae_u8)(((value / 10) << 4) | (value % 10));
}

static int command_msf_to_lsn(const uae_u8 *command)
{
    int minutes = bcd_to_int(command[0]);
    int seconds = bcd_to_int(command[1]);
    int frames = bcd_to_int(command[2]);
    return (minutes * 60 * 75) + (seconds * 75) + frames - 150;
}

static void akiko_c2p_do(void)
{
    int i;
    for (i = 0; i < 8; i++)
        akiko_c2p_out[i] = 0;
    for (i = 0; i < 8 * 32; i++) {
        if (akiko_c2p_buffer[7 - (i >> 5)] & (1u << (i & 31)))
            akiko_c2p_out[i & 7] |= 1u << (i >> 3);
    }
}

static void akiko_c2p_write(int offset, uae_u32 v)
{
    if (offset == 3)
        akiko_c2p_buffer[akiko_c2p_write_offset] = 0;
    akiko_c2p_buffer[akiko_c2p_write_offset] |= v << (8 * (3 - offset));
    if (offset == 0) {
        akiko_c2p_write_offset++;
        akiko_c2p_write_offset &= 7;
    }
    akiko_c2p_read_offset = -1;
}

static uae_u32 akiko_c2p_read(int offset)
{
    if (akiko_c2p_read_offset < 0) {
        akiko_c2p_do();
        akiko_c2p_read_offset = 0;
    }
    akiko_c2p_write_offset = 0;
    uae_u32 v = akiko_c2p_out[akiko_c2p_read_offset];
    return v >> (8 * (3 - offset));
}

static void check_read_c2p(uaecptr addr)
{
    addr &= 0x3f;
    if (addr < 0x38 || addr >= 0x3c)
        return;
    akiko_c2p_read_offset++;
    akiko_c2p_read_offset &= 7;
}

static int akiko_toc_entry(int index, uae_u8 *entry)
{
    CdromTrackInfo info;
    int track_count = cdrom_get_track_count();
    int first_track = 1;
    int last_track = track_count;
    int point;
    int address;
    int first_track_audio = 0;

    if (!entry || track_count <= 0) return 0;
    gui_data.cdled = HDLED_READ;
    if (cdrom_get_track_info(0, &info) && info.audio)
        first_track_audio = 1;

    memset(entry, 0, 13);
    if (index == 0) {
        point = 0xa0;
        entry[1] = first_track_audio ? 0x01 : 0x41;
        entry[8] = int_to_bcd(first_track);
        entry[9] = 0;
        entry[10] = 0;
    } else if (index == 1) {
        point = 0xa1;
        entry[1] = first_track_audio ? 0x01 : 0x41;
        entry[8] = int_to_bcd(last_track);
        entry[9] = 0;
        entry[10] = 0;
    } else if (index == 2) {
        point = 0xa2;
        entry[1] = first_track_audio ? 0x01 : 0x41;
        address = (int)cdrom_get_capacity() + 150;
        entry[8] = int_to_bcd(address / (60 * 75));
        entry[9] = int_to_bcd((address / 75) % 60);
        entry[10] = int_to_bcd(address % 75);
    } else {
        if (index - 3 >= track_count || !cdrom_get_track_info(index - 3, &info)) return 0;
        point = info.number;
        entry[1] = info.audio ? 0x01 : 0x41;
        address = (int)info.start_lba + 150;
        entry[8] = int_to_bcd(address / (60 * 75));
        entry[9] = int_to_bcd((address / 75) % 60);
        entry[10] = int_to_bcd(address % 75);
    }
    entry[3] = point < 100 ? int_to_bcd(point) : (uae_u8)point;
    return 1;
}

static int cdrom_return_toc_entry(void)
{
    cdrom_result_buffer[0] = 6;
    if (!cdrom_is_inserted || cdrom_get_track_count() <= 0) {
        cdrom_result_buffer[1] = 0x80 | 1;
        return 15;
    }
    cdrom_result_buffer[1] = 0x0a;
    akiko_toc_entry(cdrom_toc_counter / 3, cdrom_result_buffer + 2);
    cdrom_result_buffer[6] = int_to_bcd(99);
    cdrom_result_buffer[7] = int_to_bcd(24 + cdrom_toc_counter / 75);
    cdrom_result_buffer[8] = int_to_bcd(cdrom_toc_counter % 75);
    cdrom_toc_counter++;
    if (cdrom_toc_counter / 3 >= cdrom_get_track_count() + 3)
        cdrom_toc_counter = -1;
    return 15;
}

static bool cdrom_can_return_data(void)
{
    return cdrom_receive_length == 0;
}

static int cdrom_start_return_data(int len)
{
    if (!cdrom_can_return_data())
        return 0;
    if (len <= 0)
        return -1;
    cdrom_receive_length = len;
    uae_u8 checksum = 0xff;
    for (int i = 0; i < cdrom_receive_length; i++) {
        checksum -= cdrom_result_buffer[i];
    }
    cdrom_result_buffer[cdrom_receive_length++] = checksum;
    cdrom_receive_offset = 0;
    set_status(CDINTERRUPT_DRIVERECV);
    return 1;
}

static void cdrom_return_data(void)
{
    uae_u32 cmd_buf = cdrx_address;

    if (!cdrom_receive_length)
        return;
    if (!(cdrom_flags & CDFLAG_RXD))
        return;
    if (cdcomrxinx == cdcomrxcmp)
        return;
    if (cdrom_rx_dma_delay > 0)
        return;

    while (cdrom_receive_offset < cdrom_receive_length) {
        cdrom_last_rx = cdrom_result_buffer[cdrom_receive_offset];
        put_byte(cmd_buf + cdcomrxinx, cdrom_last_rx);
        cdcomrxinx++;
        cdrom_receive_offset++;
        if (cdcomrxinx == cdcomrxcmp) {
            set_status(CDINTERRUPT_RXDMADONE);
            break;
        }
    }
    if (cdrom_receive_offset == cdrom_receive_length) {
        cdrom_receive_length = 0;
        cdrom_receive_offset = 0;
        cdrom_intreq &= ~CDINTERRUPT_DRIVERECV;
        set_status(CDINTERRUPT_DRIVEXMIT);
    }
}

static bool can_send_command(void)
{
    if (!cd_initialized)
        return false;
    if (cdrom_command_active)
        return false;
    if (cdrom_receive_length > 0)
        return false;
    return true;
}

static void cdrom_run_command_run(void)
{
    int len = 0;
    int cmd = cdrom_command & 0x0f;

    cdrom_command_length = 0;
    cdrom_command_active = 0;

    memset(cdrom_result_buffer, 0, sizeof(cdrom_result_buffer));

    if (cdrom_checksum_error || cdrom_unknown_command) {
        write_log("[AKIKO] command error: checksum=%d unknown=%d cmd=%02x\n",
            cdrom_checksum_error, cdrom_unknown_command, cdrom_command);
        cdrom_result_buffer[0] = (cdrom_command & 0xf0) | 5;
        cdrom_result_buffer[1] = 0x80 | 1;
        cdrom_checksum_error = 0;
        cdrom_unknown_command = 0;
        cdrom_command_length = 0;
        cdrom_start_return_data(2);
        return;
    }

    cdrom_result_buffer[0] = cdrom_command;

    switch (cmd) {
        case 0:
            len = 1;
            break;
        case 1: // STOP
            cdrom_audio_stop();
            cdrom_audiotimeout = 0;
            cdrom_playing = 0;
            cdrom_data_offset = -1;
            cdrom_sector_counter = 0;
            cdrom_result_buffer[1] = cdrom_is_inserted ? 0 : 0xf8;
            len = 2;
            break;
        case 2: // PAUSE
            cdrom_audio_pause(1);
            cdrom_audiotimeout = 0;
            cdrom_result_buffer[1] = cdrom_is_inserted ? (cdrom_playing ? 0x08 : 0x00) : 0xf8;
            len = 2;
            break;
        case 3: // UNPAUSE
            cdrom_audio_pause(0);
            cdrom_audiotimeout = 0;
            cdrom_result_buffer[1] = cdrom_is_inserted ? (cdrom_playing ? 0x08 : 0x00) : 0xf8;
            len = 2;
            break;
        case 4: { // MULTI (Read / Play / Read TOC)
            int seekpos = command_msf_to_lsn(cdrom_command_buffer + 1);
            int endpos = command_msf_to_lsn(cdrom_command_buffer + 4);
            cdrom_speed = (cdrom_command_buffer[8] & 0x40) ? 2 : 1;
            cdrom_result_buffer[1] = 0;
            if (!cdrom_is_inserted) {
                cdrom_result_buffer[1] = 1;
                len = 2;
            } else if (cdrom_command_buffer[7] & 0x80) { // DATA READ
                cdrom_audio_stop();
                cdrom_audiotimeout = 0;
                cdrom_playing = 0;
                cdrom_data_offset = seekpos;
                cdrom_sector_counter = 0;
                cdrom_result_buffer[1] |= 0x02;
                gui_data.cdled = HDLED_READ;
                write_log("[AKIKO] DATA READ seekpos=%d (%06x) endpos=%d speed=%d\n",
                    seekpos, seekpos, endpos, cdrom_speed);
                len = 2;
            } else { // PLAY AUDIO or READ TOC
                if (seekpos < 0) {
                    cdrom_toc_counter = 0;
                    cdrom_data_offset = -1;
                    cdrom_audio_stop();
                    cdrom_audiotimeout = 0;
                    cdrom_playing = 0;
                    cdrom_result_buffer[1] = 0x00;
                    gui_data.cdled = HDLED_READ;
                    write_log("[AKIKO] READ TOC requested\n");
                    len = 2;
                } else {
                    cdrom_toc_counter = -1;
                    cdrom_data_offset = -1;
                    if (endpos <= seekpos) endpos = seekpos + 1;
                    cdrom_audio_start((uae_u32)seekpos, (uae_u32)endpos);
                    cdrom_result_buffer[1] = 0x42;
                    cdrom_playing = 1;
                    cdrom_audiotimeout = 10;
                    write_log("[AKIKO] PLAY AUDIO from %d to %d\n", seekpos, endpos);
                    len = 2;
                }
            }
            break;
        }
        case 5: { // LED
            int v = cdrom_command_buffer[1];
            if (v & 0x80) {
                cdrom_result_buffer[1] = (v & 1) ? 1 : 0;
                len = 2;
            } else {
                len = 0;
            }
            break;
        }
        case 6: { // SUBCODE
            cdrom_result_buffer[1] = 0;
            uae_u32 lba = 0;
            if (cdrom_audio_is_playing()) {
                lba = cdrom_get_current_lba();
            } else if (cdrom_data_offset >= 0) {
                lba = (uae_u32)(cdrom_data_offset + cdrom_sector_counter);
            } else {
                lba = cdrom_get_current_lba();
            }
            cdrom_get_subcode(lba, cdrom_result_buffer + 2);
            len = 15;
            break;
        }
        case 7: // INFO / FIRMWARE
            cdrom_result_buffer[1] = cdrom_is_inserted ? 1 : 0;
            memcpy(cdrom_result_buffer + 2, "CHINON  O-658-2 24", 18);
            cd_initialized = 2;
            write_log("[AKIKO] INFO / FIRMWARE returned (inserted=%d)\n", cdrom_is_inserted);
            len = 20;
            break;
        default:
            cdrom_result_buffer[1] = 0x80;
            len = 2;
            break;
    }

    if (len == 0) {
        set_status(CDINTERRUPT_DRIVEXMIT);
    } else {
        cdrom_start_return_data(len);
    }
}

static void cdrom_add_command_byte(uae_u8 b)
{
    cdrom_command_buffer[cdrom_command_length++] = b;
    cdrom_command = cdrom_command_buffer[0];
    int cmd_code = cdrom_command & 0x0f;
    int cmd_len = command_lengths[cmd_code];

    cdrom_checksum_error = 0;
    cdrom_unknown_command = 0;

    if (cmd_len < 0) {
        cdrom_unknown_command = 1;
        cdrom_command_active = 1;
        return;
    }

    if (cmd_len + 1 > cdrom_command_length)
        return;

    uae_u8 checksum = 0;
    for (int i = 0; i < cmd_len + 1; i++) {
        checksum += cdrom_command_buffer[i];
    }
    if (checksum != 0xff) {
        write_log("[AKIKO] command checksum error cmd=%02x sum=%02x\n", cdrom_command, checksum);
        cdrom_checksum_error = 1;
    }
    cdrom_command_active = 1;
    cdrom_command_length = cmd_len;
}

static void cdrom_run_command(void)
{
    if (!(cdrom_flags & CDFLAG_TXD))
        return;
    if (cdrom_flags & CDFLAG_ENABLE)
        return;
    if (cdcomtxinx == cdcomtxcmp)
        return;
    if (cdrom_tx_dma_delay > 0)
        return;
    if (!can_send_command())
        return;

    uae_u8 b = (uae_u8)get_byte(cdtx_address + cdcomtxinx);
    cdrom_add_command_byte(b);
    cdcomtxinx++;

    if (cdcomtxinx == cdcomtxcmp) {
        set_status(CDINTERRUPT_TXDMADONE);
    }
}

static void cdrom_run_read(void)
{
    int secnum;
    if (!(cdrom_flags & CDFLAG_ENABLE)) return;
    if (!cdrom_pbx) return;
    if (!(cdrom_flags & CDFLAG_PBX)) return;
    if (cdrom_data_offset < 0) return;
    if (!cdrom_is_inserted) return;

    for (secnum = 15; secnum >= 0; secnum--) {
        if (cdrom_pbx & (1 << secnum))
            break;
    }
    if (secnum < 0) return;

    int sector = cdrom_data_offset + cdrom_sector_counter;
    if ((uae_u32)sector >= cdrom_get_capacity()) return;

    uae_u8 buf[2352];
    if (!cdrom_read_raw_sector((uae_u32)sector, buf)) {
        write_log("[AKIKO] data read failed sector=%d\n", sector);
        set_status(CDINTERRUPT_OVERFLOW);
        return;
    }
    gui_data.cdled = HDLED_READ;

    buf[0] = 0;
    buf[1] = 0;
    buf[2] = 0;
    buf[3] = (uae_u8)(cdrom_sector_counter & 31);
    for (int i = 0; i < 2352; i++) {
        put_byte(cdrom_addressdata + (uaecptr)secnum * 4096 + i, buf[i]);
    }
    for (int i = 0; i < 73 * 2; i++) {
        put_byte(cdrom_addressdata + (uaecptr)secnum * 4096 + 0xc00 + i, 0);
    }

    if (cdrom_sector_counter < 8 || (sector % 50 == 0)) {
        write_log("[AKIKO] data DMA sector=%d (offset=%d cnt=%d) slot=%d pbx=%04x\n",
            sector, cdrom_data_offset, cdrom_sector_counter, secnum, cdrom_pbx);
    }

    cdrom_pbx &= (uae_u16)~(1 << secnum);
    set_status(CDINTERRUPT_PBX);

    if (cdrom_flags & CDFLAG_SUBCODE) {
        uae_u8 subbuf[16];
        cdrom_get_subcode((uae_u32)sector, subbuf);
        if (cdrom_subcodeoffset >= 128)
            cdrom_subcodeoffset = 0;
        else
            cdrom_subcodeoffset = 128;
        for (int i = 0; i < 96; i++) {
            put_byte(subcode_address + cdrom_subcodeoffset + i, i < 13 ? subbuf[i] : 0);
        }
        put_word(subcode_address + cdrom_subcodeoffset + 96, 0xffff);
        put_word(subcode_address + cdrom_subcodeoffset + 98, 0x0000);
        cdrom_subcodeoffset += 100;
        set_status(CDINTERRUPT_SUBCODE);
    }

    cdrom_sector_counter++;
}

static void akiko_internal(void)
{
    cdrom_return_data();
    cdrom_run_command();
    if (cdrom_command_active > 0) {
        cdrom_command_active--;
        if (!cdrom_command_active)
            cdrom_run_command_run();
    }
}

void akiko_hsync_handler(int vpos)
{
    static int framecounter1 = 0;
    bool framesync = (vpos == 0);

    framecounter1--;
    if (framecounter1 <= 0) {
        if (cdrom_seek_delay <= 0) {
            cdrom_run_read();
        } else {
            cdrom_seek_delay--;
        }
        framecounter1 = (cdrom_speed == 2) ? 104 : 208;
    }

    if (cdrom_tx_dma_delay > 0)
        cdrom_tx_dma_delay--;
    if (cdrom_rx_dma_delay > 0)
        cdrom_rx_dma_delay--;

    akiko_internal();

    if (!cd_initialized) {
        if (cdrom_is_inserted) {
            cdrom_result_buffer[0] = 0x0a;
            cdrom_result_buffer[1] = 0x01;
            cdrom_start_return_data(2);
            write_log("[AKIKO] media status sent (inserted=1)\n");
        }
        cd_initialized = 1;
        akiko_internal();
    } else if (cd_initialized < 2) {
        return;
    } else {
        if (cdrom_playing == 2 && !cdrom_audio_is_playing()) {
            cdrom_playing = 0;
            cdrom_audiotimeout = -2;
        }

        if (cdrom_audiotimeout > 1) {
            cdrom_audiotimeout--;
        } else if (cdrom_audiotimeout == 1 && cdrom_can_return_data() && !cdrom_command_active) {
            if (!cdrom_playing)
                cdrom_playing = 1;
            if (cdrom_playing == 1) {
                cdrom_result_buffer[0] = 4;
                cdrom_result_buffer[1] = 0x0A;
                cdrom_start_return_data(2);
                akiko_internal();
            }
            cdrom_playing = 2;
            cdrom_audiotimeout = 0;
        } else if (cdrom_audiotimeout == -2 && cdrom_can_return_data() && !cdrom_command_active) {
            cdrom_result_buffer[0] = 4;
            cdrom_result_buffer[1] = 0x04;
            cdrom_start_return_data(2);
            cdrom_audiotimeout = 0;
            akiko_internal();
        } else if (cdrom_audiotimeout == -3 && cdrom_can_return_data() && !cdrom_command_active) {
            cdrom_result_buffer[0] = 4;
            cdrom_result_buffer[1] = 0x80;
            cdrom_start_return_data(2);
            cdrom_audiotimeout = 0;
            akiko_internal();
        } else if (framesync && cdrom_toc_counter >= 0 && !cdrom_command_active && cdrom_can_return_data()) {
            cdrom_start_return_data(cdrom_return_toc_entry());
            akiko_internal();
        }
    }
}

/*
* CD32 1Kb NVRAM (EEPROM) emulation
*
* NVRAM chip is 24C08 CMOS EEPROM (1024x8 bits = 1Kb)
* Chip interface is I2C (2 wire serial)
* Akiko addresses used:
* 0xb80030: bit 7 = SCL (clock), 6 = SDA (data)
* 0xb80032: 0xb80030 data direction register (0 = input, 1 = output)
*/

#ifndef SAVE_PREFIX
#define SAVE_PREFIX "ux0:/data/uae4all/saves/"
#endif

#define BITBANG_I2C_SDA 0
#define BITBANG_I2C_SCL 1
#define NVRAM_PAGE_SIZE 16

typedef enum {
    I2C_STOPPED = 0,
    SENDING_BIT7,
    SENDING_BIT6,
    SENDING_BIT5,
    SENDING_BIT4,
    SENDING_BIT3,
    SENDING_BIT2,
    SENDING_BIT1,
    SENDING_BIT0,
    WAITING_FOR_ACK,
    RECEIVING_BIT7,
    RECEIVING_BIT6,
    RECEIVING_BIT5,
    RECEIVING_BIT4,
    RECEIVING_BIT3,
    RECEIVING_BIT2,
    RECEIVING_BIT1,
    RECEIVING_BIT0,
    SENDING_ACK,
    SENT_NACK
} bitbang_i2c_state;

typedef enum {
    I2C_DEVICEADDR,
    I2C_WORDADDR,
    I2C_DATA
} eeprom_state;

struct bitbang_i2c_interface {
    bitbang_i2c_state state;
    int last_data;
    int last_clock;
    int device_out;
    uint8_t buffer;
    int current_addr;
    uae_u8 device_address, device_address_mask;

    eeprom_state estate;
    int eeprom_addr;
    int size;
    int write_offset;
    int addressbitmask;
    uae_u8 *memory;
};

static uae_u8 cd32_nvram[1024];
static bool cd32_nvram_loaded = false;
static bool cd32_nvram_dirty = false;
static uae_u8 cd32_i2c_direction = 0;
static bool cd32_i2c_data_scl = true, cd32_i2c_data_sda = true;
static struct bitbang_i2c_interface cd32_i2c;

void akiko_nvram_flush(void)
{
    if (!cd32_nvram_dirty) return;
    char path[256];
    snprintf(path, sizeof(path), "%scd32.nvram", SAVE_PREFIX);
    FILE *f = fopen(path, "wb");
    if (f) {
        fwrite(cd32_nvram, 1, 1024, f);
        fclose(f);
        cd32_nvram_dirty = false;
    }
}

static void akiko_nvram_load(void)
{
    char path[256];
    snprintf(path, sizeof(path), "%scd32.nvram", SAVE_PREFIX);
    FILE *f = fopen(path, "rb");
    if (!f) {
        snprintf(path, sizeof(path), "%scd32.nvr", SAVE_PREFIX);
        f = fopen(path, "rb");
    }
    if (f) {
        size_t rd = fread(cd32_nvram, 1, 1024, f);
        fclose(f);
        if (rd < 1024)
            memset(cd32_nvram + rd, 0, 1024 - rd);
    } else {
        memset(cd32_nvram, 0, 1024);
    }
    cd32_nvram_loaded = true;
    cd32_nvram_dirty = false;
}

static void bitbang_i2c_enter_stop(struct bitbang_i2c_interface *i2c)
{
    if (i2c->write_offset >= 0) {
        cd32_nvram_dirty = true;
        akiko_nvram_flush();
    }
    i2c->write_offset = -1;
    i2c->current_addr = -1;
    i2c->state = I2C_STOPPED;
    i2c->estate = I2C_DEVICEADDR;
}

static int bitbang_i2c_ret(struct bitbang_i2c_interface *i2c, int level)
{
    i2c->device_out = level;
    return level & i2c->last_data;
}

static int bitbang_i2c_nop(struct bitbang_i2c_interface *i2c)
{
    return bitbang_i2c_ret(i2c, i2c->device_out);
}

static int eeprom_i2c_set(struct bitbang_i2c_interface *i2c, int line, int level)
{
    int data;

    if (line == BITBANG_I2C_SDA) {
        if (level < 0)
            level = i2c->last_data;
        if (level == i2c->last_data)
            return bitbang_i2c_nop(i2c);
        i2c->last_data = level;
        if (i2c->last_clock == 0)
            return bitbang_i2c_nop(i2c);
        if (level == 0) {
            /* START condition */
            i2c->state = SENDING_BIT7;
            i2c->current_addr = -1;
        } else {
            /* STOP condition */
            bitbang_i2c_enter_stop(i2c);
        }
        return bitbang_i2c_ret(i2c, 1);
    } else {
        if (level < 0)
            level = i2c->last_clock;
    }

    data = i2c->last_data;
    if (i2c->last_clock == level)
        return bitbang_i2c_nop(i2c);
    i2c->last_clock = level;
    if (level == 0) {
        /* Line released at end of pulse */
        return bitbang_i2c_ret(i2c, 1);
    }

    switch (i2c->state) {
    case I2C_STOPPED:
    case SENT_NACK:
        return bitbang_i2c_ret(i2c, 1);

    case SENDING_BIT7:
    case SENDING_BIT6:
    case SENDING_BIT5:
    case SENDING_BIT4:
    case SENDING_BIT3:
    case SENDING_BIT2:
    case SENDING_BIT1:
    case SENDING_BIT0:
        i2c->buffer = (i2c->buffer << 1) | (uint8_t)data;
        i2c->state = (bitbang_i2c_state)((int)i2c->state + 1);
        return bitbang_i2c_ret(i2c, 1);

    case WAITING_FOR_ACK:
        if (i2c->estate == I2C_DEVICEADDR) {
            i2c->current_addr = i2c->buffer;
            if ((i2c->current_addr & i2c->device_address_mask) != i2c->device_address) {
                i2c->state = I2C_STOPPED;
                return bitbang_i2c_ret(i2c, 0);
            }
            if (i2c->current_addr & 1) {
                i2c->estate = I2C_DATA;
            } else {
                i2c->estate = I2C_WORDADDR;
                i2c->eeprom_addr = ((i2c->buffer >> 1) & i2c->addressbitmask) << 8;
            }
        } else if (i2c->estate == I2C_WORDADDR) {
            i2c->estate = I2C_DATA;
            i2c->eeprom_addr &= (i2c->addressbitmask << 8);
            i2c->eeprom_addr |= i2c->buffer;
        } else if (!(i2c->current_addr & 1)) {
            if (i2c->write_offset < 0)
                i2c->write_offset = i2c->eeprom_addr;
            i2c->memory[i2c->eeprom_addr] = i2c->buffer;
            i2c->eeprom_addr = (i2c->eeprom_addr & ~(NVRAM_PAGE_SIZE - 1)) | ((i2c->eeprom_addr + 1) & (NVRAM_PAGE_SIZE - 1));
            gui_data.cdled = HDLED_WRITE;
        }
        if (i2c->current_addr & 1)
            i2c->state = RECEIVING_BIT7;
        else
            i2c->state = SENDING_BIT7;
        return bitbang_i2c_ret(i2c, 0);

    case RECEIVING_BIT7:
        i2c->buffer = i2c->memory[i2c->eeprom_addr];
        i2c->eeprom_addr++;
        i2c->eeprom_addr &= (i2c->size - 1);
        gui_data.cdled = HDLED_READ;
        /* Fall through */
    case RECEIVING_BIT6:
    case RECEIVING_BIT5:
    case RECEIVING_BIT4:
    case RECEIVING_BIT3:
    case RECEIVING_BIT2:
    case RECEIVING_BIT1:
    case RECEIVING_BIT0:
        data = i2c->buffer >> 7;
        i2c->state = (bitbang_i2c_state)((int)i2c->state + 1);
        i2c->buffer <<= 1;
        return bitbang_i2c_ret(i2c, data);

    case SENDING_ACK:
        i2c->state = RECEIVING_BIT7;
        if (data != 0)
            i2c->state = SENT_NACK;
        return bitbang_i2c_ret(i2c, 1);
    }
    return bitbang_i2c_ret(i2c, 1);
}

static void akiko_nvram_init(void)
{
    if (cd32_nvram_dirty)
        akiko_nvram_flush();
    if (!cd32_nvram_loaded)
        akiko_nvram_load();

    memset(&cd32_i2c, 0, sizeof(cd32_i2c));
    cd32_i2c.last_data = 1;
    cd32_i2c.last_clock = 1;
    cd32_i2c.device_out = 1;
    cd32_i2c.eeprom_addr = 0;
    cd32_i2c.write_offset = -1;
    cd32_i2c.estate = I2C_DEVICEADDR;
    cd32_i2c.memory = cd32_nvram;
    cd32_i2c.size = 1024;
    cd32_i2c.addressbitmask = (1024 / 256) - 1; /* 3 */
    cd32_i2c.device_address = 0xa0;
    cd32_i2c.device_address_mask = 0xf0;

    cd32_i2c_data_scl = true;
    cd32_i2c_data_sda = true;
    cd32_i2c_direction = 0;
}

static void akiko_nvram_write(int offset, uae_u32 v)
{
    switch (offset) {
    case 0:
        if (cd32_i2c_direction & 0x80)
            cd32_i2c_data_scl = (v & 0x80) != 0;
        else
            cd32_i2c_data_scl = true;
        eeprom_i2c_set(&cd32_i2c, BITBANG_I2C_SCL, cd32_i2c_data_scl);
        if (cd32_i2c_direction & 0x40)
            cd32_i2c_data_sda = (v & 0x40) != 0;
        else
            cd32_i2c_data_sda = true;
        eeprom_i2c_set(&cd32_i2c, BITBANG_I2C_SDA, cd32_i2c_data_sda);
        break;
    case 2:
        cd32_i2c_direction = (uae_u8)v;
        break;
    }
}

static uae_u32 akiko_nvram_read(int offset)
{
    uae_u32 v = 0;
    switch (offset) {
    case 0:
        v |= eeprom_i2c_set(&cd32_i2c, BITBANG_I2C_SCL, cd32_i2c_data_scl) ? 0x80 : 0x00;
        v |= eeprom_i2c_set(&cd32_i2c, BITBANG_I2C_SDA, cd32_i2c_data_sda) ? 0x40 : 0x00;
        break;
    case 2:
        v = cd32_i2c_direction;
        break;
    }
    return v;
}

static uae_u8 akiko_reg_read_byte(uaecptr address)
{
    int offset = (int)(address & 0x3f);
    switch (offset) {
        case 0x00: return 0xc0;
        case 0x01: return 0xca;
        case 0x02: return 0xca;
        case 0x03: return 0xfe;
        case 0x04: case 0x05: case 0x06: case 0x07:
            return (uae_u8)(cdrom_intreq >> ((3 - (offset - 4)) * 8));
        case 0x08: case 0x09: case 0x0a: case 0x0b:
            return (uae_u8)(cdrom_intena >> ((3 - (offset - 8)) * 8));
        case 0x0c: case 0x0d: case 0x0e: case 0x0f:
            return (uae_u8)(cdrom_intena >> ((3 - (offset - 0x0c)) * 8));
        case 0x10: case 0x14: case 0x18: case 0x1c:
            return cdrom_subcodeoffset;
        case 0x11: case 0x15: case 0x19: case 0x1d:
            return cdcomtxinx;
        case 0x12: case 0x16: case 0x1a: case 0x1e:
            return cdcomrxinx;
        case 0x13: case 0x17: case 0x1b: case 0x1f:
            return 0;
        case 0x20:
            return (uae_u8)(cdrom_pbx >> 8);
        case 0x21:
            return (uae_u8)cdrom_pbx;
        case 0x24: case 0x25: case 0x26: case 0x27:
            return (uae_u8)(cdrom_flags >> ((3 - (offset - 0x24)) * 8));
        case 0x28:
            if (!(cdrom_flags & CDFLAG_RXD) && cdrom_receive_offset < cdrom_receive_length) {
                cdrom_last_rx = cdrom_result_buffer[cdrom_receive_offset++];
                if (cdrom_receive_offset == cdrom_receive_length) {
                    cdrom_intreq &= ~CDINTERRUPT_DRIVERECV;
                    cdrom_receive_length = 0;
                    cdrom_receive_offset = 0;
                    set_status(CDINTERRUPT_DRIVEXMIT);
                }
            } else {
                cdrom_intreq &= ~CDINTERRUPT_DRIVERECV;
                akiko_update_irq();
            }
            return cdrom_last_rx;
        case 0x30:
            return (uae_u8)akiko_nvram_read(0);
        case 0x31:
            return 0x00;
        case 0x32:
            return (uae_u8)akiko_nvram_read(2);
        case 0x33:
            return 0x00;
        case 0x38: case 0x39: case 0x3a: case 0x3b:
            return (uae_u8)akiko_c2p_read(offset - 0x38);
        default:
            return 0;
    }
}

static void akiko_reg_write_byte(uaecptr address, uae_u8 value)
{
    int offset = (int)(address & 0x3f);
    uae_u32 mask;
    uae_u32 tmp;

    switch (offset) {
        case 0x08: case 0x09: case 0x0a: case 0x0b:
            mask = 0xffu << ((3 - (offset - 8)) * 8);
            cdrom_intena = (cdrom_intena & ~mask) | ((uae_u32)value << ((3 - (offset - 8)) * 8));
            cdrom_intena &= 0xff000000;
            akiko_update_irq();
            break;
        case 0x10: case 0x11: case 0x12: case 0x13:
            mask = 0xffu << ((3 - (offset - 0x10)) * 8);
            cdrom_addressdata = (cdrom_addressdata & ~mask) | ((uae_u32)value << ((3 - (offset - 0x10)) * 8));
            cdrom_addressdata &= 0x00fff000;
            break;
        case 0x14: case 0x15: case 0x16: case 0x17:
            mask = 0xffu << ((3 - (offset - 0x14)) * 8);
            cdrom_addressmisc = (cdrom_addressmisc & ~mask) | ((uae_u32)value << ((3 - (offset - 0x14)) * 8));
            cdrom_addressmisc &= 0x00fffc00;
            subcode_address = cdrom_addressmisc | 0x100;
            cdrx_address = cdrom_addressmisc;
            cdtx_address = cdrom_addressmisc | 0x200;
            if (offset == 0x17) {
                write_log("[AKIKO] MISC_ADDR=%08x (RX=%08x TX=%08x SUB=%08x)\n",
                    cdrom_addressmisc, cdrx_address, cdtx_address, subcode_address);
            }
            break;
        case 0x18:
            cdrom_intreq &= ~CDINTERRUPT_SUBCODE;
            akiko_update_irq();
            break;
        case 0x1d:
            cdrom_intreq &= ~CDINTERRUPT_TXDMADONE;
            cdcomtxcmp = value;
            cdrom_tx_dma_delay = 3;
            akiko_update_irq();
            break;
        case 0x1f:
            cdrom_intreq &= ~CDINTERRUPT_RXDMADONE;
            cdcomrxcmp = value;
            cdrom_rx_dma_delay = 3;
            akiko_update_irq();
            break;
        case 0x20: case 0x21:
            tmp = cdrom_pbx;
            mask = 0xffu << ((1 - (offset - 0x20)) * 8);
            cdrom_pbx = (cdrom_pbx & ~mask) | ((uae_u16)value << ((1 - (offset - 0x20)) * 8));
            cdrom_pbx |= (uae_u16)tmp;
            cdrom_pbx &= 0xffff;
            if (!(cdrom_flags & CDFLAG_PBX))
                cdrom_pbx = 0;
            cdrom_intreq &= ~CDINTERRUPT_PBX;
            akiko_update_irq();
            break;
        case 0x24: case 0x25: case 0x26: case 0x27:
            tmp = cdrom_flags;
            mask = 0xffu << ((3 - (offset - 0x24)) * 8);
            cdrom_flags = (cdrom_flags & ~mask) | ((uae_u32)value << ((3 - (offset - 0x24)) * 8));
            if ((cdrom_flags & CDFLAG_ENABLE) && !(tmp & CDFLAG_ENABLE)) {
                cdrom_sector_counter = 0;
                cdrom_intreq &= ~CDINTERRUPT_OVERFLOW;
            }
            if (!(cdrom_flags & CDFLAG_PBX))
                cdrom_pbx = 0;
            cdrom_flags &= 0xff800000;
            if (offset == 0x27)
                write_log("[AKIKO] FLAGS=%08x\n", cdrom_flags);
            akiko_update_irq();
            break;
        case 0x28:
            if (!(cdrom_flags & CDFLAG_TXD)) {
                cdrom_intreq &= ~CDINTERRUPT_DRIVEXMIT;
                if (can_send_command()) {
                    cdrom_add_command_byte(value);
                    if (can_send_command())
                        set_status(CDINTERRUPT_DRIVEXMIT);
                }
                akiko_update_irq();
            }
            break;
        case 0x30:
            akiko_nvram_write(0, value);
            break;
        case 0x32:
            akiko_nvram_write(2, value);
            break;
        case 0x38: case 0x39: case 0x3a: case 0x3b:
            akiko_c2p_write(offset - 0x38, value);
            break;
        default:
            break;
    }

    akiko_internal();
}

int akiko_bget(uaecptr addr)
{
    int v = akiko_reg_read_byte(addr);
    check_read_c2p(addr);
    return v;
}

int akiko_wget(uaecptr addr)
{
    int v = (akiko_reg_read_byte(addr) << 8) | akiko_reg_read_byte(addr + 1);
    check_read_c2p(addr);
    return v;
}

int akiko_lget(uaecptr addr)
{
    int v = ((uae_u32)akiko_reg_read_byte(addr) << 24) |
           ((uae_u32)akiko_reg_read_byte(addr + 1) << 16) |
           ((uae_u32)akiko_reg_read_byte(addr + 2) << 8) |
           (uae_u32)akiko_reg_read_byte(addr + 3);
    check_read_c2p(addr);
    return v;
}

void akiko_bput(uaecptr addr, int val)
{
    akiko_reg_write_byte(addr, (uae_u8)val);
}

void akiko_wput(uaecptr addr, int val)
{
    akiko_reg_write_byte(addr + 1, (uae_u8)val);
    akiko_reg_write_byte(addr, (uae_u8)(val >> 8));
}

void akiko_lput(uaecptr addr, int val)
{
    akiko_reg_write_byte(addr + 3, (uae_u8)val);
    akiko_reg_write_byte(addr + 2, (uae_u8)((uae_u32)val >> 8));
    akiko_reg_write_byte(addr + 1, (uae_u8)((uae_u32)val >> 16));
    akiko_reg_write_byte(addr, (uae_u8)((uae_u32)val >> 24));
}

uae_u32 akiko_read(uaecptr addr)
{
    return akiko_lget(addr);
}

void akiko_write(uaecptr addr, uae_u32 value)
{
    akiko_lput(addr, value);
}

void akiko_init(void)
{
    memset(akiko_buffer, 0, sizeof(akiko_buffer));
    memset(akiko_c2p_buffer, 0, sizeof(akiko_c2p_buffer));
    memset(akiko_c2p_out, 0, sizeof(akiko_c2p_out));
    akiko_c2p_read_offset = 0;
    akiko_bplcon = 0;

    cdrom_intreq = 0;
    cdrom_intena = 0;
    cdrom_addressdata = 0;
    cdrom_addressmisc = 0;
    subcode_address = 0;
    cdrx_address = 0;
    cdtx_address = 0;
    cdrom_flags = 0;
    cdrom_pbx = 0;
    cdrom_subcodeoffset = 0;
    cdcomtxinx = 0;
    cdcomtxcmp = 0;
    cdcomrxinx = 0;
    cdcomrxcmp = 0;
    memset(cdrom_command_buffer, 0, sizeof(cdrom_command_buffer));
    cdrom_command_length = 0;
    cdrom_command = 0;
    cdrom_checksum_error = 0;
    cdrom_unknown_command = 0;
    memset(cdrom_result_buffer, 0, sizeof(cdrom_result_buffer));
    cdrom_receive_length = 0;
    cdrom_receive_offset = 0;
    cdrom_last_rx = 0;
    cdrom_data_offset = -1;
    cdrom_sector_counter = 0;
    cdrom_speed = 2;
    cdrom_seek_delay = 0;
    cd_irq_asserted = 0;
    cd_initialized = 0;
    cdrom_command_active = 0;
    cdrom_tx_dma_delay = 0;
    cdrom_rx_dma_delay = 0;
    cdrom_audiotimeout = 0;
    cdrom_playing = 0;
    cdrom_audio_stop();
    akiko_c2p_read_offset = -1;
    akiko_c2p_write_offset = 0;
    memset(akiko_c2p_buffer, 0, sizeof(akiko_c2p_buffer));
    memset(akiko_c2p_out, 0, sizeof(akiko_c2p_out));
    cdrom_toc_counter = -1;
    akiko_nvram_init();
}

void akiko_reset(void)
{
    cdrom_audio_stop();
    akiko_init();
}

uae_u8 *akiko_save_state(int *length)
{
    uae_u8 *buffer = (uae_u8 *)malloc(1024);
    uae_u8 *dst = buffer;
    uae_u32 audio_start, audio_end, audio_phase;
    int audio_playing, audio_paused;

    if (!buffer) return NULL;
    cdrom_audio_get_state(&audio_start, &audio_end, &audio_phase, &audio_playing, &audio_paused);
    save_u32(2);
    save_u32(cdrom_intreq);
    save_u32(cdrom_intena);
    save_u32(cdrom_addressdata);
    save_u32(cdrom_addressmisc);
    save_u32(cdrom_flags);
    save_u16(cdrom_pbx);
    save_u8(cdrom_subcodeoffset);
    save_u8(cdcomtxinx);
    save_u8(cdcomtxcmp);
    save_u8(cdcomrxinx);
    save_u8(cdcomrxcmp);
    save_u32((uae_u32)cdrom_command_length);
    save_u32((uae_u32)cdrom_receive_length);
    save_u32((uae_u32)cdrom_receive_offset);
    save_u32((uae_u32)cdrom_data_offset);
    save_u32((uae_u32)cdrom_sector_counter);
    save_u32((uae_u32)cdrom_toc_counter);
    save_u32((uae_u32)cd_initialized);
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
    uae_u32 audio_start, audio_end, audio_phase;
    int audio_playing, audio_paused;

    if (!source) return source;
    uae_u32 ver = restore_u32();
    if (ver != 2) return source;
    cdrom_intreq = restore_u32();
    cdrom_intena = restore_u32();
    cdrom_addressdata = restore_u32();
    cdrom_addressmisc = restore_u32();
    subcode_address = cdrom_addressmisc | 0x100;
    cdrx_address = cdrom_addressmisc;
    cdtx_address = cdrom_addressmisc | 0x200;
    cdrom_flags = restore_u32();
    cdrom_pbx = restore_u16();
    cdrom_subcodeoffset = restore_u8();
    cdcomtxinx = restore_u8();
    cdcomtxcmp = restore_u8();
    cdcomrxinx = restore_u8();
    cdcomrxcmp = restore_u8();
    cdrom_command_length = (int)restore_u32();
    cdrom_receive_length = (int)restore_u32();
    cdrom_receive_offset = (int)restore_u32();
    cdrom_data_offset = (int)restore_u32();
    cdrom_sector_counter = (int)restore_u32();
    cdrom_toc_counter = (int)restore_u32();
    cd_initialized = (int)restore_u32();
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
