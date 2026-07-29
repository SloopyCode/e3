/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 sulfurLabs
 *
 * PROJECT: s4
 * FILE: desktop_input_dispatch.c
 *
 */

#include "desktop_input_dispatch.h"
#include "desktop.h"
#include "../libdesktop/libdesktop.h"

#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define DT_INPUT_PFX "/tmp/dt/input_"
#define DT_WSIZE_PFX "/tmp/dt/wsize_"

static int g_scr_w = 1280;
static int g_scr_h = 720;

void dt_set_screen_size(int w, int h)
{
    g_scr_w = w;
    g_scr_h = h;
}

static void _pid_path(const char *pfx, pid_t pid, char *out)
{
    int i = 0;
    int j = 0;
    char ps[12];

    while (*pfx) out[i++] = *pfx++;

    _itoa((int)pid, ps);

    while (ps[j]) out[i++] = ps[j++];
    out[i] = '\0';
}

#define DT_QUEUE_BYTES (sizeof(int) + sizeof(dt_event_t) * DT_EVENT_QUEUE_MAX)
void dt_dispatch_event(pid_t focused_pid, const dt_event_t *ev)
{
    if (focused_pid<=0||!ev) return;

    char path[64];
    _pid_path(DT_INPUT_PFX, focused_pid, path);

    static unsigned char raw[DT_QUEUE_BYTES];
    int count = 0;

    int fd = open(path, O_RDONLY);

    if (fd >= 0)
    {
        int r  = (int)read(fd, raw, sizeof(raw));
        close(fd);

        if (r >= (int)sizeof(int))
        {
            count = *(int *)raw;
            if (count < 0 || count > DT_EVENT_QUEUE_MAX) count = 0;
        }
    }

    if (count >= DT_EVENT_QUEUE_MAX) return;

    dt_event_t *evs = (dt_event_t *)(raw + sizeof(int));
    evs[count]  = *ev;
    count++;
    *(int *)raw = count;

    fd = open(path, O_WRONLY | O_CREAT);
    if (fd < 0) return;

    write(fd, raw, sizeof(int) + sizeof(dt_event_t) * (unsigned)count);
    close(fd);
}

void dt_clear_input(pid_t pid)
{
    if (pid <= 0) return;

    char path[64];
    _pid_path(DT_INPUT_PFX, pid, path);
    int fd = open(path, O_WRONLY | O_CREAT);

    if (fd >= 0)
    {
        int zero = 0;
        write(fd, &zero, sizeof(zero));
        close(fd);
    }
}

int dt_make_mouse_event(
	int focused_idx,
	int abs_x,
	int abs_y,
    unsigned char buttons,
    dt_event_t *out
){
    dt_win_t *w = win_get(focused_idx);

    if (!w) return 0;

    out->type      = DT_EV_MOUSE;
    out->mx        = (short)(abs_x - w->home_cx);
    out->my        = (short)(abs_y - w->home_cy);
    out->buttons   = buttons;
    out->scroll    = 0;
    out->keycode   = 0;
    out->pressed   = 0;
    out->modifiers = 0;

    return 1;
}

int dt_make_key_event(
	unsigned int keycode,
	unsigned char modifiers,
    unsigned char pressed,
    dt_event_t *out
){
    out->type      = DT_EV_KEY;
    out->keycode   = keycode;
    out->modifiers = modifiers;
    out->pressed   = pressed;
    out->mx        =0;
    out->my        =0;
    out->buttons   =0;
    out->scroll    =0;
    return 1;
}

// popup clamping
void dt_clamp_popup(int *x, int *y, int w, int h)
{
    if (*x + w > g_scr_w)  *x = g_scr_w-w;
    if (*y + h > g_scr_h)  *y = g_scr_h-h;
    if (*x < 0) *x=0;
    if (*y < 0) *y=0;
}

int dt_make_resize_event(int content_w, int content_h, dt_event_t *out)
{
    out->type = DT_EV_RESIZE;
    out->mx = out->my = 0;
    out->buttons = out->scroll = out->keycode = out->pressed = out->modifiers = 0;
    out->width  = content_w;
    out->height = content_h;
    return 1;
}

void dt_write_window_size(pid_t pid, int w, int h)
{
    if (pid <= 0) return;

    char path[64], buf[32], ns[12];
    int i = 0, j = 0;

    _pid_path(DT_WSIZE_PFX, pid, path);

    _itoa(w, ns); j = 0; while (ns[j]) buf[i++] = ns[j++];
    buf[i++] = ' ';
    _itoa(h, ns); j = 0; while (ns[j]) buf[i++] = ns[j++];
    buf[i++] = '\n';
    buf[i] = '\0';

    int fd = open(path, O_WRONLY | O_CREAT);
    if (fd >= 0) { write(fd, buf, (unsigned)i); close(fd); }
}
