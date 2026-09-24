 /*
  * UAE - The Un*x Amiga Emulator
  *
  * DMS (Disk Masher System) disk archives
  *
  * The track decompressors follow the public domain xDMS engine of
  * Andre Rodrigues de la Rocha and Heikki Orsila.
  */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "sysconfig.h"
#include "sysdeps.h"

#include "dms.h"

#define DMS_HEADER_LEN 56
#define DMS_TRACK_HEADER_LEN 20
#define DMS_MAX_TRACK_LEN 32000
#define DMS_DEFAULT_ADF_LEN 901120
#define DMS_MIN_ADF_LEN (11 * 512)
#define DMS_MAX_ADF_LEN (4 * 1024 * 1024)
#define DMS_TRACK_FILE_ID 80
#define DMS_TRACK_BANNER 0xffff
#define DMS_MIN_DATA_TRACK_LEN 2048
#define DMS_MAX_TRACKS 4096

#define DMS_GENINFO_NO_ZERO 0x0001
#define DMS_GENINFO_ENCRYPTED 0x0002

#define DMS_FLAG_KEEP_STATE 0x01
#define DMS_FLAG_HEAVY_REBUILD 0x02
#define DMS_FLAG_HEAVY_RLE 0x04

#define DMS_MODE_NONE 0
#define DMS_MODE_SIMPLE 1
#define DMS_MODE_QUICK 2
#define DMS_MODE_MEDIUM 3
#define DMS_MODE_DEEP 4
#define DMS_MODE_HEAVY1 5
#define DMS_MODE_HEAVY2 6

/* first stage buffer for the two stage modes */

#define DMS_WINDOW_SIZE 0x4000
#define DMS_WINDOW_CLEAR 0x3fc8
#define DMS_QUICK_INIT_POS 251
#define DMS_MEDIUM_INIT_POS 0x3fbe
#define DMS_DEEP_INIT_POS 0x3fc4

#define DMS_NC 510
#define DMS_NPT 20
#define DMS_DEEP_N_CHAR (256 - 2 + 60)
#define DMS_DEEP_T (DMS_DEEP_N_CHAR * 2 - 1)
#define DMS_DEEP_R (DMS_DEEP_T - 1)

#define DMS_RLE_ESCAPE 0x90
#define DMS_RLE_LONG_COUNT 0xff

#define DMS_NODE_SLACK 16

static const uint8_t dms_d_code[256] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
    0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A,
    0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
    0x0C, 0x0C, 0x0C, 0x0C, 0x0D, 0x0D, 0x0D, 0x0D,
    0x0E, 0x0E, 0x0E, 0x0E, 0x0F, 0x0F, 0x0F, 0x0F,
    0x10, 0x10, 0x10, 0x10, 0x11, 0x11, 0x11, 0x11,
    0x12, 0x12, 0x12, 0x12, 0x13, 0x13, 0x13, 0x13,
    0x14, 0x14, 0x14, 0x14, 0x15, 0x15, 0x15, 0x15,
    0x16, 0x16, 0x16, 0x16, 0x17, 0x17, 0x17, 0x17,
    0x18, 0x18, 0x19, 0x19, 0x1A, 0x1A, 0x1B, 0x1B,
    0x1C, 0x1C, 0x1D, 0x1D, 0x1E, 0x1E, 0x1F, 0x1F,
    0x20, 0x20, 0x21, 0x21, 0x22, 0x22, 0x23, 0x23,
    0x24, 0x24, 0x25, 0x25, 0x26, 0x26, 0x27, 0x27,
    0x28, 0x28, 0x29, 0x29, 0x2A, 0x2A, 0x2B, 0x2B,
    0x2C, 0x2C, 0x2D, 0x2D, 0x2E, 0x2E, 0x2F, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
};

static const uint8_t dms_d_len[256] = {
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
};

typedef struct {
    const uint8_t *data;
    size_t len;
    size_t pos;
    uint32_t buf;
    uint32_t count;
} DmsBits;

