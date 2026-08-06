/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 sulfurLabs
 *
 * PROJECT: s4
 * FILE: desktop.c
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
//#include <sys/ioctl.h>

#include "config/cfg.h"
#include "compositor/comp.h"
#include "bg/bg.h"
#include "win/win.h"
#include "render/render.h"
#include "cursor/cursor.h"
#include "input/input.h"
#include "taskbar/taskbar.h"
#include "shm/shm_host.h"
#include "cmd/cmd.h"

#include <sys/fb.h>
#include <sys/input.h>

#define DT_WBUF_PREFIX "/tmp/dt/wbuf_"
#define DT_DIRTY_PFX "/tmp/dt/dirty_"

extern drag_info_t g_input_drag_prev;
static int g_dirty_x0;
static int g_dirty_y0;
static int g_dirty_x1;
static int g_dirty_y1;

static int _slen(const char *s)
{
	int n = 0;
	while (s[n]) n++;
	return n;
}

void _itoa(int v, char *out)
{
    char tmp[16];
    int i = 0;
    int neg = (v < 0);
    int j = 0;

    if (v == 0)
    {
    	out[0]='0';
     	out[1]='\0';
     	return;
    }

    if (neg) v = -v;
    while (v)
    {
    	tmp[i++] = '0' + v % 10;
        v /= 10;
    }
    if (neg) tmp[i++] = '-';
    while (i > 0) {
        out[j++] = tmp[--i];
    }
    out[j] = '\0';
}

static void dirty_begin(void)
{
    g_dirty_x0 = 0x7FFFFFFF;
    g_dirty_y0 = 0x7FFFFFFF;
    g_dirty_x1 = -1;
    g_dirty_y1 = -1;
}

static void dirty_mark(int x, int y, int w, int h)
{
    if (w <= 0 || h <= 0) return;
    if (x < g_dirty_x0) g_dirty_x0 = x;
    if (y < g_dirty_y0) g_dirty_y0 = y;
    if (x + w > g_dirty_x1) g_dirty_x1 = x + w;
    if (y + h > g_dirty_y1) g_dirty_y1 = y + h;
}

static void _build_dirty_path(char *out, pid_t pid)
{
    int i = 0;
    int ti = 0;
    char tmp[12];
    unsigned int p = (unsigned int)pid;
    const char *pfx = DT_DIRTY_PFX;

    while (*pfx) out[i++] = *pfx++;
    if (!p)
    {
    	tmp[ti++] = '0';
    } else
    {
    	while (p)
     	{
      		tmp[ti++] = '0' + p % 10;
        	p /= 10;
      	}
    }

    while (ti > 0) out[i++] = tmp[--ti];
    out[i] = '\0';
}

static void bg_draw_rect_dirty(int x, int y, int w, int h)
{
    bg_draw_rect(x, y, w, h);
    dirty_mark(x, y, w, h);
}

static void write_cursor_pos(int x, int y)
{
    char buf[32];
    char xs[12];
    char ys[12];
    int i = 0;
    int j = 0;
    int fd = open(DT_CURSOR, O_WRONLY | O_CREAT);

    _itoa(x, xs);
    _itoa(y, ys);

    while (xs[j]) buf[i++] = xs[j++];
    buf[i++] = ' ';
    j = 0;

    while (ys[j]) buf[i++] = ys[j++];

    buf[i++] = '\n';
    buf[i] = '\0';

    if (fd >= 0) { write(fd, buf, (unsigned)_slen(buf)); close(fd); }
}

static void current_content(
    dt_win_t *w,
    int *cx, int *cy, int *cw, int *ch
) {
    unsigned int s = w->style;
    if (s & DT_POPUP) {
        *cx = w->x + 1;
        *cy = w->y + 1;
        *cw = w->w - 2;
        *ch = w->h - 2;
    } else if (s & DT_NOTITLE) {
        *cx = w->x + DT_BORDER;
        *cy = w->y + DT_BORDER;
        *cw = w->w - DT_BORDER * 2;
        *ch = w->h - DT_BORDER * 2;
    } else {
        *cx = w->x + DT_BORDER;
        *cy = w->y + DT_TITLE_H + 1;
        *cw = w->w - DT_BORDER * 2;
        *ch = w->h - DT_TITLE_H - 1 - DT_BORDER;
    }
}

