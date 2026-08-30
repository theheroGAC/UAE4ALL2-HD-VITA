#include <string.h>
#include <stdbool.h>
#include <psp2/apputil.h>
#include <psp2/display.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/ime_dialog.h>
#include <psp2/message_dialog.h>
#include <SDL.h>
#include "psp2_kbdvita.h"

extern SDL_Surface *prSDLScreen;

#define IME_DIALOG_RESULT_NONE 0
#define IME_DIALOG_RESULT_RUNNING 1
#define IME_DIALOG_RESULT_FINISHED 2
#define IME_DIALOG_RESULT_CANCELED 3

static int ime_dialog_running = 0;
static int ime_dialog_option = 0;
static int ime_init_apputils = 0;

static uint16_t ime_title_utf16[SCE_IME_DIALOG_MAX_TITLE_LENGTH];
static uint16_t ime_initial_text_utf16[SCE_IME_DIALOG_MAX_TEXT_LENGTH];
static uint16_t ime_input_text_utf16[SCE_IME_DIALOG_MAX_TEXT_LENGTH + 1];
static uint8_t ime_input_text_utf8[SCE_IME_DIALOG_MAX_TEXT_LENGTH + 1];
static int ime_async_result = 0;

void utf16_to_utf8(uint16_t *src, uint8_t *dst) {
	int i;
	for (i = 0; src[i]; i++) {
		if ((src[i] & 0xFF80) == 0) {
			*(dst++) = src[i] & 0xFF;
		} else if((src[i] & 0xF800) == 0) {
			*(dst++) = ((src[i] >> 6) & 0xFF) | 0xC0;
			*(dst++) = (src[i] & 0x3F) | 0x80;
		} else if((src[i] & 0xFC00) == 0xD800 && (src[i + 1] & 0xFC00) == 0xDC00) {
			*(dst++) = (((src[i] + 64) >> 8) & 0x3) | 0xF0;
			*(dst++) = (((src[i] >> 2) + 16) & 0x3F) | 0x80;
			*(dst++) = ((src[i] >> 4) & 0x30) | 0x80 | ((src[i + 1] << 2) & 0xF);
			*(dst++) = (src[i + 1] & 0x3F) | 0x80;
			i += 1;
		} else {
			*(dst++) = ((src[i] >> 12) & 0xF) | 0xE0;
			*(dst++) = ((src[i] >> 6) & 0x3F) | 0x80;
			*(dst++) = (src[i] & 0x3F) | 0x80;
		}
	}

	*dst = '\0';
}

void utf8_to_utf16(uint8_t *src, uint16_t *dst) {
	int i;
	for (i = 0; src[i];) {
		if ((src[i] & 0xE0) == 0xE0) {
			*(dst++) = ((src[i] & 0x0F) << 12) | ((src[i + 1] & 0x3F) << 6) | (src[i + 2] & 0x3F);
			i += 3;
		} else if ((src[i] & 0xC0) == 0xC0) {
			*(dst++) = ((src[i] & 0x1F) << 6) | (src[i + 1] & 0x3F);
			i += 2;
		} else {
			*(dst++) = src[i];
			i += 1;
		}
	}

	*dst = '\0';
}

int initImeDialog(char *title, const char *initial_text, int max_text_length, int type, int option) {
	if (ime_dialog_running)
		return -1;

	memset(ime_title_utf16, 0, sizeof(ime_title_utf16));
	memset(ime_initial_text_utf16, 0, sizeof(ime_initial_text_utf16));
	utf8_to_utf16((uint8_t *)title, ime_title_utf16);
	utf8_to_utf16((uint8_t *)initial_text, ime_initial_text_utf16);

	memset(ime_input_text_utf16, 0, sizeof(ime_input_text_utf16));
	memset(ime_input_text_utf8, 0, sizeof(ime_input_text_utf8));

	SceImeDialogParam param;
	sceImeDialogParamInit(&param);

	param.supportedLanguages = 0x0001FFFF;
	param.languagesForced = SCE_TRUE;
	param.type = type;
	param.option = option;
	param.dialogMode = SCE_IME_DIALOG_DIALOG_MODE_WITH_CANCEL;
	param.title = ime_title_utf16;
	param.maxTextLength = max_text_length;
	param.initialText = ime_initial_text_utf16;
	param.inputTextBuffer = ime_input_text_utf16;

	int res = sceImeDialogInit(&param);
	if (res >= 0) {
		ime_dialog_running = 1;
		ime_dialog_option = option;
	}

	return res;
}

