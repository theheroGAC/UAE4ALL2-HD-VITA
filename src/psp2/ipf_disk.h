#ifndef UAE_VITA_IPF_DISK_H
#define UAE_VITA_IPF_DISK_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VitaIpfImage VitaIpfImage;

VitaIpfImage *vita_ipf_open(const char *filename);
void vita_ipf_close(VitaIpfImage *image);
int vita_ipf_get_track_count(const VitaIpfImage *image);
int vita_ipf_read_track(VitaIpfImage *image, int cylinder, int head,
                        unsigned char *destination, int destination_size,
                        int *track_bits);

#ifdef __cplusplus
}
#endif

#endif