static void _old_full_rect(
    dt_win_t *w, int hcx, int hcy, int hcw, int hch,
    int *ox, int *oy, int *ow, int *oh
){
    unsigned int s = w->style;
    if (s & DT_POPUP) {
        *ox = hcx - 1;
        *oy = hcy - 1;
        *ow = hcw + 2;
        *oh = hch + 2;
    } else if (s & DT_NOTITLE) {
        *ox = hcx - DT_BORDER;
        *oy = hcy - DT_BORDER;
        *ow = hcw + DT_BORDER * 2;
        *oh = hch + DT_BORDER * 2;
    } else {
        *ox = hcx - DT_BORDER;
        *oy = hcy - DT_TITLE_H - 1;
        *ow = hcw + DT_BORDER * 2;
        *oh = hch + DT_TITLE_H + 1 + DT_BORDER;
    }
}

static void sync_home_to_current(void)
{
    int i;
    for (i = 0; i < DT_WIN_MAX; i++)
    {
        dt_win_t *w = win_get(i);
        if (!w) continue;

        int cx, cy, cw, ch;
        int ox, oy, ow, oh;
        int nox, noy, now_, noh;

        current_content(w, &cx, &cy, &cw, &ch);

        // if window hasn't moved or resized, nothing to do
        if (cx == w->home_cx && cy == w->home_cy && cw == w->home_cw && ch == w->home_ch) continue;

        _old_full_rect(
            w, w->home_cx, w->home_cy, w->home_cw, w->home_ch,
            &ox, &oy, &ow, &oh
        );
        _old_full_rect(w, cx, cy, cw, ch, &nox, &noy, &now_, &noh);
        {
            int ix0 = ox > nox ? ox : nox;
            int iy0 = oy > noy ? oy : noy;
            int ix1 = (ox+ow)<(nox+now_) ? (ox+ow) : (nox+now_);
            int iy1 = (oy+oh)<(noy+noh) ? (oy+oh) : (noy+noh);

            if (ix0 >= ix1 || iy0 >= iy1)
            {
                bg_draw_rect_dirty(ox, oy, ow, oh);
            } else
            {
                if (oy < iy0) bg_draw_rect_dirty(ox, oy, ow, iy0-oy);
                if (oy+oh > iy1) bg_draw_rect_dirty(ox, iy1, ow, (oy+oh)-iy1);

                if (iy0 < iy1)
                {
                    if (ox < ix0) bg_draw_rect_dirty(ox, iy0, ix0-ox, iy1-iy0);
                    if (ox+ow > ix1) bg_draw_rect_dirty(ix1, iy0, (ox+ow)-ix1, iy1-iy0);
                }
            }
            dirty_mark(nox, noy, now_, noh);
        }

        w->home_cx = cx;
        w->home_cy = cy;
        w->home_cw = cw;
        w->home_ch = ch;
    }
}

static int poll_dirty_windows(void)
{
    char path[64];
    int any = 0;

    for (int i = 0; i < DT_WIN_MAX; i++)
    {
    	char flag[2];
        dt_win_t *wn = win_get(i);
        if (!wn || !wn->buf) continue;

        _build_dirty_path(path, wn->pid);

        int fd = open(path, O_RDONLY);
        if (fd < 0) continue;

        int n = (int)read(fd, flag, 1);
        close(fd);
        if (n <= 0 || flag[0] != '1') continue;

        fd = open(path, O_WRONLY | O_CREAT);
        if (fd >= 0)
        {
        	write(fd, "0", 1);
         	close(fd);
        }

        dirty_mark(wn->x, wn->y, wn->w, wn->h);
        any = 1;
    }
    return any;
}

static void clear_closed_windows(void)
{
    if (!g_input_drag_prev.valid) return;
    bg_draw_rect_dirty(
        g_input_drag_prev.wx,
        g_input_drag_prev.wy,
        g_input_drag_prev.ww,
        g_input_drag_prev.wh
    );
    g_input_drag_prev.valid = 0;
}

static void clear_cmd_rects(cmd_result_t *cr)
{
    int i;
    for (i = 0; i < cr->count; i++)
    {
        drag_info_t *d = &cr->rects[i];
        if (!d->valid) continue;

        bg_draw_rect_dirty(d->wx, d->wy, d->ww, d->wh);

        int idx = win_find_pid(d->pid);
        dt_win_t *wn = win_get(idx);
        if (wn) dirty_mark(wn->x, wn->y, wn->w, wn->h);
    }
}