int isImeDialogRunning() {
	return ime_dialog_running;
}

uint16_t *getImeDialogInputTextUTF16() {
	return ime_input_text_utf16;
}

uint8_t *getImeDialogInputTextUTF8() {
	return ime_input_text_utf8;
}

int updateImeDialog() {
	if (!ime_dialog_running)
		return IME_DIALOG_RESULT_NONE;

	SceCommonDialogStatus status = sceImeDialogGetStatus();
	if (status == SCE_COMMON_DIALOG_STATUS_FINISHED) {
		SceImeDialogResult result;
		memset(&result, 0, sizeof(SceImeDialogResult));
		sceImeDialogGetResult(&result);

		if ((ime_dialog_option == SCE_IME_OPTION_MULTILINE && result.button == SCE_IME_DIALOG_BUTTON_CLOSE) ||
			(ime_dialog_option != SCE_IME_OPTION_MULTILINE && (result.button == SCE_IME_DIALOG_BUTTON_ENTER || result.button == SCE_IME_DIALOG_BUTTON_CLOSE))) {
			utf16_to_utf8(ime_input_text_utf16, ime_input_text_utf8);
			status = IME_DIALOG_RESULT_FINISHED;
		} else {
			status = IME_DIALOG_RESULT_CANCELED;
		}

		sceImeDialogTerm();
		ime_dialog_running = 0;
		return status;
	} else if (status == SCE_COMMON_DIALOG_STATUS_RUNNING) {
		return IME_DIALOG_RESULT_RUNNING;
	}

	return IME_DIALOG_RESULT_NONE;
}

int kbdvita_start(char *title, const char *initial_text, int maxLen, int multiline) {
	if (ime_init_apputils == 0) {
		sceAppUtilInit(&(SceAppUtilInitParam){}, &(SceAppUtilBootParam){});
		sceCommonDialogSetConfigParam(&(SceCommonDialogConfigParam){});
		ime_init_apputils = 1;
	}
	ime_async_result = 0;
	return initImeDialog(title, initial_text ? initial_text : "", maxLen, SCE_IME_TYPE_BASIC_LATIN, multiline ? SCE_IME_OPTION_MULTILINE : 0);
}

int kbdvita_update(void) {
	if (!ime_dialog_running)
		return ime_async_result;
	int result = updateImeDialog();
	if (result == IME_DIALOG_RESULT_FINISHED || result == IME_DIALOG_RESULT_CANCELED)
		ime_async_result = result;
	return result;
}

const char *kbdvita_result(void) {
	return (ime_async_result == IME_DIALOG_RESULT_FINISHED) ? (const char *)ime_input_text_utf8 : NULL;
}

void kbdvita_cancel(void) {
	if (ime_dialog_running) {
		sceImeDialogTerm();
		ime_dialog_running = 0;
	}
	ime_async_result = IME_DIALOG_RESULT_CANCELED;
}

char *kbdvita_get(char *title, const char *initial_text, int maxLen, int multiline) {
	char *name = NULL;

	if (ime_init_apputils == 0) {
		sceAppUtilInit(&(SceAppUtilInitParam){}, &(SceAppUtilBootParam){});
		sceCommonDialogSetConfigParam(&(SceCommonDialogConfigParam){});
		ime_init_apputils = 1;
	}
	int init_result;
	if (multiline)
	  init_result = initImeDialog(title, initial_text ? initial_text : "", maxLen, SCE_IME_TYPE_BASIC_LATIN, SCE_IME_OPTION_MULTILINE);
	else
	  init_result = initImeDialog(title, initial_text ? initial_text : "", maxLen, SCE_IME_TYPE_BASIC_LATIN, 0);
	if (init_result < 0)
		return NULL;

	bool done = false;
	while (!done) {
		int ime_result = updateImeDialog();
		if (ime_result == IME_DIALOG_RESULT_FINISHED) {
			name = (char *)getImeDialogInputTextUTF8();
			done = true;
		} else if (ime_result == IME_DIALOG_RESULT_CANCELED) {
			done = true;
		}

		if (prSDLScreen)
			SDL_Flip(prSDLScreen);
		else
			sceDisplayWaitVblankStart();
		SDL_Delay(16);
	}

	return name;
}
