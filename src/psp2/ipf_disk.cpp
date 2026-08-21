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
    int mincylinder;
    int maxcylinder;
    int minhead;
    int maxhead;
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
    image->mincylinder = (int)info.mincylinder;
    image->maxcylinder = (int)info.maxcylinder;
    image->minhead = (int)info.minhead;
    image->maxhead = (int)info.maxhead;
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

    int caps_cylinder = image->mincylinder + cylinder;
    int caps_head = image->minhead + head;
    if (caps_cylinder < image->mincylinder || caps_cylinder > image->maxcylinder ||
        caps_head < image->minhead || caps_head > image->maxhead)
        return 0;

    CapsTrackInfo track;
    memset(&track, 0, sizeof(track));

    /* Use DI_LOCK_TRKBIT so that track.tracklen returns the real
       bit count of the track instead of the byte size of the internal
       buffer (which may include multi-revolution data and padding). */
    int result = CAPSLockTrack(&track, image->id, (UDWORD)caps_cylinder,
                               (UDWORD)caps_head,
                               DI_LOCK_INDEX | DI_LOCK_ALIGN |
                               DI_LOCK_UPDATEFD | DI_LOCK_TRKBIT);
    if (result != imgeOk || !track.trackbuf || track.tracklen == 0) {
        CAPSUnlockTrack(image->id, (UDWORD)caps_cylinder, (UDWORD)caps_head);
        return 0;
    }

    /* track.tracklen is now in bits (thanks to DI_LOCK_TRKBIT).
       Calculate how many bytes we need to copy for one revolution. */
    int real_bits = (int)track.tracklen;
    int bytes = (real_bits + 7) / 8;
    if (bytes > destination_size)
        bytes = destination_size;
    if (bytes <= 0) {
        CAPSUnlockTrack(image->id, (UDWORD)caps_cylinder, (UDWORD)caps_head);
        return 0;
    }
    memcpy(destination, track.trackbuf, (size_t)bytes);
    *track_bits = real_bits;

    CAPSUnlockTrack(image->id, (UDWORD)caps_cylinder, (UDWORD)caps_head);
    return bytes;
}

#endif