typedef struct {
    uint8_t window[DMS_WINDOW_SIZE];
    uint16_t quick_pos;
    uint16_t medium_pos;
    uint16_t deep_pos;
    uint16_t heavy_pos;
    uint16_t heavy_lastlen;
    uint8_t c_len[DMS_NC];
    uint8_t pt_len[DMS_NPT];
    uint16_t c_table[4096];
    uint16_t pt_table[256];
    uint16_t left[2 * DMS_NC - 1 + DMS_NODE_SLACK];
    uint16_t right[2 * DMS_NC - 1 + 9 + DMS_NODE_SLACK];
    uint16_t deep_freq[DMS_DEEP_T + 1 + DMS_NODE_SLACK];
    uint16_t deep_prnt[DMS_DEEP_T + DMS_DEEP_N_CHAR + DMS_NODE_SLACK];
    uint16_t deep_son[DMS_DEEP_T + DMS_NODE_SLACK];
    int deep_init;
    uint8_t scratch[DMS_MAX_TRACK_LEN];
} DmsDecompressor;

typedef struct {
    const uint8_t *blen;
    uint16_t *table;
    uint16_t *left;
    uint16_t *right;
    uint16_t n;
    uint16_t avail;
    uint16_t table_size;
    uint16_t bit;
    uint16_t max_depth;
    uint16_t depth;
    uint16_t len;
    int c;
    uint16_t codeword;
    uint32_t calls;
} DmsTableBuilder;

static uint16_t dms_be16 (const uint8_t *p)
{
    return (uint16_t) ((p[0] << 8) | p[1]);
}

static uint32_t dms_be24 (const uint8_t *p)
{
    return ((uint32_t) p[0] << 16) | ((uint32_t) p[1] << 8) | p[2];
}

static uint16_t dms_crc16 (const uint8_t *data, size_t len)
{
    uint16_t crc = 0;
    size_t i;
    int bit;

    for (i = 0; i < len; i++) {
	crc ^= data[i];
	for (bit = 0; bit < 8; bit++)
	    crc = (crc & 1) ? (uint16_t) ((crc >> 1) ^ 0xa001) : (uint16_t) (crc >> 1);
    }
    return crc;
}

static uint16_t dms_checksum16 (const uint8_t *data, size_t len)
{
    uint16_t sum = 0;
    size_t i;

    for (i = 0; i < len; i++)
	sum = (uint16_t) (sum + data[i]);
    return sum;
}

/*
 * bit reader
 */

static void dms_bits_refill (DmsBits *bits)
{
    while (bits->count < 16) {
	uint32_t byte = bits->pos < bits->len ? bits->data[bits->pos] : 0;
	bits->pos++;
	bits->buf = (bits->buf << 8) | byte;
	bits->count += 8;
    }
}

static void dms_bits_init (DmsBits *bits, const uint8_t *data, size_t len)
{
    bits->data = data;
    bits->len = len;
    bits->pos = 0;
    bits->buf = 0;
    bits->count = 0;
    dms_bits_refill (bits);
}

static uint16_t dms_bits_peek (const DmsBits *bits, unsigned n)
{
    if (n > bits->count)
	n = bits->count;
    return (uint16_t) (bits->buf >> (bits->count - n));
}

static void dms_bits_consume (DmsBits *bits, unsigned n)
{
    if (n > bits->count)
	n = bits->count;
    bits->count -= n;
    bits->buf &= (1u << bits->count) - 1;
    dms_bits_refill (bits);
}

static uint16_t dms_bits_read (DmsBits *bits, unsigned n)
{
    uint16_t value = dms_bits_peek (bits, n);
    dms_bits_consume (bits, n);
    return value;
}

/*
 * sliding window
 */

static void dms_push_window (uint8_t *window, uint16_t *pos, uint16_t mask, uint8_t byte)
{
    window[*pos & mask] = byte;
    *pos = (uint16_t) (*pos + 1);
}

static int dms_copy_match (uint8_t *window, uint16_t *pos, uint16_t mask, uint16_t distance, uint16_t length, uint8_t *out, size_t *out_pos, size_t out_len)
{
    uint32_t src = (uint32_t) (uint16_t) (*pos - distance - 1);
    uint16_t i;

    for (i = 0; i < length; i++) {
	uint8_t byte;

	if (*out_pos >= out_len)
	    return 0;
	byte = window[src & mask];
	dms_push_window (window, pos, mask, byte);
	src++;
	out[*out_pos] = byte;
	(*out_pos)++;
    }
    return 1;
}

static void dms_reset (DmsDecompressor *d)
{
    d->quick_pos = DMS_QUICK_INIT_POS;
    d->medium_pos = DMS_MEDIUM_INIT_POS;
    d->deep_pos = DMS_DEEP_INIT_POS;
    d->heavy_pos = 0;
    d->heavy_lastlen = 0;
    d->deep_init = 1;
    memset (d->window, 0, DMS_WINDOW_CLEAR);
}

