/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 sulfurLabs
 *
 * PROJECT: s4
 * FILE: render.c
 *
 */

#include "render.h"
#include "../compositor/comp.h"
#include "../config/cfg.h"
#include "../win/win.h"
#include "../../../libs/libfont/libfont.h"

#define ROW_MAX 4096
static unsigned int row_buf[ROW_MAX];

#include "icons.h"

static int slen(const char *s) { int n = 0; while (s[n]) n++; return n; }

static unsigned int stripe(int y, int focused)
{
	#if DARK_MODE == 1
	    return focused ? DT_TITLE_ACT : DT_TITLE_INA;
	#else
	    if (focused) return (y & 1) ? WIN_BLACK : WIN_WHITE;

	    return (y & 1) ? WIN_UNFOCUSED_BG : WIN_FOCUSED_BG;
	#endif
}

static void buf_char(int bx, char c, unsigned int fg, unsigned int bg, int frow)
{
    uint16_t bits = font_glyph(FONT8X12_BOLD, (unsigned char)c & 0x7Fu, frow);

    for (int col = 0; col < DT_FW; col++)
    {
        int dx = bx + col;
        if (dx >= 0 && dx < ROW_MAX) row_buf[dx] = (bits & (1u << col)) ? fg : bg;
    }
}

/*writes string into row_buf  */
static void buf_str_clamped(
    int bx, int maxw, const char *s,
    unsigned int fg, unsigned int bg, int frow
) {
    int px = bx;
    while (*s)
    {
        if (px - bx >= maxw) break;
        buf_char(px, *s++, fg, bg, frow);
        px += DT_FW;
    }
}

static void flush_row(int x, int y, int w) {
    comp_put_row(x, y, row_buf, w);
}

static void hline_comp(int x, int y, int w, unsigned int c)
{
    for (int dx = 0; dx < w && dx < ROW_MAX; dx++) row_buf[dx] = c;
    comp_put_row(x, y, row_buf, w);
}
// to the backbuffer
static void side_borders_comp(int wx, int y, int ww)
{
    comp_set(wx,          y, WIN_BLACK);
    comp_set(wx + 1,      y, WIN_BLACK);
    comp_set(wx + ww - 2, y, WIN_BLACK);
    comp_set(wx + ww - 1, y, WIN_BLACK);
}

static void blit_win_buf_scaled(
	int cx,
	int cy,
	int cw,
	int ch,
    const unsigned int *src,
    int sw,
    int sh
) {
    if (
    	cw <= 0 ||
     	ch <= 0 ||
      	sw <= 0 ||
       	sh <= 0
    ) return;

    for (int dy = 0; dy < ch; dy++)
    {
        int sy = dy * sh / ch;
        if (sy >= sh) sy = sh - 1;
        const unsigned int *srow = src + sy * sw;

        int len = cw < ROW_MAX ? cw : ROW_MAX;
        for (int dx = 0; dx < len; dx++)
        {
            int sx = dx * sw / cw;
            if (sx >= sw) sx = sw - 1;
            row_buf[dx] = srow[sx];
        }
        comp_put_pixels(cx, cy + dy, len, 1, row_buf);
    }
}

static void blit_win_buf(
	dt_win_t *w,
	unsigned int style,
	int has_title,
	int wx,
	int wy
) {
    if (
    	!w->buf ||
     	w->buf_w <= 0 ||
      	w->buf_h <= 0
    ) return;

    int cx;
    int cy;
    int cw;
    int ch;

    if (style & DT_POPUP)
    {
        cx = wx + 1;
        cy = wy + 1;
        cw = w->w - 2;
        ch = w->h - 2;
    } else if (has_title) {
        cx = wx + DT_BORDER;
        cy = wy + DT_TITLE_H + 1;
        cw = w->w - DT_BORDER * 2;
        ch = w->h - DT_TITLE_H - 1 - DT_BORDER;
    } else {
        cx = wx + DT_BORDER;
        cy = wy + DT_BORDER;
        cw = w->w - DT_BORDER * 2;
        ch = w->h - DT_BORDER * 2;
    }

    if (w->buf_w == cw && w->buf_h == ch) comp_put_pixels(cx, cy, w->buf_w, w->buf_h, w->buf);
    else blit_win_buf_scaled(cx, cy, cw, ch, w->buf, w->buf_w, w->buf_h);
}

