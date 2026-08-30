#ifdef __PSP2__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <psp2/sysmodule.h>
#include <psp2/net/net.h>
#include <psp2/net/netctl.h>
extern "C" {
#include <ftpvita.h>
}

#include "ftp_server.h"

static int s_net_initialized = 0;
static int s_ftp_active = 0;
static char s_ip[32] = "0.0.0.0";
static unsigned short s_port = 1337;

static void ftp_info_log_callback(const char *message)
{
    (void)message;
}

static int ftp_network_init(void)
{
    if (s_net_initialized)
        return 1;

    if (sceSysmoduleLoadModule(SCE_SYSMODULE_NET) < 0)
        return 0;

    SceNetInitParam param;
    memset(&param, 0, sizeof(param));
    param.memory = malloc(128 * 1024);
    param.size = 128 * 1024;
    param.flags = 0;
    if (!param.memory)
        return 0;

    if (sceNetInit(&param) < 0) {
        free(param.memory);
        return 0;
    }
    if (sceNetCtlInit() < 0) {
        sceNetTerm();
        free(param.memory);
        return 0;
    }

    s_net_initialized = 1;
    return 1;
}

static void ftp_network_term(void)
{
    if (!s_net_initialized)
        return;
    sceNetCtlTerm();
    sceNetTerm();
    s_net_initialized = 0;
}

int vita_ftp_start(void)
{
    if (s_ftp_active)
        return 0;
    if (!ftp_network_init())
        return -1;

    SceNetCtlInfo net_info;
    memset(&net_info, 0, sizeof(net_info));
    if (sceNetCtlInetGetInfo(SCE_NETCTL_INFO_GET_IP_ADDRESS, &net_info) < 0 || net_info.ip_address[0] == '\0') {
        ftp_network_term();
        return -1;
    }
    strncpy(s_ip, net_info.ip_address, sizeof(s_ip) - 1);
    s_ip[sizeof(s_ip) - 1] = '\0';
    s_port = 1337;
    ftpvita_set_info_log_cb(ftp_info_log_callback);
    if (ftpvita_init(s_ip, &s_port) < 0) {
        ftp_network_term();
        strcpy(s_ip, "0.0.0.0");
        return -1;
    }

    ftpvita_add_device("app0:");
    ftpvita_add_device("ux0:");
    ftpvita_add_device("ur0:");
    ftpvita_add_device("uma0:");
    ftpvita_add_device("gro0:");
    ftpvita_add_device("grw0:");
    ftpvita_add_device("os0:");
    ftpvita_add_device("pd0:");
    ftpvita_add_device("sa0:");
    ftpvita_add_device("tm0:");
    ftpvita_add_device("ud0:");
    ftpvita_add_device("vd0:");
    ftpvita_add_device("vs0:");
    ftpvita_set_file_buf_size(512 * 1024);
    s_ftp_active = 1;
    return 0;
}

void vita_ftp_stop(void)
{
    if (!s_ftp_active)
        return;
    ftpvita_fini();
    ftp_network_term();
    s_ftp_active = 0;
}

int vita_ftp_is_running(void)
{
    return s_ftp_active;
}

int vita_ftp_get_port(void)
{
    return (int)s_port;
}

int vita_ftp_get_ip(char *buffer, int buffer_size)
{
    if (!buffer || buffer_size <= 0)
        return -1;
    strncpy(buffer, s_ip[0] ? s_ip : "0.0.0.0", (size_t)buffer_size - 1);
    buffer[buffer_size - 1] = '\0';
    return 0;
}

#endif