/*
 * run length decoding, second stage of most modes
 */

static int dms_unpack_rle (const uint8_t *in, size_t in_len, uint8_t *out, size_t out_len)
{
    size_t in_pos = 0;
    size_t pos = 0;

    while (pos < out_len) {
	uint8_t byte, run, value;
	uint32_t count;

	if (in_pos >= in_len)
	    return 0;
	byte = in[in_pos++];
	if (byte != DMS_RLE_ESCAPE) {
	    out[pos++] = byte;
	    continue;
	}
	if (in_pos >= in_len)
	    return 0;
	run = in[in_pos++];
	if (run == 0) {
	    out[pos++] = DMS_RLE_ESCAPE;
	    continue;
	}
	if (in_pos >= in_len)
	    return 0;
	value = in[in_pos++];
	if (run == DMS_RLE_LONG_COUNT) {
	    if (in_pos + 1 >= in_len)
		return 0;
	    count = ((uint32_t) in[in_pos] << 8) | in[in_pos + 1];
	    in_pos += 2;
	} else {
	    count = run;
	}
	if (count > out_len - pos)
	    return 0;
	memset (out + pos, value, count);
	pos += count;
    }
    return 1;
}

/*
 * quick
 */

static int dms_unpack_quick (DmsDecompressor *d, const uint8_t *packed, size_t packed_len, uint8_t *out, size_t out_len)
{
    DmsBits bits;
    size_t pos = 0;

    dms_bits_init (&bits, packed, packed_len);
    while (pos < out_len) {
	if (dms_bits_read (&bits, 1) != 0) {
	    uint8_t byte = (uint8_t) dms_bits_read (&bits, 8);
	    dms_push_window (d->window, &d->quick_pos, 0xff, byte);
	    out[pos++] = byte;
	} else {
	    uint16_t length = (uint16_t) (dms_bits_read (&bits, 2) + 2);
	    uint16_t distance = dms_bits_read (&bits, 8);
	    if (! dms_copy_match (d->window, &d->quick_pos, 0xff, distance, length, out, &pos, out_len))
		return 0;
	}
    }
    d->quick_pos = (uint16_t) ((d->quick_pos + 5) & 0xff);
    return 1;
}

/*
 * medium
 */

static uint16_t dms_medium_distance (DmsBits *bits, unsigned prefix)
{
    unsigned extra = dms_d_len[prefix];
    unsigned mid = (unsigned) ((((uint32_t) prefix << extra) | dms_bits_read (bits, extra)) & 0xff);
    unsigned low;

    extra = dms_d_len[mid];
    low = (unsigned) ((((uint32_t) mid << extra) | dms_bits_read (bits, extra)) & 0xff);
    return (uint16_t) (((uint16_t) dms_d_code[mid] << 8) | low);
}

static int dms_unpack_medium (DmsDecompressor *d, const uint8_t *packed, size_t packed_len, uint8_t *out, size_t out_len)
{
    DmsBits bits;
    size_t pos = 0;

    dms_bits_init (&bits, packed, packed_len);
    while (pos < out_len) {
	if (dms_bits_read (&bits, 1) != 0) {
	    uint8_t byte = (uint8_t) dms_bits_read (&bits, 8);
	    dms_push_window (d->window, &d->medium_pos, 0x3fff, byte);
	    out[pos++] = byte;
	} else {
	    unsigned prefix = dms_bits_read (&bits, 8) & 0xff;
	    uint16_t length = (uint16_t) (dms_d_code[prefix] + 3);
	    uint16_t distance = dms_medium_distance (&bits, prefix);
	    if (! dms_copy_match (d->window, &d->medium_pos, 0x3fff, distance, length, out, &pos, out_len))
		return 0;
	}
    }
    d->medium_pos = (uint16_t) ((d->medium_pos + 66) & 0x3fff);
    return 1;
}

/*
 * deep
 */

