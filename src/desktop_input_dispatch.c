/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: e3
 * FILE: desktop_input_dispatch.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#include "desktop_input_dispatch.h"
#include "desktop.h"

#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define DT_INPUT_PFX "/tmp/dt/input_"

static int g_scr_w = 1280;
static int g_scr_h = 720;

void dt_set_screen_size(int w, int h)
{
    g_scr_w = w;
    g_scr_h = h;
}

static void _pid_path(pid_t pid, char *out)
{
    const char *pfx=DT_INPUT_PFX;

    int i = 0;
    int j = 0;
    char ps[12];

    while (*pfx) out[i++]=*pfx++;

    _itoa((int)pid, ps);

    while (ps[j]) out[i++]=ps[j++];
    out[i]='\0';
}

#define DT_QUEUE_BYTES (sizeof(int) + sizeof(dt_event_t) * DT_EVENT_QUEUE_MAX)
void dt_dispatch_event(pid_t focused_pid, const dt_event_t *ev)
{
    if (focused_pid<=0||!ev) return;

    char path[64];
    _pid_path(focused_pid, path);

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
    _pid_path(pid, path);
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
