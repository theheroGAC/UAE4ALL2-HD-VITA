 /*
  * UAE - The Un*x Amiga Emulator
  *
  * Hardfile emulation
  *
  * Copyright 1995 Bernd Schmidt
  */

#include "sysconfig.h"
#include "sysdeps.h"

#include "config.h"
#include "options.h"
#include "memory-uae.h"
#include "custom.h"
#include "m68k/m68k_intrf.h"
#include "disk.h"
#include "autoconf.h"
#include "filesys.h"
#include "hdf_io64.h"
#include "execlib.h"
#include "gui.h"

static int opencount = 0;
static uaecptr hardfile_nsd_cmdlist = 0;

static uae_u32 hardfile_open (void)
{
    uaecptr tmp1 = m68k_areg(regs, 1); /* IOReq */
    int unit = m68k_dreg (regs, 0);
    struct hardfiledata *hfd = get_hardfile_data (unit);

    /* Check unit number */
    if (hfd && hdf_is_open (hfd->fd)) {
	opencount++;
	put_word (m68k_areg(regs, 6)+32, get_word (m68k_areg(regs, 6)+32) + 1);
	put_long (tmp1 + 24, m68k_dreg (regs, 0)); /* io_Unit */
	put_byte (tmp1 + 31, 0); /* io_Error */
	put_byte (tmp1 + 8, 7); /* ln_type = NT_REPLYMSG */
	return 0;
    }

    put_long (tmp1 + 20, (uae_u32)-1);
    put_byte (tmp1 + 31, (uae_u8)-1);
    return (uae_u32)-1;
}

static uae_u32 hardfile_close (void)
{
    opencount--;
    put_word (m68k_areg(regs, 6) + 32, get_word (m68k_areg(regs, 6) + 32) - 1);

    return 0;
}

static uae_u32 hardfile_expunge (void)
{
    return 0; /* Simply ignore this one... */
}

