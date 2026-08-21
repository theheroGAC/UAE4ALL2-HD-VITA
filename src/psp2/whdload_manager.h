#ifndef UAE_WHDLOAD_MANAGER_H
#define UAE_WHDLOAD_MANAGER_H

#ifdef __PSP2__

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VITA_WHDLOAD_ROOT "ux0:/data/uae4all/whdload"

int vita_whdload_install_lha(const char *archive_path, char *installed_path, size_t installed_path_size);
int vita_whdload_list(char names[][128], int max_names);
int vita_whdload_prepare_launch(const char *game_name);
const char *vita_whdload_root(void);

#ifdef __cplusplus
}
#endif

#endif
#endif
