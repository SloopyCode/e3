/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 sulfurLabs
 *
 * PROJECT: s4
 * FILE: win.c
 *
 */

#include "win.h"
#include "../config/cfg.h"
#include "../shm/shm_host.h"

#include <string.h>
#include <stdlib.h>

// window creating
// the guilui (gui lib) should handle the window content

static dt_win_t wins[DT_WIN_MAX];
static int win_cnt = 0;
static int z_next  = 0;

static void scopy(char *dst, const char *src, int max)
{
    int i = 0;
    while (i < max - 1 && src[i]) { dst[i] = src[i]; i++; }
    dst[i] = '\0';
}

static void compute_home(dt_win_t *w)
{
    unsigned int style = w->style;
    if (style & DT_POPUP) {
        w->home_cx = w->x + 1;
        w->home_cy = w->y + 1;
        w->home_cw = w->w - 2;
        w->home_ch = w->h - 2;
    } else if (style & DT_NOTITLE) {
        w->home_cx = w->x + DT_BORDER;
        w->home_cy = w->y + DT_BORDER;
        w->home_cw = w->w - DT_BORDER * 2;
        w->home_ch = w->h - DT_BORDER * 2;
    } else {
        w->home_cx = w->x + DT_BORDER;
        w->home_cy = w->y + DT_TITLE_H + 1;
        w->home_cw = w->w - DT_BORDER * 2;
        w->home_ch = w->h - DT_TITLE_H - 1 - DT_BORDER;
    }
    w->orig_cx = w->home_cx;
    w->orig_cy = w->home_cy;
    w->orig_cw = w->home_cw;
    w->orig_ch = w->home_ch;
}

int win_add(
	pid_t pid, const char *title,
    int x, int y, int w, int h, unsigned int style
) {
    for (int i = 0; i < DT_WIN_MAX; i++)
    {
        if (!wins[i].valid)
        {
            wins[i].pid = pid;

            wins[i].x = x;
            wins[i].y = y;
            wins[i].w = w;
            wins[i].h = h;

            wins[i].style = style;
            wins[i].valid = 1;
            wins[i].focused = 0;
            wins[i].z = z_next++;

            wins[i].buf = NULL;
            wins[i].buf_w = 0;
            wins[i].buf_h = 0;
            wins[i].shm_id  = 0;

            wins[i].maximized = 0;
            wins[i].premax_x = x;
            wins[i].premax_y = y;
            wins[i].premax_w = w;
            wins[i].premax_h = h;

            scopy(wins[i].title, title, DT_TITLE_MAX);
            compute_home(&wins[i]);

            win_cnt++;

            return i;
        }

    }

    return -1;
}

void win_remove(pid_t pid)
{
    int i = win_find_pid(pid);
    if (i < 0) return;

    if (wins[i].buf && wins[i].shm_id) shm_host_unmap(wins[i].shm_id, wins[i].buf);

    wins[i].buf = NULL;
    wins[i].buf_w = 0;
    wins[i].buf_h = 0;
    wins[i].shm_id = 0;
    wins[i].valid = 0;
    win_cnt--;
}

void win_set_shm_buffer(pid_t pid, unsigned long long shm_id, unsigned int *buf, int w, int h)
{
    int i = win_find_pid(pid);
    if (i < 0) return;

    dt_win_t *wn = &wins[i];

    if (
    	wn->buf &&
     	wn->shm_id &&
      	wn->shm_id != shm_id
    ) shm_host_unmap(wn->shm_id, wn->buf);   // free old segmnet

    wn->buf    = buf;
    wn->buf_w  = w;
    wn->buf_h  = h;
    wn->shm_id = shm_id;
}
/*
void win_update_buf(int idx, const unsigned int *pixels, int w, int h)
{
    if (idx < 0 || idx >= DT_WIN_MAX || !wins[idx].valid) return;
    if (!pixels || w <= 0 || h <= 0) return;

    dt_win_t *wn = &wins[idx];
    int sz = w * h;

    if (!wn->buf) {
        wn->buf = (unsigned int *)malloc((unsigned)(sz * 4));
        if (!wn->buf) return;
        wn->buf_w = w;
        wn->buf_h = h;
    }

    if (wn->buf_w == w && wn->buf_h == h) memcpy(wn->buf, pixels, (unsigned)(sz * 4));
}*/

