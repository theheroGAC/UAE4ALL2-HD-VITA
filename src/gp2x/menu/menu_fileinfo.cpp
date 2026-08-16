#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "sysconfig.h"
#include "sysdeps.h"
#include "config.h"
#include "menu.h"
#include<SDL.h>
#include <sys/stat.h>
#include <unistd.h>
#if defined(__PSP2__) // NOT __SWITCH__
#include "psp2-dirent.h"
#else
#include<dirent.h>
#endif
#include "uae.h"
#include "options.h"
#include "sound.h"
#include "gp2x.h"

#include <SDL_image.h>

static const char *text_str_fileinfo_title=    "File info & Boxart Preview";
char* fileInfo_fileName;

static bool is_valid_png_file(const char *path) {
	FILE *f = fopen(path, "rb");
	if (!f) return false;
	unsigned char header[8];
	size_t read_bytes = fread(header, 1, 8, f);
	fseek(f, 0, SEEK_END);
	long sz = ftell(f);
	fclose(f);
	if (read_bytes < 8 || sz < 30) return false;
	return (header[0] == 0x89 && header[1] == 'P' && header[2] == 'N' && header[3] == 'G' &&
	        header[4] == 0x0D && header[5] == 0x0A && header[6] == 0x1A && header[7] == 0x0A);
}

static bool is_valid_jpg_file(const char *path) {
	FILE *f = fopen(path, "rb");
	if (!f) return false;
	unsigned char header[4];
	size_t read_bytes = fread(header, 1, 4, f);
	fseek(f, 0, SEEK_END);
	long sz = ftell(f);
	fclose(f);
	if (read_bytes < 4 || sz < 100) return false;
	return (header[0] == 0xFF && header[1] == 0xD8 && header[2] == 0xFF);
}

static void draw_fileinfoMenu(int c)
{
	int menuLine = 0;
	SDL_Rect r;
	extern SDL_Surface *text_screen;
	r.x=80-64; r.y=60; r.w=110+64+64; r.h=120;

	text_draw_background();
	text_draw_window(2,2,40,40,text_str_fileinfo_title);

	menuLine = 4;
	write_text(3, menuLine, "File info:");
	menuLine+=2;
	write_text(3, menuLine, "----------");
	menuLine+=2;

	// now wrap the filename if necessary (at 32)
	int i = 0;
	char line [40];
	const int LINELEN = 32;

	for (i = 0; i < strlen(fileInfo_fileName); i+=LINELEN)
	{
		strncpy(line, fileInfo_fileName + i, LINELEN);
		line[LINELEN] = '\0';
		write_text(3, menuLine, line);
		menuLine+=2;
	}

	// Try loading cover preview image
	char coverPath[512];
	char baseName[256];
	strcpy(baseName, fileInfo_fileName);
	char *dot = strrchr(baseName, '.');
	if (dot) *dot = '\0';

	snprintf(coverPath, sizeof(coverPath), "ux0:/data/uae4all/covers/%s.png", baseName);
	SDL_Surface *cover = NULL;
	if (is_valid_png_file(coverPath)) {
		cover = IMG_Load(coverPath);
	}
	if (!cover) {
		snprintf(coverPath, sizeof(coverPath), "ux0:/data/uae4all/covers/%s.jpg", baseName);
		if (is_valid_jpg_file(coverPath)) {
			cover = IMG_Load(coverPath);
		}
	}

	if (cover && cover->w > 0 && cover->h > 0) {
		SDL_Surface *formatted = SDL_DisplayFormat(cover);
		SDL_FreeSurface(cover);
		if (formatted) {
			int max_w = 140;
			int max_h = 160;
			int dst_w = formatted->w;
			int dst_h = formatted->h;
			if (dst_w > max_w || dst_h > max_h) {
				float scale_w = (float)max_w / (float)formatted->w;
				float scale_h = (float)max_h / (float)formatted->h;
				float scale = (scale_w < scale_h) ? scale_w : scale_h;
				dst_w = (int)(formatted->w * scale);
				dst_h = (int)(formatted->h * scale);
			}
			SDL_Surface *scaled = SDL_CreateRGBSurface(formatted->flags, dst_w, dst_h,
				formatted->format->BitsPerPixel, formatted->format->Rmask,
				formatted->format->Gmask, formatted->format->Bmask, formatted->format->Amask);
			if (scaled) {
				SDL_Rect src_r = { 0, 0, (Uint16)formatted->w, (Uint16)formatted->h };
				SDL_Rect dst_r = { 0, 0, (Uint16)dst_w, (Uint16)dst_h };
				SDL_SoftStretch(formatted, &src_r, scaled, &dst_r);

				SDL_Rect blit_r;
				blit_r.x = (Sint16)(text_screen->w - dst_w - 20);
				if (blit_r.x < text_screen->w / 2) blit_r.x = (Sint16)(text_screen->w / 2);
				blit_r.y = 30;
				blit_r.w = (Uint16)dst_w;
				blit_r.h = (Uint16)dst_h;
				SDL_BlitSurface(scaled, NULL, text_screen, &blit_r);
				SDL_FreeSurface(scaled);
			}
			SDL_FreeSurface(formatted);
		}
	}
	
	text_flip();
}

static int key_fileinfoMenu(int *c)
{
	int end=0;
	int left=0, right=0, up=0, down=0, hit0=0, hit1=0;
	SDL_Event event;

	while (SDL_PollEvent(&event) > 0)
	{
		if (event.type == SDL_QUIT)
			end=-1;
		else if (event.type == SDL_KEYUP )
			end=-1;
		else if (event.type == SDL_JOYBUTTONUP )
			end=-1;
	}
	return end;
}

static void raise_fileinfoMenu()
{
	int i;

	text_draw_background();
	text_flip();
#if !defined(__PSP2__) && !defined(__SWITCH__)
	for(i=0;i<10;i++)
	{
		text_draw_background();
		text_draw_window(80-64,(10-i)*24,160+64+64,220,text_str_fileinfo_title);
		text_flip();
	}
#endif
}

static void unraise_fileinfoMenu()
{
	int i;

#if !defined(__PSP2__) && !defined(__SWITCH__)
	for(i=9;i>=0;i--)
	{
		text_draw_background();
		text_draw_window(80-64,(10-i)*24,160+64+64,220,text_str_fileinfo_title);
		text_flip();
	}
#endif
	text_draw_background();
	text_flip();
}

int run_menuFileinfo(char* fileName)
{
	int end=0,c=0;

	fileInfo_fileName = fileName;

	while(!end)
	{
		draw_fileinfoMenu(c);
		end=key_fileinfoMenu(&c);
	}

	return end;
}
