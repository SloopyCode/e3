/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 sulfurLabs
 *
 * PROJECT: s4
 * FILE: ipc.h
 *
 */

#pragma once

#include <sys/types.h>
#include "../../libdesktop/libdesktop.h" /* proly bad idea, idc xd*/
#include "../../libdesktop/dt_ipc.h"
#include "../win/win.h"

void ipc_init(void);
void ipc_set_screen_size(int w, int h);
void ipc_dispatch_event(pid_t focused_pid, const dt_event_t *ev);
void ipc_clear_input(pid_t pid);
void ipc_clamp_popup(int *x, int *y, int w, int h);
void ipc_publish_window_size(pid_t pid, int w, int h);
void ipc_publish_cursor(int x, int y);

void ipc_request_open_window(
    pid_t pid,
    unsigned int style,
    int x,
    int y,
    int w,
    int h,
    const char *title
);

int ipc_window_is_dirty(pid_t pid);
int ipc_read_cmd_batch(char *buf, unsigned max);
int ipc_make_mouse_event(int focused_idx, int abs_x, int abs_y, unsigned char buttons, dt_event_t *out);
int ipc_make_key_event(unsigned int keycode, unsigned char modifiers, unsigned char pressed, dt_event_t *out);
int ipc_make_resize_event(int content_w, int content_h, dt_event_t *out);