static void render_band(input_state_t *is)
{
	/*
	 * RUBBERBAND FIX
		* by @offihito
	 */
	if (!is->sel_active) return;

	// compute actual rect (supports all drag directions)
	int x0 = is->sel_x0 < is->sel_x1 ? is->sel_x0 : is->sel_x1;
	int y0 = is->sel_y0 < is->sel_y1 ? is->sel_y0 : is->sel_y1;
	int x1 = is->sel_x0 > is->sel_x1 ? is->sel_x0 : is->sel_x1;
	int y1 = is->sel_y0 > is->sel_y1 ? is->sel_y0 : is->sel_y1;
	int w = x1 - x0;
	int h = y1 - y0;

	if (w < 2 || h < 2) return;

	dirty_mark(x0, y0, w + 1, h + 1);

	int sw = comp_w();
	int sh = comp_h();

	for (int ry = y0 + 1; ry < y1 && ry < sh; ry++)
	{
		for (int rx = x0 + 1; rx < x1 && rx < sw; rx++)
		{
			unsigned int bg = comp_get(rx, ry);
			unsigned int br = (bg >> 16) & 0xFF;
			unsigned int bg2 = (bg >> 8) & 0xFF;
			unsigned int bb = bg & 0xFF;

			br = (br * 4 + 128) / 5;
			bg2 = (bg2 * 4 + 128) / 5;
			bb = (bb * 4 + 128) / 5;

			comp_set(rx, ry, 0xFF000000u | (br << 16) | (bg2 << 8) | bb);
		}
	}

	// top and bottom edges
	for (int rx = x0; rx <= x1 && rx < sw; rx++) {
		if (y0 >= 0 && y0 < sh) comp_set(rx, y0, BAND_BORDER);
		if (y1 >= 0 && y1 < sh) comp_set(rx, y1, BAND_BORDER);
	}
	// left and right edges
	for (int ry = y0; ry <= y1 && ry < sh; ry++) {
		if (x0 >= 0 && x0 < sw) comp_set(x0, ry, BAND_BORDER);
		if (x1 >= 0 && x1 < sw) comp_set(x1, ry, BAND_BORDER);
	}
}