static uae_u32 hardfile_beginio (void)
{
	uae_u32 tmp1, tmp2, dataptr;
	unsigned long long offset;
	uae_u32 retval = m68k_dreg(regs, 0);
	int unit;
	struct hardfiledata *hfd;
	
	tmp1 = m68k_areg(regs, 1);
	unit = get_long (tmp1 + 24);

	hfd = get_hardfile_data (unit);
	
	put_byte (tmp1+8, NT_MESSAGE);
	put_byte (tmp1+31, 0);
	tmp2 = get_word (tmp1+28);

	if (!hfd || !hdf_is_open (hfd->fd)) {
		put_byte (tmp1+31, (uae_u8)-3);
		return 0;
	}

	switch (tmp2) {
		case CMD_READ:
		case 24:
		case 0x4002:
			gui_data.hdled = HDLED_READ;
			
			dataptr = get_long (tmp1 + 40);
			if (tmp2 == 24)
				offset = ((unsigned long long)(uae_u32)get_long (tmp1 + 32) << 32) | (unsigned long long)(uae_u32)get_long (tmp1 + 44);
			else if (tmp2 == 0x4002)
				offset = ((unsigned long long)(uae_u32)get_long (tmp1 + 48) << 32) | (unsigned long long)(uae_u32)get_long (tmp1 + 44);
			else
				offset = (unsigned long long)(uae_u32)get_long (tmp1 + 44);
			tmp2 = get_long (tmp1 + 36);

			if (dataptr & 1 || offset & 511 || tmp2 & 511 || offset + (unsigned long long)tmp2 > hfd->size)
				goto bad_command;
			
			put_long (tmp1 + 32, tmp2);
			hdf_file_seek64 (hfd->fd, (long long)(hfd->offset + offset));
			while (tmp2) {
				int i;
				char buffer[512];
				hdf_file_read (hfd->fd, buffer, 512);
				for (i = 0; i < 512; i++, dataptr++)
					put_byte(dataptr, buffer[i]);
				tmp2 -= 512;
			}
			break;
			
		case CMD_WRITE:
		case 11:
		case 25:
		case 27:
		case 0x4003:
		case 0x400B:
			gui_data.hdled = HDLED_WRITE;
			
			dataptr = get_long (tmp1 + 40);
			if (tmp2 == 25 || tmp2 == 27)
				offset = ((unsigned long long)(uae_u32)get_long (tmp1 + 32) << 32) | (unsigned long long)(uae_u32)get_long (tmp1 + 44);
			else if (tmp2 == 0x4003 || tmp2 == 0x400B)
				offset = ((unsigned long long)(uae_u32)get_long (tmp1 + 48) << 32) | (unsigned long long)(uae_u32)get_long (tmp1 + 44);
			else
				offset = (unsigned long long)(uae_u32)get_long (tmp1 + 44);
			tmp2 = get_long (tmp1 + 36);

			if (dataptr & 1 || offset & 511 || tmp2 & 511 || offset + (unsigned long long)tmp2 > hfd->size)
				goto bad_command;
			
			put_long (tmp1 + 32, tmp2);
			hdf_file_seek64 (hfd->fd, (long long)(hfd->offset + offset));
			while (tmp2) {
				char buffer[512];
				int i;
				for (i=0; i < 512; i++, dataptr++)
					buffer[i] = get_byte(dataptr);
				hdf_file_write (hfd->fd, buffer, 512);
				tmp2 -= 512;
			}
			break;
			
			bad_command:
			put_byte (tmp1+31, (uae_u8)-3);
			break;
			
		case 18:
			put_long (tmp1 + 32, get_hardfile_readonly (unit) ? 1 : 0);
			break;
			
		case 19:
			put_long (tmp1 + 32, 0);
			break;
			
		case CMD_UPDATE:
		case CMD_CLEAR:
		case 9:
		case 10:
		case 12:
		case 13:
		case 14:
		case 15:
		case 20:
		case 21:
		case 26:
		case 0x400A:
			put_long (tmp1+32, 0);
			retval = 0;
			break;

		case 22:
		{
			uae_u32 gptr = get_long (tmp1 + 40);
			uae_u32 glen = get_long (tmp1 + 36);
			int i;

			if (gptr == 0 || glen < 24) {
				put_long (tmp1 + 32, 0);
				put_byte (tmp1 + 31, (uae_u8)-3);
				retval = 0;
				break;
			}
			if (glen > 96)
				glen = 96;
			for (i = 0; i < (int)glen; i++)
				put_byte (gptr + i, 0);
			put_long (gptr + 0, (uae_u32)hfd->blocksize);
			put_long (gptr + 4, (uae_u32)(hfd->size / (unsigned long long)hfd->blocksize));
			put_long (gptr + 8, hfd->nrcyls > 0 ? (uae_u32)hfd->nrcyls : 0);
			put_long (gptr + 12, (uae_u32)(hfd->surfaces * hfd->secspertrack));
			put_long (gptr + 16, (uae_u32)hfd->surfaces);
			put_long (gptr + 20, (uae_u32)hfd->secspertrack);
			put_long (tmp1 + 32, glen);
			retval = 0;
			break;
		}

		case 0x4000:
		{
			uaecptr query = get_long (tmp1 + 40);
			uae_u32 qlen = get_long (tmp1 + 36);
			if (query == 0 || qlen < 16) {
				put_byte (tmp1 + 31, (uae_u8)-3);
				put_long (tmp1 + 32, 0);
				retval = 0;
				break;
			}
			put_long (query + 0, 0);
			put_long (query + 4, 16);
			put_word (query + 8, 5);
			put_word (query + 10, 0);
			put_long (query + 12, hardfile_nsd_cmdlist);
			put_long (tmp1 + 32, 16);
			put_byte (tmp1 + 31, 0);
			retval = 0;
			break;
		}

		case 28:
		{
			uaecptr scsicmd = get_long (tmp1 + 40);
			if (!scsicmd) {
				put_byte (tmp1 + 31, (uae_u8)-3);
				put_long (tmp1 + 32, 0);
				retval = 0;
				break;
			}
			uaecptr scsi_data = get_long (scsicmd + 0);
			uae_u32 scsi_len = get_long (scsicmd + 4);
			uaecptr scsi_cmd = get_long (scsicmd + 12);
			uae_u16 scsi_cmdlen = get_word (scsicmd + 16);
			if (!scsi_cmd || scsi_cmdlen < 1) {
				put_byte (tmp1 + 31, (uae_u8)-3);
				put_long (tmp1 + 32, 0);
				retval = 0;
				break;
			}
			uae_u8 opcode = get_byte (scsi_cmd + 0);
			if (opcode == 0x00) {
				put_long (scsicmd + 8, 0);
				put_word (scsicmd + 18, scsi_cmdlen);
				put_byte (scsicmd + 21, 0);
				put_byte (tmp1 + 31, 0);
				put_long (tmp1 + 32, 0);
				retval = 0;
				break;
			}
			if (opcode == 0x03) {
				uae_u32 slen = scsi_len > 18 ? 18 : scsi_len;
				unsigned char sense[18];
				memset (sense, 0, sizeof(sense));
				sense[0] = 0x70;
				sense[7] = 10;
				for (uae_u32 i = 0; i < slen; i++)
					put_byte (scsi_data + i, sense[i]);
				put_long (scsicmd + 8, slen);
				put_word (scsicmd + 18, scsi_cmdlen);
				put_byte (scsicmd + 21, 0);
				put_byte (tmp1 + 31, 0);
				put_long (tmp1 + 32, 0);
				retval = 0;
				break;
			}
			if (opcode == 0x12) {
				uae_u32 slen = scsi_len > 36 ? 36 : scsi_len;
				unsigned char inq[36];
				memset (inq, 0, sizeof(inq));
				inq[0] = 0x00;
				inq[1] = 0x00;
				inq[2] = 0x02;
				inq[3] = 0x02;
				inq[4] = 31;
				memcpy (&inq[8], "UAE     ", 8);
				memcpy (&inq[16], "Harddisk        ", 16);
				memcpy (&inq[32], "0.4 ", 4);
				for (uae_u32 i = 0; i < slen; i++)
					put_byte (scsi_data + i, inq[i]);
				put_long (scsicmd + 8, slen);
				put_word (scsicmd + 18, scsi_cmdlen);
				put_byte (scsicmd + 21, 0);
				put_byte (tmp1 + 31, 0);
				put_long (tmp1 + 32, 0);
				retval = 0;
				break;
			}
			if (opcode == 0x15 || opcode == 0x55) {
				put_long (scsicmd + 8, scsi_len);
				put_word (scsicmd + 18, scsi_cmdlen);
				put_byte (scsicmd + 21, 0);
				put_byte (tmp1 + 31, 0);
				put_long (tmp1 + 32, 0);
				retval = 0;
				break;
			}
			if (opcode == 0x1B) {
				put_long (scsicmd + 8, 0);
				put_word (scsicmd + 18, scsi_cmdlen);
				put_byte (scsicmd + 21, 0);
				put_byte (tmp1 + 31, 0);
				put_long (tmp1 + 32, 0);
				retval = 0;
				break;
			}
			if (opcode == 0x25) {
				unsigned long long total_sec = hfd->size / (unsigned long long)hfd->blocksize;
				unsigned long long last_lba = total_sec > 0 ? total_sec - 1 : 0;
				if (last_lba > 0xFFFFFFFFULL)
					last_lba = 0xFFFFFFFFULL;
				if (scsi_len >= 8) {
					put_long (scsi_data + 0, (uae_u32)last_lba);
					put_long (scsi_data + 4, (uae_u32)hfd->blocksize);
					put_long (scsicmd + 8, 8);
				} else {
					put_long (scsicmd + 8, 0);
				}
				put_word (scsicmd + 18, scsi_cmdlen);
				put_byte (scsicmd + 21, 0);
				put_byte (tmp1 + 31, 0);
				put_long (tmp1 + 32, 0);
				retval = 0;
				break;
			}
			if (opcode == 0x08 || opcode == 0x28) {
				uae_u32 lba = 0;
				uae_u32 count = 0;
				if (opcode == 0x08) {
					lba = (((uae_u32)get_byte (scsi_cmd + 1) & 0x1F) << 16) |
					      ((uae_u32)get_byte (scsi_cmd + 2) << 8) |
					      (uae_u32)get_byte (scsi_cmd + 3);
					count = (uae_u32)get_byte (scsi_cmd + 4);
					if (count == 0) count = 256;
				} else {
					lba = ((uae_u32)get_byte (scsi_cmd + 2) << 24) |
					      ((uae_u32)get_byte (scsi_cmd + 3) << 16) |
					      ((uae_u32)get_byte (scsi_cmd + 4) << 8) |
					      (uae_u32)get_byte (scsi_cmd + 5);
					count = ((uae_u32)get_byte (scsi_cmd + 7) << 8) |
					        (uae_u32)get_byte (scsi_cmd + 8);
				}
				unsigned long long byte_off = (unsigned long long)lba * (unsigned long long)hfd->blocksize;
				unsigned long long byte_len = (unsigned long long)count * (unsigned long long)hfd->blocksize;
				if (byte_off + byte_len > hfd->size || byte_len > scsi_len) {
					put_byte (scsicmd + 21, 2);
					put_byte (tmp1 + 31, (uae_u8)-3);
					put_long (tmp1 + 32, 0);
					retval = 0;
					break;
				}
				gui_data.hdled = HDLED_READ;
				hdf_file_seek64 (hfd->fd, (long long)(hfd->offset + byte_off));
				uaecptr dptr = scsi_data;
				uae_u32 rem = (uae_u32)byte_len;
				while (rem) {
					char buf[512];
					int n = rem > 512 ? 512 : (int)rem;
					hdf_file_read (hfd->fd, buf, n);
					for (int i = 0; i < n; i++, dptr++)
						put_byte (dptr, buf[i]);
					rem -= (uae_u32)n;
				}
				put_long (scsicmd + 8, (uae_u32)byte_len);
				put_word (scsicmd + 18, scsi_cmdlen);
				put_byte (scsicmd + 21, 0);
				put_byte (tmp1 + 31, 0);
				put_long (tmp1 + 32, 0);
				retval = 0;
				break;
			}
			if (opcode == 0x0A || opcode == 0x2A) {
				if (get_hardfile_readonly (unit)) {
					put_byte (scsicmd + 21, 2);
					put_byte (tmp1 + 31, (uae_u8)-3);
					put_long (tmp1 + 32, 0);
					retval = 0;
					break;
				}
				uae_u32 lba = 0;
				uae_u32 count = 0;
				if (opcode == 0x0A) {
					lba = (((uae_u32)get_byte (scsi_cmd + 1) & 0x1F) << 16) |
					      ((uae_u32)get_byte (scsi_cmd + 2) << 8) |
					      (uae_u32)get_byte (scsi_cmd + 3);
					count = (uae_u32)get_byte (scsi_cmd + 4);
					if (count == 0) count = 256;
				} else {
					lba = ((uae_u32)get_byte (scsi_cmd + 2) << 24) |
					      ((uae_u32)get_byte (scsi_cmd + 3) << 16) |
					      ((uae_u32)get_byte (scsi_cmd + 4) << 8) |
					      (uae_u32)get_byte (scsi_cmd + 5);
					count = ((uae_u32)get_byte (scsi_cmd + 7) << 8) |
					        (uae_u32)get_byte (scsi_cmd + 8);
				}
				unsigned long long byte_off = (unsigned long long)lba * (unsigned long long)hfd->blocksize;
				unsigned long long byte_len = (unsigned long long)count * (unsigned long long)hfd->blocksize;
				if (byte_off + byte_len > hfd->size || byte_len > scsi_len) {
					put_byte (scsicmd + 21, 2);
					put_byte (tmp1 + 31, (uae_u8)-3);
					put_long (tmp1 + 32, 0);
					retval = 0;
					break;
				}
				gui_data.hdled = HDLED_WRITE;
				hdf_file_seek64 (hfd->fd, (long long)(hfd->offset + byte_off));
				uaecptr dptr = scsi_data;
				uae_u32 rem = (uae_u32)byte_len;
				while (rem) {
					char buf[512];
					int n = rem > 512 ? 512 : (int)rem;
					for (int i = 0; i < n; i++, dptr++)
						buf[i] = get_byte (dptr);
					hdf_file_write (hfd->fd, buf, n);
					rem -= (uae_u32)n;
				}
				put_long (scsicmd + 8, (uae_u32)byte_len);
				put_word (scsicmd + 18, scsi_cmdlen);
				put_byte (scsicmd + 21, 0);
				put_byte (tmp1 + 31, 0);
				put_long (tmp1 + 32, 0);
				retval = 0;
				break;
			}
			if (opcode == 0x1A || opcode == 0x5A) {
				uae_u8 page = get_byte (scsi_cmd + 2) & 0x3F;
				unsigned char mdata[64];
				memset (mdata, 0, sizeof(mdata));
				int mlen = 0;
				int is_10 = (opcode == 0x5A);
				int ro = get_hardfile_readonly (unit);

				if (page == 0x04 || page == 0x3F) {
					if (!is_10) {
						mdata[0] = 23;
						mdata[1] = 0;
						mdata[2] = ro ? 0x80 : 0x00;
						mdata[3] = 0;
						mdata[4] = 4;
						mdata[5] = 18;
						mdata[6] = (uae_u8)((hfd->nrcyls >> 16) & 0xFF);
						mdata[7] = (uae_u8)((hfd->nrcyls >> 8) & 0xFF);
						mdata[8] = (uae_u8)(hfd->nrcyls & 0xFF);
						mdata[9] = (uae_u8)hfd->surfaces;
						mlen = 24;
					} else {
						mdata[0] = 0;
						mdata[1] = 26;
						mdata[2] = 0;
						mdata[3] = ro ? 0x80 : 0x00;
						mdata[4] = 0;
						mdata[5] = 0;
						mdata[6] = 0;
						mdata[7] = 0;
						mdata[8] = 4;
						mdata[9] = 18;
						mdata[10] = (uae_u8)((hfd->nrcyls >> 16) & 0xFF);
						mdata[11] = (uae_u8)((hfd->nrcyls >> 8) & 0xFF);
						mdata[12] = (uae_u8)(hfd->nrcyls & 0xFF);
						mdata[13] = (uae_u8)hfd->surfaces;
						mlen = 28;
					}
				} else if (page == 0x03) {
					if (!is_10) {
						mdata[0] = 27;
						mdata[1] = 0;
						mdata[2] = ro ? 0x80 : 0x00;
						mdata[3] = 0;
						mdata[4] = 3;
						mdata[5] = 22;
						mdata[14] = (uae_u8)((hfd->secspertrack >> 8) & 0xFF);
						mdata[15] = (uae_u8)(hfd->secspertrack & 0xFF);
						mdata[16] = (uae_u8)((hfd->blocksize >> 8) & 0xFF);
						mdata[17] = (uae_u8)(hfd->blocksize & 0xFF);
						mdata[19] = 1;
						mlen = 28;
					} else {
						mdata[0] = 0;
						mdata[1] = 30;
						mdata[2] = 0;
						mdata[3] = ro ? 0x80 : 0x00;
						mdata[4] = 0;
						mdata[5] = 0;
						mdata[6] = 0;
						mdata[7] = 0;
						mdata[8] = 3;
						mdata[9] = 22;
						mdata[18] = (uae_u8)((hfd->secspertrack >> 8) & 0xFF);
						mdata[19] = (uae_u8)(hfd->secspertrack & 0xFF);
						mdata[20] = (uae_u8)((hfd->blocksize >> 8) & 0xFF);
						mdata[21] = (uae_u8)(hfd->blocksize & 0xFF);
						mdata[23] = 1;
						mlen = 32;
					}
				} else if (page == 0x00) {
					if (!is_10) {
						mdata[0] = 3;
						mdata[1] = 0;
						mdata[2] = ro ? 0x80 : 0x00;
						mdata[3] = 0;
						mlen = 4;
					} else {
						mdata[0] = 0;
						mdata[1] = 6;
						mdata[2] = 0;
						mdata[3] = ro ? 0x80 : 0x00;
						mdata[4] = 0;
						mdata[5] = 0;
						mdata[6] = 0;
						mdata[7] = 0;
						mlen = 8;
					}
				}

				if (mlen > 0) {
					uae_u32 to_copy = (uae_u32)mlen > scsi_len ? scsi_len : (uae_u32)mlen;
					for (uae_u32 i = 0; i < to_copy; i++)
						put_byte (scsi_data + i, mdata[i]);
					put_long (scsicmd + 8, to_copy);
					put_word (scsicmd + 18, scsi_cmdlen);
					put_byte (scsicmd + 21, 0);
					put_byte (tmp1 + 31, 0);
					put_long (tmp1 + 32, 0);
					retval = 0;
					break;
				}

				put_long (scsicmd + 8, 0);
				put_word (scsicmd + 18, scsi_cmdlen);
				put_byte (scsicmd + 21, 2);
				put_byte (tmp1 + 31, 0);
				put_long (tmp1 + 32, 0);
				retval = 0;
				break;
			}
			put_long (scsicmd + 8, 0);
			put_word (scsicmd + 18, scsi_cmdlen);
			put_byte (scsicmd + 21, 2);
			uaecptr sense_ptr = get_long (scsicmd + 22);
			uae_u16 sense_len = get_word (scsicmd + 26);
			if (sense_ptr && sense_len >= 18) {
				unsigned char sbuf[18];
				memset (sbuf, 0, sizeof(sbuf));
				sbuf[0] = 0x70;
				sbuf[2] = 0x05;
				sbuf[7] = 10;
				sbuf[12] = 0x20;
				for (int i = 0; i < 18; i++)
					put_byte (sense_ptr + i, sbuf[i]);
				put_word (scsicmd + 28, 18);
			}
			put_byte (tmp1 + 31, 0);
			put_long (tmp1 + 32, 0);
			retval = 0;
			break;
		}

		default:
			put_byte (tmp1+31, (uae_u8)-3);
			retval = 0;
			break;
	}
	return retval;
}

