#ifndef _AUTO_DISPLAY_H
#define _AUTO_DISPLAY_H

struct AutoDisplayLine {
    int plfleft;
    int nr_planes;
    int nr_sprites;
    int bplres;
};

struct AutoDisplayRect {
    int left;
    int width;
    int top;
    int height;
    int hires;
    int valid;
};

static inline void auto_display_hwindow(int left, int right, int max_window,
        int *window_left, int *window_right)
{
    if (left < 0)
        left = 0;
    if (right <= left || right > max_window) {
        if (right <= left) {
            left = 0;
            right = max_window;
        } else {
            right = max_window;
        }
    }
    if (left > max_window - 1)
        left = max_window - 1;
    if (right <= left)
        right = left + 1;
    *window_left = left;
    *window_right = right;
}

static inline void auto_display_window(int diw_hleft, int diw_hright, int diw_vfirst, int diw_vlast,
        int min_line, int max_line, int max_window, int max_lines, int hires,
        struct AutoDisplayRect *out)
{
    int left, right, top, bottom;

    auto_display_hwindow (diw_hleft, diw_hright, max_window, &left, &right);

    top = diw_vfirst;
    bottom = diw_vlast;
    if (top < min_line)
        top = min_line;
    if (bottom <= top)
        bottom = top + max_lines;
    if (bottom > max_line + 1)
        bottom = max_line + 1;
    if (bottom - top > max_lines)
        bottom = top + max_lines;
    if (bottom <= top)
        bottom = top + 1;

    out->left = left;
    out->width = right - left;
    out->top = top;
    out->height = bottom - top;
    out->hires = hires ? 1 : 0;
    out->valid = 1;
}

static inline void auto_display_scan(const struct AutoDisplayLine *lines, int count,
        int diw_hleft, int diw_hright, int diw_vfirst, int diw_vlast,
        int min_line, int max_line, int max_window, int max_lines,
        struct AutoDisplayRect *out)
{
    int first = -1, last = -1;
    int active = 0, hires = 0, i;

    out->left = 0;
    out->width = 0;
    out->top = 0;
    out->height = 0;
    out->hires = 0;
    out->valid = 0;

    for (i = min_line; i < count; i++) {
        const struct AutoDisplayLine *ln = &lines[i];
        if ((ln->plfleft >= 0 && ln->nr_planes > 0) || ln->nr_sprites > 0) {
            if (first < 0)
                first = i;
            last = i;
            active++;
            if (ln->bplres >= 1)
                hires++;
        }
    }

    if (active == 0)
        return;

    if (diw_vfirst >= min_line && diw_vlast > diw_vfirst) {
        if (first < 0 || diw_vfirst < first)
            first = diw_vfirst;
        if (last < 0 || diw_vlast - 1 > last)
            last = diw_vlast - 1;
    }
    if (first < min_line)
        first = min_line;
    if (last > max_line)
        last = max_line;
    if (last < first)
        return;
    if (last - first + 1 > max_lines)
        last = first + max_lines - 1;
    if (last < first)
        return;

    auto_display_hwindow (diw_hleft, diw_hright, max_window, &out->left, &out->width);
    out->width -= out->left;
    out->top = first;
    out->height = last - first + 1;
    out->hires = (hires * 2 > active) ? 1 : 0;
    out->valid = 1;
}

static inline void auto_display_clamp(struct AutoDisplayRect *r, int min_line, int max_line, int max_lines)
{
    if (!r->valid)
        return;
    if (r->top < min_line)
        r->top = min_line;
    if (r->top > max_line)
        r->top = max_line;
    if (r->top + r->height > max_line + 1)
        r->height = max_line + 1 - r->top;
    if (r->height > max_lines)
        r->height = max_lines;
    if (r->height < 1)
        r->height = 1;
}

static inline void auto_display_commit(struct AutoDisplayRect *dst, const struct AutoDisplayRect *src,
        int min_line, int max_line, int max_lines, int shrink_frames, int *timer)
{
    int l, r, t, b;

    if (!src->valid) {
        auto_display_clamp (dst, min_line, max_line, max_lines);
        return;
    }

    if (!dst->valid) {
        *dst = *src;
        *timer = 0;
    } else if (src->left < dst->left || src->left + src->width > dst->left + dst->width ||
        src->top < dst->top || src->top + src->height > dst->top + dst->height) {
        l = src->left < dst->left ? src->left : dst->left;
        r = src->left + src->width > dst->left + dst->width ? src->left + src->width : dst->left + dst->width;
        t = src->top < dst->top ? src->top : dst->top;
        b = src->top + src->height > dst->top + dst->height ? src->top + src->height : dst->top + dst->height;
        dst->left = l;
        dst->width = r - l;
        dst->top = t;
        dst->height = b - t;
        dst->hires = src->hires;
        *timer = 0;
    } else if (src->left == dst->left && src->width == dst->width &&
        src->top == dst->top && src->height == dst->height) {
        dst->hires = src->hires;
        *timer = 0;
    } else {
        (*timer)++;
        if (*timer >= shrink_frames) {
            *dst = *src;
            *timer = 0;
        }
    }

    auto_display_clamp (dst, min_line, max_line, max_lines);
}

static inline void auto_display_fit_surface(struct AutoDisplayRect *r, int surface_w, int surface_h,
        int max_window)
{
    int max_units, px;

    if (!r->valid)
        return;

    px = r->hires ? 2 : 1;
    max_units = surface_w / px;
    if (max_units < 1)
        max_units = 1;
    if (r->width > max_units)
        r->width = max_units;
    if (r->left + r->width > max_units)
        r->left = max_units - r->width;
    if (r->left < 0)
        r->left = 0;
    if (r->width > max_window)
        r->width = max_window;
    if (r->width < 1)
        r->width = 1;
    if (r->top < 0)
        r->top = 0;
    if (r->height > surface_h)
        r->height = surface_h;
    if (r->height < 1)
        r->height = 1;
    if (r->hires && r->width * 2 > surface_w)
        r->width = surface_w / 2;
    if (r->width < 1)
        r->width = 1;
}

static inline void auto_display_source(const struct AutoDisplayRect *r, int max_window,
        int *visible_left, int *visible_right, int *adjust_bytes, int *start_line)
{
    int left = r->left;
    int width = r->width;

    if (left < 0)
        left = 0;
    if (left > max_window)
        left = max_window;
    if (width < 1)
        width = 1;
    if (width > max_window)
        width = max_window;

    *visible_left = left;
    *visible_right = left + width;
    *adjust_bytes = 2 * left;
    *start_line = r->top;
}

static inline void auto_display_geometry(const struct AutoDisplayRect *r, int screen_w, int screen_h,
        int surface_w, int surface_h, float *x, float *y, float *sw, float *sh)
{
    int px = r->hires ? 2 : 1;
    int cw = r->width;
    int ch = r->height;
    float sx, sy, scale, dw, dh, kx;

    if (cw < 1)
        cw = 1;
    if (ch < 1)
        ch = 1;

    sx = (float)screen_w / (float)cw;
    sy = (float)screen_h / (float)ch;
    scale = sx < sy ? sx : sy;

    dw = (float)cw * scale;
    dh = (float)ch * scale;
    kx = scale / (float)px;

    *x = ((float)screen_w - dw) * 0.5f;
    *y = ((float)screen_h - dh) * 0.5f;
    *sw = (float)surface_w * kx;
    *sh = (float)surface_h * scale;
}

#endif