static void dms_init_deep (DmsDecompressor *d)
{
    int i, j;

    for (i = 0; i < DMS_DEEP_N_CHAR; i++) {
	d->deep_freq[i] = 1;
	d->deep_son[i] = (uint16_t) (i + DMS_DEEP_T);
	d->deep_prnt[i + DMS_DEEP_T] = (uint16_t) i;
    }
    i = 0;
    j = DMS_DEEP_N_CHAR;
    while (j <= DMS_DEEP_R) {
	d->deep_freq[j] = (uint16_t) (d->deep_freq[i] + d->deep_freq[i + 1]);
	d->deep_son[j] = (uint16_t) i;
	d->deep_prnt[i] = (uint16_t) j;
	d->deep_prnt[i + 1] = (uint16_t) j;
	i += 2;
	j++;
    }
    d->deep_freq[DMS_DEEP_T] = 0xffff;
    d->deep_prnt[DMS_DEEP_R] = 0;
    d->deep_init = 0;
}

static void dms_reconstruct_deep (DmsDecompressor *d)
{
    int i, j, k;

    j = 0;
    for (i = 0; i < DMS_DEEP_T; i++) {
	if (d->deep_son[i] >= DMS_DEEP_T) {
	    d->deep_freq[j] = (uint16_t) ((d->deep_freq[i] + 1) / 2);
	    d->deep_son[j] = d->deep_son[i];
	    j++;
	}
    }
    i = 0;
    j = DMS_DEEP_N_CHAR;
    while (j < DMS_DEEP_T) {
	uint16_t f = (uint16_t) (d->deep_freq[i] + d->deep_freq[i + 1]);

	d->deep_freq[j] = f;
	k = j - 1;
	while (f < d->deep_freq[k])
	    k--;
	k++;
	memmove (&d->deep_freq[k + 1], &d->deep_freq[k], (size_t) (j - k) * sizeof (uint16_t));
	d->deep_freq[k] = f;
	memmove (&d->deep_son[k + 1], &d->deep_son[k], (size_t) (j - k) * sizeof (uint16_t));
	d->deep_son[k] = (uint16_t) i;
	i += 2;
	j++;
    }
    for (i = 0; i < DMS_DEEP_T; i++) {
	k = d->deep_son[i];
	d->deep_prnt[k] = (uint16_t) i;
	if (k < DMS_DEEP_T)
	    d->deep_prnt[k + 1] = (uint16_t) i;
    }
}

static void dms_update_deep (DmsDecompressor *d, uint16_t symbol)
{
    uint16_t c, k, i, j, l;

    if (d->deep_freq[DMS_DEEP_R] == 0x8000)
	dms_reconstruct_deep (d);
    c = d->deep_prnt[symbol + DMS_DEEP_T];
    for (;;) {
	d->deep_freq[c] = (uint16_t) (d->deep_freq[c] + 1);
	k = d->deep_freq[c];
	l = (uint16_t) (c + 1);
	if (k > d->deep_freq[l]) {
	    for (;;) {
		l++;
		if (k <= d->deep_freq[l])
		    break;
	    }
	    l--;
	    d->deep_freq[c] = d->deep_freq[l];
	    d->deep_freq[l] = k;
	    i = d->deep_son[c];
	    d->deep_prnt[i] = l;
	    if (i < DMS_DEEP_T)
		d->deep_prnt[i + 1] = l;
	    j = d->deep_son[l];
	    d->deep_son[l] = i;
	    d->deep_prnt[j] = c;
	    if (j < DMS_DEEP_T)
		d->deep_prnt[j + 1] = c;
	    d->deep_son[c] = j;
	    c = l;
	}
	c = d->deep_prnt[c];
	if (c == 0)
	    break;
    }
}

static int dms_decode_deep_char (DmsDecompressor *d, DmsBits *bits, uint16_t *symbol)
{
    uint16_t node = d->deep_son[DMS_DEEP_R];
    unsigned guard = 0;

    while (node < DMS_DEEP_T) {
	node = d->deep_son[node + dms_bits_read (bits, 1)];
	if (++guard > DMS_DEEP_T)
	    return 0;
    }
    *symbol = (uint16_t) (node - DMS_DEEP_T);
    if (*symbol >= DMS_DEEP_N_CHAR)
	return 0;
    dms_update_deep (d, *symbol);
    return 1;
}

static uint16_t dms_deep_distance (DmsBits *bits)
{
    unsigned prefix = dms_bits_read (bits, 8) & 0xff;
    unsigned extra = dms_d_len[prefix];
    uint16_t low = (uint16_t) ((((uint32_t) prefix << extra) | dms_bits_read (bits, extra)) & 0xff);

    return (uint16_t) (((uint16_t) dms_d_code[prefix] << 8) | low);
}

