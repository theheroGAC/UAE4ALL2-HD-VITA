/*
 * cover_downloader.cpp - WHDLoad boxart downloader for UAE4ALL2-HD-VITA
 *
 * Fetches cover art from the libretro-thumbnails repository (raw GitHub)
 * into ux0:/data/uae4all/covers/<GameName>.png using the Vita's SceHttp
 * stack (HTTP + HTTPS). The base URL can be overridden with a single line
 * in ux0:/data/uae4all/covers/source.txt.
 */

#ifdef __PSP2__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <psp2/net/net.h>
#include <psp2/net/http.h>
#include <psp2/net/netctl.h>
#include <psp2/libssl.h>
#include <psp2/sysmodule.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>

#include "cover_downloader.h"

extern "C" void vita_gui_draw_progress(const char *title, const char *subtitle, float fraction, const char *item_name);

#define COVER_DIR "ux0:/data/uae4all/covers"
#define COVER_SOURCE_FILE COVER_DIR "/source.txt"

#define DEFAULT_COVER_BASE \
    "https://raw.githubusercontent.com/libretro-thumbnails/libretro-thumbnails/master/Commodore%20-%20Amiga/Named_Snaps"

static int s_net_inited = 0;

static int cover_net_init(void)
{
    if (s_net_inited)
        return 0;

    /* Loading an already-loaded module returns an error; ignore it. */
    sceSysmoduleLoadModule(SCE_SYSMODULE_NET);
    sceSysmoduleLoadModule(SCE_SYSMODULE_HTTP);
    sceSysmoduleLoadModule(SCE_SYSMODULE_SSL);

    {
        SceNetInitParam param;
        memset(&param, 0, sizeof(param));
        param.memory = malloc(0x40000);
        param.size = 0x40000;
        if (!param.memory)
            return -1;
        if (sceNetInit(&param) < 0) {
            free(param.memory);
            return -1;
        }
    }

    if (sceNetCtlInit() < 0)
        return -1;
    if (sceSslInit(0x100000) < 0)
        return -1;
    if (sceHttpInit(0x100000) < 0)
        return -1;

    s_net_inited = 1;
    return 0;
}

static void cover_ensure_dir(void)
{
    sceIoMkdir("ux0:/data", 0777);
    sceIoMkdir("ux0:/data/uae4all", 0777);
    sceIoMkdir(COVER_DIR, 0777);
}

/* Percent-encode a game name for use in a URL path. */
static void cover_url_encode(const char *src, char *dst, size_t dst_size)
{
    static const char hex[] = "0123456789ABCDEF";
    size_t out = 0;
    for (const unsigned char *p = (const unsigned char *)src; *p && out + 3 < dst_size; p++) {
        unsigned char c = *p;
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            dst[out++] = (char)c;
        } else {
            dst[out++] = '%';
            dst[out++] = hex[(c >> 4) & 0xF];
            dst[out++] = hex[c & 0xF];
        }
    }
    dst[out] = '\0';
}

/* Read the optional server override (single line, no trailing slash). */
static void cover_read_base_url(char *base, size_t base_size)
{
    strncpy(base, DEFAULT_COVER_BASE, base_size - 1);
    base[base_size - 1] = '\0';

    FILE *f = fopen(COVER_SOURCE_FILE, "rb");
    if (!f)
        return;
    char line[768];
    if (fgets(line, sizeof(line), f)) {
        /* Trim whitespace / CR / LF. */
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r' || line[len - 1] == ' ')) {
            line[len - 1] = '\0';
            len--;
        }
        if (len > 0 && strstr(line, "://") != NULL) {
            strncpy(base, line, base_size - 1);
            base[base_size - 1] = '\0';
        }
    }
    fclose(f);
}

int vita_cover_download(const char *game_name, char *out_path, size_t out_path_size)
{
    char url[1024];
    char encoded[256];
    char base[768];
    int tmpl = -1, conn = -1, req = -1;
    int result = -3;
    SceUID fd = -1;

    if (!game_name || game_name[0] == '\0' || !out_path || out_path_size == 0)
        return -1;

    cover_ensure_dir();
    if (cover_net_init() != 0)
        return -2;

    cover_read_base_url(base, sizeof(base));
    cover_url_encode(game_name, encoded, sizeof(encoded));
    snprintf(url, sizeof(url), "%s/%s.png", base, encoded);

    tmpl = sceHttpCreateTemplate("uae4all2-cover", SCE_HTTP_VERSION_1_1, 0);
    if (tmpl < 0)
        return -2;
    sceHttpSetResolveTimeOut(tmpl, 10 * 1000);
    sceHttpSetConnectTimeOut(tmpl, 10 * 1000);
    sceHttpSetRecvTimeOut(tmpl, 20 * 1000);

    conn = sceHttpCreateConnectionWithURL(tmpl, url, 0);
    if (conn < 0)
        goto cleanup;

    req = sceHttpCreateRequestWithURL(conn, SCE_HTTP_METHOD_GET, url, 0);
    if (req < 0)
        goto cleanup;

    if (sceHttpSendRequest(req, NULL, 0) < 0)
        goto cleanup;

    {
        int status = 0;
        if (sceHttpGetStatusCode(req, &status) < 0 || status != 200) {
            result = -6; /* cover not found (404 etc.) */
            goto cleanup;
        }
    }

    {
        unsigned char buf[8192];
        SceOff total = 0;

        for (;;) {
            int r = sceHttpReadData(req, buf, sizeof(buf));
            if (r < 0) {
                result = -4;
                goto cleanup;
            }
            if (r == 0)
                break;
            if (fd < 0) {
                fd = sceIoOpen(out_path, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
                if (fd < 0) {
                    result = -5;
                    goto cleanup;
                }
            }
            if (sceIoWrite(fd, buf, r) != r) {
                result = -5;
                goto cleanup;
            }
            total += r;
            if ((total & 0x1FFFF) == 0) {
                char sub[64];
                snprintf(sub, sizeof(sub), "%d KB received", (int)(total / 1024));
                vita_gui_draw_progress("Downloading Cover...", sub, 0.5f, game_name);
            }
        }
        if (total < 100) { /* not a real image */
            result = -6;
            goto cleanup;
        }
        result = 0;
    }

cleanup:
    if (fd >= 0)
        sceIoClose(fd);
    if (req >= 0)
        sceHttpDeleteRequest(req);
    if (conn >= 0)
        sceHttpDeleteConnection(conn);
    if (tmpl >= 0)
        sceHttpDeleteTemplate(tmpl);
    if (result != 0 && out_path[0] != '\0')
        sceIoRemove(out_path);

    return result;
}

#endif /* __PSP2__ */