//pubs
void render_win(dt_win_t *w, int mx, int my)
{
    int wx = w->x, wy = w->y;
    int ww = w->w, wh = w->h;

    int focused = w->focused;

    unsigned int style = w->style;

    int h = comp_h();
    int cw = comp_w();
    (void)cw;

    int has_title = !(style & DT_NOTITLE);

    int hover_close = (
    	mx >= wx + DT_CLOSE_X && mx < wx + DT_CLOSE_X + DT_CLOSE_SZ &&
     	my >= wy + DT_CLOSE_Y && my < wy + DT_CLOSE_Y + DT_CLOSE_SZ
    );

    #if !ENABLE_TILING
        int hover_max = (
        	mx >= wx + DT_MAX_X && mx < wx + DT_MAX_X + DT_MAX_SZ &&
         	my >= wy + DT_MAX_Y && my < wy + DT_MAX_Y + DT_MAX_SZ
        );
    #endif

    blit_win_buf(w, style, has_title, wx, wy);

    if (style & DT_POPUP)
    {
    	// content will be untouched
        hline_comp(wx, wy,          ww, WIN_BLACK);
        hline_comp(wx, wy + wh - 1, ww, WIN_BLACK);
        for (int dy = 1; dy < wh - 1; dy++) {
            int ay = wy + dy;
            if (ay < 0 || ay >= h) continue;
            comp_set(wx,          ay, WIN_BLACK);
            comp_set(wx + ww - 1, ay, WIN_BLACK);
        }
        return;
    }

    //int has_title = !(style & DT_NOTITLE);
    int tw = slen(w->title) * DT_FW;

    #if ENABLE_TILING
        int deco_end = DT_CLOSE_X + DT_CLOSE_SZ;
    #else
        int deco_end = DT_MAX_X + DT_MAX_SZ;
    #endif

    int title_area = ww - deco_end - 4 - 4;
    if (tw > title_area) tw = title_area;

    int tx = (ww - tw) / 2;
    if (tx < deco_end + 2) tx = deco_end + 2;

    int sl = tx - 4;
    int sr = tx + tw + 4;

    unsigned int title_bg = has_title
        ? (focused ? DT_TITLE_ACT : DT_TITLE_INA)
        : WIN_FACE
    ;

    for (int dy = 0; dy < wh; dy++)
    {
        int ay = wy + dy;
        if (ay < 0 || ay >= h) continue;
        if (dy < 2 || dy >= wh - 2) {
            hline_comp(wx, ay, ww, WIN_BLACK);
            continue;
        }
        if (has_title && dy >= 2 && dy < DT_TITLE_H)
        {
        	int frow = dy - DT_TITLE_PB; // title text

            // fill titlebar bg
            for (int dx = 2; dx < ww - 2 && dx < ROW_MAX; dx++)
            {
                row_buf[dx] = (dx < sl || dx >= sr)
                    ? stripe(ay, focused)
                    : title_bg;
            }
            // left, right border pixels
            row_buf[0] = row_buf[1] = WIN_BLACK;

            if (ww - 2 < ROW_MAX) row_buf[ww - 2] = row_buf[ww - 1] = WIN_BLACK;

            if (frow >= 0 && frow < DT_FH) buf_str_clamped(tx, tw + DT_FW, w->title, DT_TITLE_TXT, title_bg, frow);

            // close button
            {
                int bx  = DT_CLOSE_X;
                int bry = dy - DT_CLOSE_Y;
                if (bry >= 0 && bry < DT_CLOSE_SZ)
                {
                    for (
                    	int dx = 0; dx < DT_CLOSE_SZ && (bx + dx) < ROW_MAX; dx++
                    ) {
                        unsigned int icon_px = dt_icon_close_px[bry * DT_ICON_W + dx];
                        unsigned int out;

                        // hover mechanism for icons
                        if (!(icon_px >> 24)) out = title_bg; // transparent
                        else if (icon_px == DT_ICON_FILL) out = hover_close ? DT_ICON_FILL_HOVER : DT_ICON_FILL;
                        else out = icon_px;

                        row_buf[bx + dx] = out;
                    }
                }
            }

            #if !ENABLE_TILING
                {
                    int mbx  = DT_MAX_X;
                    int mbry = dy - DT_MAX_Y;
                    if (mbry >= 0 && mbry < DT_MAX_SZ)
                    {
                        for (
                        	int dx = 0; dx < DT_MAX_SZ && (mbx + dx) < ROW_MAX; dx++
                        ) {
                            unsigned int icon_px = dt_icon_maximize_px[mbry * DT_ICON_W + dx];
                            unsigned int out;

                            if (!(icon_px >> 24)) out = title_bg; // transparent
                            else if (icon_px == DT_ICON_FILL) out = hover_max ? DT_ICON_FILL_HOVER : DT_ICON_FILL;
                            else out = icon_px; // border pixel, unaffected by hover

                            row_buf[mbx + dx] = out;
                        }
                    }
                }
            #endif

            flush_row(wx, ay, ww);
            continue;
        }

        // separator line after the titlebar
        if (has_title && dy == DT_TITLE_H)
        {
            hline_comp(wx, ay, ww, WIN_BLACK);
            continue;
        }

        // content + window border
        {
            int cs = has_title ? (DT_TITLE_H + 1) : 2;
            if (dy >= cs && dy < wh - 2) side_borders_comp(wx, ay, ww);
        }
    }
}