static int dms_unpack_deep (DmsDecompressor *d, const uint8_t *packed, size_t packed_len, uint8_t *out, size_t out_len)
{
    DmsBits bits;
    size_t pos = 0;

    dms_bits_init (&bits, packed, packed_len);
    if (d->deep_init)
	dms_init_deep (d);
    while (pos < out_len) {
	uint16_t code;

	if (! dms_decode_deep_char (d, &bits, &code))
	    return 0;
	if (code < 256) {
	    dms_push_window (d->window, &d->deep_pos, 0x3fff, (uint8_t) code);
	    out[pos++] = (uint8_t) code;
	} else {
	    uint16_t length = (uint16_t) (code - 253);
	    uint16_t distance = dms_deep_distance (&bits);
	    if (! dms_copy_match (d->window, &d->deep_pos, 0x3fff, distance, length, out, &pos, out_len))
		return 0;
	}
    }
    d->deep_pos = (uint16_t) ((d->deep_pos + 60) & 0x3fff);
    return 1;
}

/*
 * heavy
 */

static int dms_build_table (DmsTableBuilder *b, uint16_t *result)
{
    uint16_t node = 0;

    if (++b->calls > 0x10000)
	return 0;
    if (b->len > 32)
	return 0;
    if (b->len == b->depth) {
	for (;;) {
	    uint16_t start, i;

	    b->c++;
	    if (b->c >= (int) b->n)
		break;
	    if (b->blen[b->c] == b->len) {
		start = b->codeword;
		b->codeword = (uint16_t) (b->codeword + b->bit);
		if (b->codeword > b->table_size)
		    return 0;
		for (i = start; i < b->codeword; i++)
		    b->table[i] = (uint16_t) b->c;
		*result = (uint16_t) b->c;
		return 1;
	    }
	}
	b->c = -1;
	b->len++;
	b->bit >>= 1;
    }
    b->depth++;
    if (b->depth < b->max_depth) {
	uint16_t dummy;

	if (! dms_build_table (b, &dummy))
	    return 0;
	if (! dms_build_table (b, &dummy))
	    return 0;
    } else if (b->depth > 32) {
	return 0;
    } else {
	uint16_t l, r;

	node = b->avail;
	b->avail++;
	if (node >= (uint16_t) (2 * b->n - 1))
	    return 0;
	if (! dms_build_table (b, &l))
	    return 0;
	b->left[node] = l;
	if (! dms_build_table (b, &r))
	    return 0;
	b->right[node] = r;
	if (b->codeword >= b->table_size)
	    return 0;
	if (b->depth == b->max_depth) {
	    b->table[b->codeword] = node;
	    b->codeword++;
	}
    }
    b->depth--;
    *result = node;
    return 1;
}

static int dms_make_table (uint16_t *left, uint16_t *right, uint16_t nchar, const uint8_t *bitlen, uint16_t tablebits, uint16_t *table)
{
    DmsTableBuilder b;
    uint16_t dummy;

    memset (&b, 0, sizeof (b));
    b.blen = bitlen;
    b.table = table;
    b.left = left;
    b.right = right;
    b.n = nchar;
    b.avail = nchar;
    b.table_size = (uint16_t) (1 << tablebits);
    b.bit = (uint16_t) (b.table_size >> 1);
    b.max_depth = (uint16_t) (tablebits + 1);
    b.depth = 1;
    b.len = 1;
    b.c = -1;
    b.codeword = 0;
    if (! dms_build_table (&b, &dummy))
	return 0;
    if (! dms_build_table (&b, &dummy))
	return 0;
    return b.codeword == b.table_size;
}

static int dms_read_tree_c (DmsDecompressor *d, DmsBits *bits)
{
    unsigned count = dms_bits_read (bits, 9);

    if (count > 0) {
	unsigned i;

	if (count > DMS_NC)
	    return 0;
	for (i = 0; i < count; i++)
	    d->c_len[i] = (uint8_t) dms_bits_read (bits, 5);
	memset (d->c_len + count, 0, DMS_NC - count);
	return dms_make_table (d->left, d->right, DMS_NC, d->c_len, 12, d->c_table);
    }
    {
	uint16_t symbol = dms_bits_read (bits, 9);
	unsigned i;

	memset (d->c_len, 0, sizeof (d->c_len));
	for (i = 0; i < 4096; i++)
	    d->c_table[i] = symbol;
    }
    return 1;
}

