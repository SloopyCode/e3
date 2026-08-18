/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 sulfurLabs
 *
 * PROJECT: s4
 * FILE: ipc.c
 *
 */

#include "ipc.h"
#include <stdio.h>
#include <string.h>

static int g_scr_w = 1280;
static int g_scr_h = 720;

void ipc_init(void)
{
    int wakefd = dt_ipc_desktop_init();
    (void)wakefd;
    printf(":: init ipc\n");
}

void ipc_set_screen_size(int w, int h)
{
    g_scr_w = w;
    g_scr_h = h;
}

#define IPC_QUEUE_BYTES (sizeof(int) + sizeof(dt_event_t) * DT_EVENT_QUEUE_MAX)

void ipc_dispatch_event(pid_t focused_pid, const dt_event_t *ev)
{
    if (focused_pid <= 0 || !ev) return;

    if (dt_ipc_uses_kernel())
    {
        dt_ipc_write(DT_CHAN_INPUT, focused_pid, ev, sizeof(*ev));
        return;
    }

    static unsigned char raw[IPC_QUEUE_BYTES];
    int count = 0;

    int r = dt_ipc_read(DT_CHAN_INPUT, focused_pid, raw, sizeof(raw));
    if (r >= (int)sizeof(int))
    {
        memcpy(&count, raw, sizeof(int));
        if (count < 0 || count > DT_EVENT_QUEUE_MAX) count = 0;
    }

    if (count >= DT_EVENT_QUEUE_MAX) return;

    dt_event_t *evs = (dt_event_t *)(raw + sizeof(int));
    evs[count] = *ev;
    count++;
    memcpy(raw, &count, sizeof(int));

    dt_ipc_write(
        DT_CHAN_INPUT, focused_pid, raw,
        (unsigned)(sizeof(int) + sizeof(dt_event_t) * (unsigned)count)
    );
}

void ipc_clear_input(pid_t pid)
{
    if (pid <= 0) return;

    int zero = 0;
    dt_ipc_write(DT_CHAN_INPUT, pid, &zero, sizeof(zero));
}

int ipc_make_mouse_event(int focused_idx, int abs_x, int abs_y, unsigned char buttons, dt_event_t *out)
{
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

int ipc_make_key_event(unsigned int keycode, unsigned char modifiers, unsigned char pressed, dt_event_t *out)
{
    out->type      = DT_EV_KEY;
    out->keycode   = keycode;
    out->modifiers = modifiers;
    out->pressed   = pressed;
    out->mx        = 0;
    out->my        = 0;
    out->buttons   = 0;
    out->scroll    = 0;

    return 1;
}

int ipc_make_resize_event(int content_w, int content_h, dt_event_t *out)
{
    out->type = DT_EV_RESIZE;
    out->mx = out->my = 0;
    out->buttons = out->scroll = out->keycode = out->pressed = out->modifiers = 0;
    out->width  = content_w;
    out->height = content_h;

    return 1;
}

void ipc_clamp_popup(int *x, int *y, int w, int h)
{
    if (*x + w > g_scr_w) *x = g_scr_w - w;
    if (*y + h > g_scr_h) *y = g_scr_h - h;
    if (*x < 0) *x = 0;
    if (*y < 0) *y = 0;
}

void ipc_publish_window_size(pid_t pid, int w, int h)
{
    if (pid <= 0) return;

    char buf[32];
    char ns[12];
    int i = 0;
    int j = 0;

    dt_ipc_itoa(w, ns); j = 0; while (ns[j]) buf[i++] = ns[j++];
    buf[i++] = ' ';
    dt_ipc_itoa(h, ns); j = 0; while (ns[j]) buf[i++] = ns[j++];
    buf[i++] = '\n';

    dt_ipc_write(DT_CHAN_WSIZE, pid, buf, (unsigned)i);
}

void ipc_publish_cursor(int x, int y)
{
    char buf[32];
    char xs[12];
    char ys[12];
    int i = 0;
    int j = 0;

    dt_ipc_itoa(x, xs);
    dt_ipc_itoa(y, ys);

    while (xs[j]) buf[i++] = xs[j++];
    buf[i++] = ' ';
    j = 0;
    while (ys[j]) buf[i++] = ys[j++];
    buf[i++] = '\n';

    dt_ipc_write(DT_CHAN_CURSOR, 0, buf, (unsigned)i);
}

int ipc_window_is_dirty(pid_t pid)
{
    char flag[2];
    int n = dt_ipc_read(DT_CHAN_DIRTY, pid, flag, 1);
    if (n <= 0 || flag[0] != '1') return 0;

    char zero = '0';
    dt_ipc_write(DT_CHAN_DIRTY, pid, &zero, 1);
    return 1;
}

int ipc_read_cmd_batch(char *buf, unsigned max)
{
    return dt_ipc_read(DT_CHAN_CMD, 0, buf, max);
}

void ipc_request_open_window(
    pid_t pid,
    unsigned int style,
    int x,
    int y,
    int w,
    int h,
    const char *title
) {
    char line[256];
    dt_ipc_build_open_cmd(line, sizeof(line), pid, style, x, y, w, h, title ? title : "");
    dt_ipc_cmd_append(line);
}
