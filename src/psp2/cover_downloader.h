#ifndef UAE_COVER_DOWNLOADER_H
#define UAE_COVER_DOWNLOADER_H

#ifdef __PSP2__

#ifdef __cplusplus
extern "C" {
#endif

/* Download the boxart for a WHDLoad game into
 * ux0:/data/uae4all/covers/<game>.png and return its path in out_path.
 *
 * The cover server base URL defaults to the libretro-thumbnails repository
 * and can be overridden by placing a single-line URL (no trailing slash)
 * in ux0:/data/uae4all/covers/source.txt.
 *
 * Returns 0 on success, a negative error code on failure:
 *  -1 invalid arguments, -2 network init failed, -3 HTTP request failed,
 *  -4 read error, -5 file write error, -6 cover not found on server. */
int vita_cover_download(const char *game_name, char *out_path, size_t out_path_size);

#ifdef __cplusplus
}
#endif

#endif /* __PSP2__ */

#endif /* UAE_COVER_DOWNLOADER_H */