static int dms_read_tree_p (DmsDecompressor *d, DmsBits *bits, uint16_t np)
{
    unsigned count = dms_bits_read (bits, 5);

    if (count > 0) {
	unsigned i;

	if (count > DMS_NPT)
	    return 0;
	for (i = 0; i < count; i++)
	    d->pt_len[i] = (uint8_t) dms_bits_read (bits, 4);
	memset (d->pt_len + count, 0, DMS_NPT - count);
	return dms_make_table (d->left, d->right, np, d->pt_len, 8, d->pt_table);
    }
    {
	uint16_t symbol = dms_bits_read (bits, 5);
	unsigned i;

	memset (d->pt_len, 0, sizeof (d->pt_len));
	for (i = 0; i < 256; i++)
	    d->pt_table[i] = symbol;
    }
    return 1;
}

static int dms_decode_c (DmsDecompressor *d, DmsBits *bits, uint16_t *code)
{
    uint16_t node = d->c_table[dms_bits_peek (bits, 12) & 0xfff];

    if (node < DMS_NC) {
	dms_bits_consume (bits, d->c_len[node]);
    } else {
	uint16_t path, probe = 0x8000;

	if (node >= (uint16_t) (2 * DMS_NC - 1 + 9))
	    return 0;
	dms_bits_consume (bits, 12);
	path = dms_bits_peek (bits, 16);
	for (;;) {
	    node = (path & probe) ? d->right[node] : d->left[node];
	    probe >>= 1;
	    if (node < DMS_NC)
		break;
	    if (probe == 0)
		return 0;
	    if (node >= (uint16_t) (2 * DMS_NC - 1 + 9))
		return 0;
	}
	if (d->c_len[node] < 12)
	    return 0;
	dms_bits_consume (bits, d->c_len[node] - 12);
    }
    *code = node;
    return 1;
}

static int dms_decode_p (DmsDecompressor *d, DmsBits *bits, uint16_t np, uint16_t *distance)
{
    uint16_t node = d->pt_table[dms_bits_peek (bits, 8) & 0xff];

    if (node < np) {
	dms_bits_consume (bits, d->pt_len[node]);
    } else {
	uint16_t path, probe = 0x8000;

	if (node >= (uint16_t) (2 * DMS_NC - 1 + 9))
	    return 0;
	dms_bits_consume (bits, 8);
	path = dms_bits_peek (bits, 16);
	for (;;) {
	    node = (path & probe) ? d->right[node] : d->left[node];
	    probe >>= 1;
	    if (node < np)
		break;
	    if (probe == 0)
		return 0;
	    if (node >= (uint16_t) (2 * DMS_NC - 1 + 9))
		return 0;
	}
	if (d->pt_len[node] < 8)
	    return 0;
	dms_bits_consume (bits, d->pt_len[node] - 8);
    }
    if (node != np - 1) {
	if (node > 0) {
	    uint16_t extra = (uint16_t) (node - 1);

	    node = (uint16_t) (dms_bits_peek (bits, extra) | (1 << extra));
	    dms_bits_consume (bits, extra);
	}
	d->heavy_lastlen = node;
    }
    *distance = d->heavy_lastlen;
    return 1;
}

static int dms_unpack_heavy (DmsDecompressor *d, int big_dict, int rebuild, const uint8_t *packed, size_t packed_len, uint8_t *out, size_t out_len)
{
    DmsBits bits;
    uint16_t np = big_dict ? 15 : 14;
    uint16_t mask = big_dict ? 0x1fff : 0x0fff;
    size_t pos = 0;

    dms_bits_init (&bits, packed, packed_len);
    if (rebuild) {
	if (! dms_read_tree_c (d, &bits))
	    return 0;
	if (! dms_read_tree_p (d, &bits, np))
	    return 0;
    }
    while (pos < out_len) {
	uint16_t code;

	if (! dms_decode_c (d, &bits, &code))
	    return 0;
	if (code < 256) {
	    dms_push_window (d->window, &d->heavy_pos, mask, (uint8_t) code);
	    out[pos++] = (uint8_t) code;
	} else {
	    uint16_t length = (uint16_t) (code - 253);
	    uint16_t distance;

	    if (! dms_decode_p (d, &bits, np, &distance))
		return 0;
	    if (! dms_copy_match (d->window, &d->heavy_pos, mask, distance, length, out, &pos, out_len))
		return 0;
	}
    }
    return 1;
}