int main(void)
{
	printf("\n:: loading e3...\n");

	printf(":: mkdir " DT_DIR "...\n");
	mkdir(DT_DIR, 0);

    int fd;
    fd = open(DT_CMD, O_WRONLY | O_CREAT); if (fd >= 0) close(fd);
    //fd = open(DT_DIRTY, O_WRONLY | O_CREAT);
    //if (fd >= 0) close(fd);


    printf(":: reading framebuffer...\n");
    int fb = open(FRAMEBUFFER_DEV, O_RDWR);
    printf(":: reading mouse...\n");
    int mfd = open(MOUSE_DEV, O_RDONLY);
    printf(":: reading keyboard...\n");
    int kfd = open(KEYBOARD_DEV, O_RDONLY);

    if (fb < 0 || mfd < 0) return 1;

    fb_info_t info;
    printf(":: reading fbinfo...\n");
    ioctl(fb, FB_IOCTL_GET_INFO, &info);

    int scr_w = (int)info.width;
    int scr_h = (int)info.height;

    //TODO:
    // look for real display size
    if (scr_w <= 0) scr_w = 1024;
    if (scr_h <= 0) scr_h = 768;

    int internal_w = scr_w;
    int internal_h = scr_h;
    #if RENDERER_SCALING_ENABLED
        internal_w = scr_w + scr_w / 2;
        internal_h = scr_h + scr_h / 2;
    #endif

    shm_host_init();

    input_set_screen_size(internal_w, internal_h);
    cmd_set_screen_size(internal_w, internal_h);
    input_init();

    comp_init(fb, internal_w, internal_h);
    bg_init(internal_w, internal_h);
    cur_init(fb, internal_w, internal_h);
    taskbar_init(internal_w, internal_h);

    bg_draw_full();
    comp_flush();

    printf(
    	"loading e3 was a success!\n"
     	"Welcome to e3!\n\n"
    );

    input_state_t is;
    is.cx = internal_w / 2;
    is.cy = internal_h / 2;


    is.win_changed = 0;
    is.sel_active = 0;
    is._sel_was_active = 0;
    is.sel_x0 = 0; is.sel_y0 = 0;
    is.sel_x1 = 0; is.sel_y1 = 0;
    is.sel_px1 = 0; is.sel_py1 = 0;

    cur_draw_fb(is.cx, is.cy);
    write_cursor_pos(is.cx, is.cy);

    int poll_tick = 0;
    int first_frame = 1;

    #define POLL_INTERVAL 4

    for (;;)
    {
        int need_full = 0;
        if (first_frame)
        {
            need_full = 1;
            first_frame = 0;
        }
        int need_cur  = 0;
        cmd_result_t cr;

        cr.count = 0;
        cr.win_changed = 0;
        dirty_begin();
        poll_tick++;

        if (poll_tick >= POLL_INTERVAL)
        {
            poll_tick = 0;
            cmd_process(&cr);
        }

        int content_dirty = poll_dirty_windows();
        if (content_dirty) need_full = 1;

        //int content_refreshed = refresh_dirty_win_bufs();
        //if (content_refreshed > 0) need_full = 1;
        //if (cr.win_changed) need_full = 1;

        input_frame_begin(&is);

        if (input_drain(mfd, &is))
        {
            need_cur = 1;
            write_cursor_pos(is.cx, is.cy);
        }
        input_drain_keyboard(kfd);

        int struct_changed = cr.win_changed || is.win_changed;
        if (is.win_changed)
        {
        	need_full = 1;
         	is.win_changed = 0;
        }

        int was_active = is._sel_was_active;
        is._sel_was_active = is.sel_active;

        if (is.sel_active || was_active) need_full = 1;

        if (need_full)
        {
            //refresh_win_bufs();
            comp_capture();

            {
            	int px = cur_last_x();
             	int py = cur_last_y();

              	dirty_mark(px, py, cur_w(), cur_h());
            }

            cur_undo_from_backbuf();

            // rubberband fix; @offihito
            if (was_active)
            {
                int x0 = is.sel_x0 < is.sel_px1 ? is.sel_x0 : is.sel_px1;
                int y0 = is.sel_y0 < is.sel_py1 ? is.sel_y0 : is.sel_py1;
                int x1 = is.sel_x0 > is.sel_px1 ? is.sel_x0 : is.sel_px1;
                int y1 = is.sel_y0 > is.sel_py1 ? is.sel_y0 : is.sel_py1;

                bg_draw_rect_dirty(x0, y0, x1 - x0 + 1, y1 - y0 + 1);
            }

            sync_home_to_current();
            clear_cmd_rects(&cr);
            clear_closed_windows();
            render_band(&is);

            if (struct_changed || is.sel_active || was_active)
            {
                render_all(is.cx, is.cy);
                for (int i = 0; i < DT_WIN_MAX; i++)
                {
                    dt_win_t *wn = win_get(i);
                    if (wn) dirty_mark(wn->x, wn->y, wn->w, wn->h);
                }
            }
            else
            {
                render_all_in_rect(
                	g_dirty_x0, g_dirty_y0,
                    g_dirty_x1 - g_dirty_x0,
                    g_dirty_y1 - g_dirty_y0,
                    is.cx, is.cy
                );
            }

            taskbar_draw(is.cx, is.cy, 0);
            dirty_mark(0, taskbar_y(), comp_w(), TB_H);
            cur_bake(is.cx, is.cy);
            dirty_mark(cur_last_x(), cur_last_y(), cur_w(), cur_h());

            {
                int fx = g_dirty_x0, fy = g_dirty_y0;
                int fw = g_dirty_x1 - g_dirty_x0;
                int fh = g_dirty_y1 - g_dirty_y0;
                if (fx < 0) { fw += fx; fx = 0; }
                if (fy < 0) { fh += fy; fy = 0; }
                if (fx + fw > comp_w()) fw = comp_w() - fx;
                if (fy + fh > comp_h()) fh = comp_h() - fy;
                if (fw > 0 && fh > 0) comp_flush_rect(fx, fy, fw, fh);
            }
        }
        else if (need_cur)
        {
            cur_erase_fb();
            cur_draw_fb(is.cx, is.cy);
        }

        if (!need_full && !need_cur)
        {
            yield();
        }
    }
}