static uae_u32 hardfile_abortio (void)
{
    return (uae_u32)-3;
}

void hardfile_install (void)
{
    uae_u32 functable, datatable;
    uae_u32 initcode, openfunc, closefunc, expungefunc;
    uae_u32 beginiofunc, abortiofunc;

    ROM_hardfile_resname = ds ("uaehf.device");
    ROM_hardfile_resid = ds ("UAE hardfile.device 0.2");

    align (2);
    hardfile_nsd_cmdlist = here ();
    dw (CMD_READ);
    dw (CMD_WRITE);
    dw (CMD_UPDATE);
    dw (CMD_CLEAR);
    dw (9);
    dw (10);
    dw (11);
    dw (12);
    dw (13);
    dw (14);
    dw (15);
    dw (18);
    dw (19);
    dw (20);
    dw (21);
    dw (22);
    dw (24);
    dw (25);
    dw (26);
    dw (27);
    dw (28);
    dw (0x4000);
    dw (0x4002);
    dw (0x4003);
    dw (0x400A);
    dw (0x400B);
    dw (0);

    /* initcode */
    initcode = filesys_initcode;

    /* Open */
    openfunc = here ();
    calltrap (deftrap (hardfile_open)); dw (RTS);

    /* Close */
    closefunc = here ();
    calltrap (deftrap (hardfile_close)); dw (RTS);

    /* Expunge */
    expungefunc = here ();
    calltrap (deftrap (hardfile_expunge)); dw (RTS);

    /* BeginIO */
    beginiofunc = here ();
    calltrap (deftrap (hardfile_beginio));
    dw (0x48E7); dw (0x8002); /* movem.l d0/a6,-(a7) */
    dw (0x0829); dw (0); dw (30); /* btst #0,30(a1) */
    dw (0x6608); /* bne.b +8 */
    dw (0x2C78); dw (0x0004); /* move.l 4,a6 */
    dw (0x4EAE); dw (-378); /* jsr ReplyMsg(a6) */
    dw (0x4CDF); dw (0x4001); /* movem.l (a7)+,d0/a6 */
    dw (RTS);

    /* AbortIO */
    abortiofunc = here ();
    calltrap (deftrap (hardfile_abortio)); dw (RTS);

    /* FuncTable */
    functable = here ();
    dl (openfunc); /* Open */
    dl (closefunc); /* Close */
    dl (expungefunc); /* Expunge */
    dl (EXPANSION_nullfunc); /* Null */
    dl (beginiofunc); /* BeginIO */
    dl (abortiofunc); /* AbortIO */
    dl (0xFFFFFFFFul); /* end of table */

    /* DataTable */
    datatable = here ();
    dw (0xE000); /* INITBYTE */
    dw (0x0008); /* LN_TYPE */
    dw (0x0300); /* NT_DEVICE */
    dw (0xC000); /* INITLONG */
    dw (0x000A); /* LN_NAME */
    dl (ROM_hardfile_resname);
    dw (0xE000); /* INITBYTE */
    dw (0x000E); /* LIB_FLAGS */
    dw (0x0600); /* LIBF_SUMUSED | LIBF_CHANGED */
    dw (0xD000); /* INITWORD */
    dw (0x0014); /* LIB_VERSION */
    dw (0x0004); /* 0.4 */
    dw (0xD000);
    dw (0x0016); /* LIB_REVISION */
    dw (0x0000);
    dw (0xC000);
    dw (0x0018); /* LIB_IDSTRING */
    dl (ROM_hardfile_resid);
    dw (0x0000); /* end of table */

    ROM_hardfile_init = here ();
    dl (0x00000100); /* ??? */
    dl (functable);
    dl (datatable);
    dl (initcode);
}