/*
 * single track
 */

static int dms_unpack_track (DmsDecompressor *d, unsigned mode, unsigned flags, const uint8_t *packed, size_t packed_len, size_t intermediate_len, uint8_t *out, size_t out_len)
{
    if (mode >= DMS_MODE_QUICK && intermediate_len > 0)
	memset (d->scratch, 0, intermediate_len);
    switch (mode) {
    case DMS_MODE_NONE:
	if (packed_len < out_len)
	    return 0;
	memcpy (out, packed, out_len);
	return 1;
    case DMS_MODE_SIMPLE:
	return dms_unpack_rle (packed, packed_len, out, out_len);
    case DMS_MODE_QUICK:
	if (! dms_unpack_quick (d, packed, packed_len, d->scratch, intermediate_len))
	    return 0;
	return dms_unpack_rle (d->scratch, intermediate_len, out, out_len);
    case DMS_MODE_MEDIUM:
	if (! dms_unpack_medium (d, packed, packed_len, d->scratch, intermediate_len))
	    return 0;
	return dms_unpack_rle (d->scratch, intermediate_len, out, out_len);
    case DMS_MODE_DEEP:
	if (! dms_unpack_deep (d, packed, packed_len, d->scratch, intermediate_len))
	    return 0;
	return dms_unpack_rle (d->scratch, intermediate_len, out, out_len);
    case DMS_MODE_HEAVY1:
    case DMS_MODE_HEAVY2:
	if (! dms_unpack_heavy (d, mode == DMS_MODE_HEAVY2, (flags & DMS_FLAG_HEAVY_REBUILD) != 0, packed, packed_len, d->scratch, intermediate_len))
	    return 0;
	if (flags & DMS_FLAG_HEAVY_RLE)
	    return dms_unpack_rle (d->scratch, intermediate_len, out, out_len);
	if (intermediate_len < out_len)
	    return 0;
	memcpy (out, d->scratch, out_len);
	return 1;
    }
    return 0;
}

/*
 * archive
 */

int dms_is_archive (const char *path)
{
    uint8_t magic[4];
    FILE *f;
    size_t got;

    if (! path)
	return 0;
    f = fopen (path, "rb");
    if (! f)
	return 0;
    got = fread (magic, 1, 4, f);
    fclose (f);
    return got == 4 && memcmp (magic, "DMS!", 4) == 0;
}

static int dms_write_zeros (FILE *out, size_t count)
{
    static const uint8_t zeros[512] = { 0 };
    size_t chunk;

    while (count > 0) {
	chunk = count > sizeof (zeros) ? sizeof (zeros) : count;
	if (fwrite (zeros, 1, chunk, out) != chunk)
	    return 0;
	count -= chunk;
    }
    return 1;
}

