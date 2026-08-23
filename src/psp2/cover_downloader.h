#ifndef UAE_COVER_DOWNLOADER_H
#define UAE_COVER_DOWNLOADER_H

#ifdef __PSP2__

#ifdef __cplusplus
extern "C" {
#endif

int vita_cover_download(const char *game_name, char *out_path, size_t out_path_size);

#ifdef __cplusplus
}
#endif

#endif

#endif