void win_set_title(pid_t pid, const char *title)
{
    int i = win_find_pid(pid);
    if (i < 0) return;

    scopy(wins[i].title, title, DT_TITLE_MAX);
}

void win_move(int idx, int nx, int ny)
{
    if (idx < 0 || idx >= DT_WIN_MAX || !wins[idx].valid) return;
    wins[idx].x = nx;
    wins[idx].y = ny;
    // home stays at orig. pos.
}

void win_focus(int idx)
{
	// auto unfocuses the others then
    for (int i = 0; i < DT_WIN_MAX; i++) wins[i].focused = 0;
    if (idx >= 0 && idx < DT_WIN_MAX && wins[idx].valid)
    {
        wins[idx].focused = 1;
        wins[idx].z = z_next++;
    }
}

int win_find_pid(pid_t pid)
{
    for (int i = 0; i < DT_WIN_MAX; i++) if (wins[i].valid && wins[i].pid == pid) return i;
    return -1;
}

int win_hit(int idx, int mx, int my)
{
    dt_win_t *w = win_get(idx);
    if (!w) return 0;

    return
    	mx >= w->x && mx < w->x + w->w &&
        my >= w->y && my < w->y + w->h
    ;
}

int win_hit_title(int idx, int mx, int my)
{
    dt_win_t *w = win_get(idx);
    if (!w || (w->style & DT_POPUP) || (w->style & DT_NOTITLE)) return 0;

    return
    	mx >= w->x && mx < w->x + w->w &&
        my >= w->y && my < w->y + DT_TITLE_H
    ;
}

int win_hit_close(int idx, int mx, int my)
{
    dt_win_t *w = win_get(idx);
    if (!w || (w->style & DT_POPUP) || (w->style & DT_NOTITLE)) return 0;
    int bx = w->x + DT_CLOSE_X;
    int by = w->y + DT_CLOSE_Y;
    return
    	mx >= bx && mx < bx + DT_CLOSE_SZ &&
        my >= by && my < by + DT_CLOSE_SZ
    ;
}

int win_hit_maximize(int idx, int mx, int my)
{
	#if ENABLE_TILING
		// tiling mode auto maximise so the button isnt needed
		(void)idx;
		(void)mx;
		(void)my;
		return 0;
	#else
		dt_win_t *w = win_get(idx);
		if (!w || (w->style & DT_POPUP) || (w->style & DT_NOTITLE)) return 0;

		int bx = w->x + DT_MAX_X;
		int by = w->y + DT_MAX_Y;
		return
		   	mx >= bx && mx < bx + DT_MAX_SZ &&
		    my >= by && my < by + DT_MAX_SZ
		;
	#endif
}

void win_maximize(int idx, int scr_w, int scr_h, int taskbar_h)
{
    dt_win_t *w = win_get(idx);
    if (!w) return;

    if (!w->maximized)
    {
        w->premax_x = w->x;
        w->premax_y = w->y;
        w->premax_w = w->w;
        w->premax_h = w->h;
    }

    w->x = 0;
    w->y = 0;
    w->w = scr_w;
    w->h = scr_h - taskbar_h;
    w->maximized = 1;

    compute_home(w);
}

void win_toggle_maximize(int idx, int scr_w, int scr_h, int taskbar_h)
{
    dt_win_t *w = win_get(idx);
    if (!w) return;

    if (w->maximized)
    {
        w->x = w->premax_x;
        w->y = w->premax_y;
        w->w = w->premax_w;
        w->h = w->premax_h;
        w->maximized = 0;

        compute_home(w);
        return;
    }

    win_maximize(idx, scr_w, scr_h, taskbar_h);
}

dt_win_t *win_get(int idx)
{
    if (idx < 0 || idx >= DT_WIN_MAX || !wins[idx].valid) return 0;
    return &wins[idx];
}

int win_count(void)
{
	return win_cnt;
}