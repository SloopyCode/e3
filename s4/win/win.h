/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 sulfurLabs
 *
 * PROJECT: s4
 * FILE: win.h
 *
 */

#pragma once

#include <sys/types.h>
#include "../cfg.h"

#define DT_WIN      0x00
#define DT_POPUP    0x01
#define DT_NOMOVE   0x02
#define DT_NOTITLE  0x04

#define DT_WIN_MAX    32
#define DT_TITLE_MAX  64

typedef struct
{
    pid_t pid;
    char title[DT_TITLE_MAX];
    int x, y, w, h;
    unsigned int style;
    int valid;
    int focused;
    int z;

    int tile_seq;

    int home_cx, home_cy, home_cw, home_ch;
    int orig_cx, orig_cy, orig_cw, orig_ch;

    unsigned int *buf;

    int buf_w;
    int buf_h;
    unsigned long long shm_id;

    int maximized;
    int premax_x;
    int premax_y;
    int premax_w;
    int premax_h;
} dt_win_t;

int win_add(
    pid_t pid, const char *title,
    int x, int y, int w, int h, unsigned int style
);
void win_remove(pid_t pid);
void win_set_title(pid_t pid, const char *title);
void win_move(int idx, int nx, int ny);
void win_focus(int idx);
void win_set_shm_buffer(pid_t pid, unsigned long long shm_id, unsigned int *buf, int w, int h);
//void win_update_buf(int idx, const unsigned int *pixels, int w, int h);

#if ENABLE_TILING
    void win_tile_all(int scr_w, int scr_h, int taskbar_h);
    void win_retile_and_notify(int scr_w, int scr_h, int taskbar_h);
    void win_tile_move(pid_t pid, int dx, int dy, int scr_w, int scr_h, int taskbar_h);
#endif

int win_find_pid(pid_t pid);
int win_hit(int idx, int mx, int my);
int win_hit_title(int idx, int mx, int my);
int win_hit_close(int idx, int mx, int my);
int win_hit_maximize(int idx, int mx, int my);
int win_hit_minimize(int idx, int mx, int my);

void win_toggle_maximize(int idx, int scr_w, int scr_h, int taskbar_h);
void win_maximize(int idx, int scr_w, int scr_h, int taskbar_h);

dt_win_t *win_get(int idx);
int win_count(void);