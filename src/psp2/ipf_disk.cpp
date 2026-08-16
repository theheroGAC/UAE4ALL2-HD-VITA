#ifdef __PSP2__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sysconfig.h"
#include "sysdeps.h"
#ifndef __cdecl
#define __cdecl
#endif
#include "capsimg/LibIPF/CapsLibAll.h"
#include "ipf_disk.h"

struct VitaIpfImage {
    int id;
    int tracks;
};

static int s_caps_users = 0;

VitaIpfImage *vita_ipf_open(const char *filename)
{
    if (!filename || filename[0] == '\0')
        return NULL;

    if (s_caps_users == 0 && CAPSInit() != imgeOk)
        return NULL;

    int id = CAPSAddImage();
    if (id < 0)
        return NULL;

    int result = CAPSLockImage(id, (PCHAR)filename);
    if (result == imgeOk)
        result = CAPSLoadImage(id, 0);
    if (result != imgeOk) {
        CAPSUnlockImage(id);
        CAPSRemImage(id);
        if (s_caps_users == 0)
            CAPSExit();
        return NULL;
    }

    CapsImageInfo info;
    memset(&info, 0, sizeof(info));
    result = CAPSGetImageInfo(&info, id);
    if (result != imgeOk || info.type != ciitFDD || info.maxcylinder < info.mincylinder) {
        CAPSUnlockImage(id);
        CAPSRemImage(id);
        if (s_caps_users == 0)
            CAPSExit();
        return NULL;
    }

    VitaIpfImage *image = (VitaIpfImage *)calloc(1, sizeof(VitaIpfImage));
    if (!image) {
        CAPSUnlockImage(id);
        CAPSRemImage(id);
        if (s_caps_users == 0)
            CAPSExit();
        return NULL;
    }

    image->id = id;
    image->tracks = (int)((info.maxcylinder - info.mincylinder + 1) *
                          (info.maxhead - info.minhead + 1));
    s_caps_users++;
    return image;
}

void vita_ipf_close(VitaIpfImage *image)
{
    if (!image)
        return;

    CAPSUnlockImage(image->id);
    CAPSRemImage(image->id);
    free(image);

    if (s_caps_users > 0)
        s_caps_users--;
    if (s_caps_users == 0)
        CAPSExit();
}

int vita_ipf_get_track_count(const VitaIpfImage *image)
{
    return image ? image->tracks : 0;
}

int vita_ipf_read_track(VitaIpfImage *image, int cylinder, int head,
                        unsigned char *destination, int destination_size,
                        int *track_bits)
{
    if (!image || !destination || destination_size <= 0 || !track_bits)
        return 0;

    CapsTrackInfo track;
    memset(&track, 0, sizeof(track));

                                                           
    int result = CAPSLockTrack(&track, image->id, (UDWORD)cylinder,
                               (UDWORD)head, DI_LOCK_INDEX | DI_LOCK_ALIGN);
    if (result != imgeOk || !track.trackbuf || track.tracklen == 0) {
        CAPSUnlockTrack(image->id, (UDWORD)cylinder, (UDWORD)head);
        return 0;
    }

    int bytes = (int)track.tracklen;
    if (bytes > destination_size)
        bytes = destination_size;
    memcpy(destination, track.trackbuf, (size_t)bytes);
    *track_bits = (int)track.tracklen * 8;

    CAPSUnlockTrack(image->id, (UDWORD)cylinder, (UDWORD)head);
    return bytes;
}

#endif
