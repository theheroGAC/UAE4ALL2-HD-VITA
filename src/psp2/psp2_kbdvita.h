#ifndef __KEYBOARD_VITA_H__
#define __KEYBOARD_VITA_H__

#ifdef __cplusplus
extern "C" {
#endif

char *kbdvita_get(char *title, const char *initial_text, int maxLen, int multiline);
int kbdvita_start(char *title, const char *initial_text, int maxLen, int multiline);
int kbdvita_update(void);
const char *kbdvita_result(void);
void kbdvita_cancel(void);

#ifdef __cplusplus
}
#endif

#endif