static int rects_intersect(int ax,int ay,int aw,int ah,int bx,int by,int bw,int bh)
{
    if (
    	aw <= 0 ||
     	ah <= 0 ||
      	bw <= 0 ||
       	bh <= 0
    ) return 0;

    return !(
    	ax + aw <= bx ||
     	bx + bw <= ax ||
      	ay + ah <= by ||
       	by + bh <= ay
    );
}

void render_all(int mx, int my)
{
    int order[DT_WIN_MAX];
    int count = 0;
    int i;
    int j;

    for (i = 0; i < DT_WIN_MAX; i++)
    {
        if (win_get(i)) order[count++] = i;
    }
    for (i = 0; i < count - 1; i++)
    {
        for (j = 0; j < count - 1 - i; j++)
        {
            dt_win_t *a = win_get(order[j]);
            dt_win_t *b = win_get(order[j + 1]);

            if (a && b && a->z > b->z)
            {
                int tmp = order[j];
                order[j] = order[j + 1];
                order[j + 1] = tmp;
            }
        }
    }

    for (i = 0; i < count; i++)
    {
        dt_win_t *wn = win_get(order[i]);
        if (wn) render_win(wn, mx, my);
    }
}

void render_all_in_rect(int rx, int ry, int rw, int rh, int mx, int my)
{
    int order[DT_WIN_MAX];
    int count = 0;
    int i;

    for (i = 0; i < DT_WIN_MAX; i++)
    {
    	if (win_get(i)) order[count++] = i;
    }

    for (i = 0; i < count - 1; i++)
    {
        for (int j = 0; j < count - 1 - i; j++)
        {
            dt_win_t *a = win_get(order[j]);
            dt_win_t *b = win_get(order[j + 1]);

            if (a && b && a->z > b->z)
            {
            	int t = order[j];

             	order[j] = order[j + 1];
              	order[j+1] = t;
            }
        }
    }

    for (i = 0; i < count; i++)
    {
        dt_win_t *wn = win_get(order[i]);

        if (!wn) continue;
        if (!rects_intersect(wn->x, wn->y, wn->w, wn->h, rx, ry, rw, rh)) continue;

        render_win(wn, mx, my);
    }
}
