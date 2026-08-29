#ifndef UAE_FTP_SERVER_H
#define UAE_FTP_SERVER_H

#ifdef __PSP2__

#ifdef __cplusplus
extern "C" {
#endif

int vita_ftp_start(void);
void vita_ftp_stop(void);
int vita_ftp_is_running(void);
int vita_ftp_get_port(void);
int vita_ftp_get_ip(char *buffer, int buffer_size);

#ifdef __cplusplus
}
#endif

#endif

#endif