int dms_uncompress (const char *path, const char *dest)
{
    uint8_t header[DMS_HEADER_LEN];
    uint8_t track_header[DMS_TRACK_HEADER_LEN];
    uint8_t *packed = NULL;
    uint8_t *unpacked = NULL;
    DmsDecompressor *d = NULL;
    FILE *in = NULL;
    FILE *out = NULL;
    uint32_t adf_size;
    unsigned geninfo;
    unsigned prev_number = 0;
    unsigned prev_len = 0;
    int have_prev = 0;
    int positional;
    int tracks = 0;
    int result = 0;
    size_t written = 0;
    int track_index;

    in = fopen (path, "rb");
    if (! in)
	return 0;
    if (fread (header, 1, DMS_HEADER_LEN, in) != DMS_HEADER_LEN) {
	fclose (in);
	return 0;
    }
    if (memcmp (header, "DMS!", 4) != 0) {
	fclose (in);
	return 0;
    }
    if (! dest) {
	fclose (in);
	return 1;
    }

    geninfo = dms_be16 (header + 10);
    if (dms_crc16 (header + 4, DMS_HEADER_LEN - 6) != dms_be16 (header + DMS_HEADER_LEN - 2))
	write_log ("DMS archive header is damaged: %s\n", path);
    if (geninfo & DMS_GENINFO_ENCRYPTED) {
	write_log ("DMS archive is password protected: %s\n", path);
	fclose (in);
	return 0;
    }

    adf_size = dms_be24 (header + 25);
    if (adf_size < DMS_MIN_ADF_LEN || adf_size > DMS_MAX_ADF_LEN)
	adf_size = DMS_DEFAULT_ADF_LEN;
    positional = (geninfo & DMS_GENINFO_NO_ZERO) != 0;

    packed = (uint8_t *) malloc (DMS_MAX_TRACK_LEN);
    unpacked = (uint8_t *) malloc (DMS_MAX_TRACK_LEN);
    d = (DmsDecompressor *) malloc (sizeof (DmsDecompressor));
    if (! packed || ! unpacked || ! d) {
	write_log ("DMS out of memory: %s\n", path);
	goto done;
    }
    memset (d, 0, sizeof (DmsDecompressor));
    out = fopen (dest, "wb");
    if (! out) {
	write_log ("DMS could not write disk image: %s\n", dest);
	goto done;
    }

    dms_reset (d);
    for (track_index = 0; track_index < DMS_MAX_TRACKS; track_index++) {
	unsigned number, packed_len, intermediate_len, unpacked_len, flags, mode, checksum, data_crc;
	size_t todo;

	if (fread (track_header, 1, DMS_TRACK_HEADER_LEN, in) != DMS_TRACK_HEADER_LEN)
	    break;
	if (memcmp (track_header, "TR", 2) != 0)
	    break;
	if (dms_crc16 (track_header, DMS_TRACK_HEADER_LEN - 2) != dms_be16 (track_header + DMS_TRACK_HEADER_LEN - 2)) {
	    write_log ("DMS track header is damaged: %s\n", path);
	    break;
	}
	number = dms_be16 (track_header + 2);
	packed_len = dms_be16 (track_header + 6);
	intermediate_len = dms_be16 (track_header + 8);
	unpacked_len = dms_be16 (track_header + 10);
	flags = track_header[12];
	mode = track_header[13];
	checksum = dms_be16 (track_header + 14);
	data_crc = dms_be16 (track_header + 16);
	if (packed_len > DMS_MAX_TRACK_LEN || intermediate_len > DMS_MAX_TRACK_LEN || unpacked_len > DMS_MAX_TRACK_LEN)
	    break;
	if (fread (packed, 1, packed_len, in) != packed_len)
	    break;
	if (dms_crc16 (packed, packed_len) != data_crc) {
	    write_log ("DMS track %u is damaged: %s\n", number, path);
	    break;
	}

	/* banner and FILEID.DIZ tracks are not part of the disk image */
	if (number == DMS_TRACK_BANNER || number == DMS_TRACK_FILE_ID)
	    continue;
	if (number >= DMS_TRACK_FILE_ID || unpacked_len <= DMS_MIN_DATA_TRACK_LEN)
	    continue;

	if (positional && written < adf_size) {
	    size_t gap = 0;

	    if (! have_prev)
		gap = number > 0 ? (size_t) number * unpacked_len : 0;
	    else if (number > prev_number + 1)
		gap = (size_t) (number - prev_number - 1) * prev_len;
	    if (gap > adf_size - written)
		gap = adf_size - written;
	    if (gap > 0) {
		if (! dms_write_zeros (out, gap))
		    goto done;
		written += gap;
	    }
	}
	have_prev = 1;
	prev_number = number;
	prev_len = unpacked_len;

	memset (unpacked, 0, unpacked_len);
	if (! dms_unpack_track (d, mode, flags, packed, packed_len, intermediate_len, unpacked, unpacked_len)
	    || dms_checksum16 (unpacked, unpacked_len) != checksum) {
	    write_log ("DMS track %u could not be decompressed: %s\n", number, path);
	    memset (unpacked, 0, unpacked_len);
	}
	if (! (flags & DMS_FLAG_KEEP_STATE))
	    dms_reset (d);

	todo = unpacked_len;
	if (todo > adf_size - written)
	    todo = adf_size - written;
	if (todo > 0 && fwrite (unpacked, 1, todo, out) != todo)
	    goto done;
	written += todo;
	tracks++;
    }

    if (tracks == 0) {
	write_log ("DMS archive holds no data: %s\n", path);
	goto done;
    }
    if (written < adf_size) {
	if (! dms_write_zeros (out, adf_size - written))
	    goto done;
	written = adf_size;
    }
    if (ferror (out))
	goto done;
    write_log ("DMS archive unpacked: %s\n", path);
    result = 1;

done:
    if (out)
	fclose (out);
    if (in)
	fclose (in);
    free (d);
    free (unpacked);
    free (packed);
    return result;
}
