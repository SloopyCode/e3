/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 sulfurLabs
 *
 * PROJECT: s4
 * FILE: cmd.c
 *
 */

#include "cmd.h"
#include "../config/cfg.h"
#include "../win/win.h"
#include "../shm/shm_host.h"

#include "../desktop_input_dispatch.h"

#include <unistd.h>
#include <fcntl.h>

static int g_scr_w = 1280;
static int g_scr_h = 720;

void cmd_set_screen_size(int w, int h)
{
    g_scr_w = w;
    g_scr_h = h;
}

static int str_to_int(const char *s)
{
    int neg = 0, v = 0;
    while (*s == ' ') s++;
    if (*s == '-') { neg = 1; s++; }
    while (*s >= '0' && *s <= '9') v = v * 10 + (*s++ - '0');
    return neg ? -v : v;
}

static const char *next_tok(const char **p)
{
    static char tok[DT_TITLE_MAX];
    int i = 0;
    while (**p == ' ') (*p)++;
    while (**p && **p != ' ' && **p != '\n' && i < DT_TITLE_MAX - 1)
        tok[i++] = *(*p)++;
    tok[i] = '\0';
    return tok;
}

static void push_rect(cmd_result_t *r, dt_win_t *wn)
{
    if (!wn || r->count >= CMD_DIRTY_MAX) return;
    drag_info_t *d = &r->rects[r->count++];

    d->valid = 1;

    d->pid = wn->pid;
    d->wx  = wn->x;
    d->wy  = wn->y;
    d->ww  = wn->w;
    d->wh  = wn->h;
}

static void clear_file(const char *path, int n)
{
    if (n <= 0) return;
    int fd = open(path, O_WRONLY | O_CREAT);
    if (fd < 0) return;
    static char clr[4096];
    int cnt = n < 4096 ? n : 4096;
    for (int i = 0; i < cnt; i++) clr[i] = '\n';
    write(fd, clr, (unsigned)cnt);
    close(fd);
}

static void process_line(const char *line, cmd_result_t *result)
{
    if (!line[0] || line[0] == '\n') return;
    char cmd = line[0];
    const char *p = line + 1;

    if (cmd == 'O')
    {
        pid_t pid = (pid_t)str_to_int(next_tok(&p));
        unsigned int style = (unsigned int)str_to_int(next_tok(&p));

        char title[DT_TITLE_MAX];

        int x = str_to_int(next_tok(&p)), y = str_to_int(next_tok(&p));
        int w = str_to_int(next_tok(&p)), h = str_to_int(next_tok(&p));
        int ti = 0;


        while (*p == ' ') p++;
        while (*p && *p != '\n' && ti < DT_TITLE_MAX - 1) title[ti++] = *p++;

        title[ti] = '\0';

        int idx = win_add(pid, title, x, y, w, h, style);

        #if ENABLE_TILING
            if (!(style & DT_POPUP) && !(style & DT_NOTITLE))
            {
                win_maximize(idx, g_scr_w, g_scr_h, TB_H);
            }
        #endif

        win_focus(idx);
        result->win_changed = 1;
        {
            dt_win_t *wn = win_get(idx);
            if (wn) dt_write_window_size(pid, wn->home_cw, wn->home_ch);
        }
    } else if (cmd == 'C')
    {
        pid_t pid = (pid_t)str_to_int(next_tok(&p));
        int idx = win_find_pid(pid);

        if (idx >= 0) push_rect(result, win_get(idx));
        win_remove(pid);
        result->win_changed = 1;

    } else if (cmd == 'T')
    {
        pid_t pid = (pid_t)str_to_int(next_tok(&p));
        char title[DT_TITLE_MAX];
        int ti = 0;

        while (*p == ' ') p++;
        while (*p && *p != '\n' && ti < DT_TITLE_MAX - 1) title[ti++] = *p++;
        title[ti] = '\0';

        win_set_title(pid, title);
        result->win_changed = 1;

    } else if (cmd == 'M')
    {
        pid_t pid = (pid_t)str_to_int(next_tok(&p));
        int nx = str_to_int(next_tok(&p)), ny = str_to_int(next_tok(&p));
        int idx = win_find_pid(pid);

        if (idx >= 0) push_rect(result, win_get(idx));
        win_move(idx, nx, ny);
        result->win_changed = 1;
    } else if (cmd == 'S')
    {
        // S <pid> <shm_id> <w> <h>
        pid_t pid = (pid_t)str_to_int(next_tok(&p));
        unsigned long long shm_id = (unsigned long long)str_to_int(next_tok(&p));
        int w = str_to_int(next_tok(&p));
        int h = str_to_int(next_tok(&p));

        unsigned int *mapped = shm_host_map(shm_id);
        if (mapped) win_set_shm_buffer(pid, shm_id, mapped, w, h);
    }
}

void cmd_process(cmd_result_t *result)
{
    static char buf[4096];
    //static char zeros[4096];
    result->count = 0;
    result->win_changed = 0;

    int fd = open(DT_CMD, O_RDONLY);
    if (fd < 0) return; /* if file exists */

    int n = (int)read(fd, buf, sizeof(buf) - 1);

    close(fd);
    if (n <= 0) return;
    buf[n] = '\0';
    if (buf[0] == '\0') return;

    clear_file(DT_CMD, n);

    /* process*/
    const char *line = buf;
    while (*line) {
        process_line(line, result);
        while (*line && *line != '\n') line++;
        if (*line == '\n') line++;
    }
}

int cmd_check_dirty(void)
{
    char buf[4];
    int fd = open(DT_DIRTY, O_RDONLY);
    if (fd < 0) return 0;

    int n = (int)read(fd, buf, 1);
    close(fd);
    if (n <= 0 || buf[0] != '1') return 0;

    fd = open(DT_DIRTY, O_WRONLY | O_CREAT);
    if (fd >= 0)
    {
        write(fd, "\n\n", 2);
        close(fd);
    }
    return 1;
}
