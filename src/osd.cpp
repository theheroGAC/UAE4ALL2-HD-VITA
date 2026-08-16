#include "osd.h"
#include <SDL.h>

static Uint32 s_osd_start_ticks = 0;
static bool s_osd_visible = false;
static int s_osd_disk = 1;
static bool s_osd_error = false;

void OSD_TriggerDiskSwap(int disk_number, bool is_error)
{
    s_osd_disk = disk_number;
    s_osd_error = is_error;
    s_osd_start_ticks = SDL_GetTicks();
    s_osd_visible = true;
}

void OSD_Render(SDL_Surface *surface)
{
    if (!s_osd_visible || !surface) return;

    Uint32 elapsed = SDL_GetTicks() - s_osd_start_ticks;
    if (elapsed >= 2500) {
        s_osd_visible = false;
        return;
    }

                                                      
                                                         
                                                                              
    int target_x = (900 * surface->w) / 960;
    int target_y = (20 * surface->h) / 544;
    int target_w = (40 * surface->w) / 960;
    int target_h = (40 * surface->h) / 544;

    if (target_w < 6) target_w = 6;
    if (target_h < 6) target_h = 6;

    SDL_Rect dst_rect;
    dst_rect.x = target_x;
    dst_rect.y = target_y;
    dst_rect.w = target_w;
    dst_rect.h = target_h;

              
                                             
                                                   
                                             
    Uint8 r = 0, g = 255, b = 0, a = 200;
    if (s_osd_error) {
        r = 255; g = 0; b = 0;
    } else if (s_osd_disk == 2) {
        r = 255; g = 165; b = 0;
    }

    SDL_Surface *osd_surface = SDL_CreateRGBSurface(
        SDL_SWSURFACE,
        target_w,
        target_h,
        32,
        0x000000FF,
        0x0000FF00,
        0x00FF0000,
        0xFF000000
    );

    if (osd_surface) {
        Uint32 color = SDL_MapRGBA(osd_surface->format, r, g, b, a);
        SDL_FillRect(osd_surface, NULL, color);
        SDL_SetAlpha(osd_surface, SDL_SRCALPHA, a);
        SDL_BlitSurface(osd_surface, NULL, surface, &dst_rect);
        SDL_FreeSurface(osd_surface);
    } else {
        Uint32 fallback_color = SDL_MapRGB(surface->format, r, g, b);
        SDL_FillRect(surface, &dst_rect, fallback_color);
    }
}
