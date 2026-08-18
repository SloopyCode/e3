/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 sulfurLabs
 *
 * PROJECT: s4
 * FILE: libdesktop.c
 */

#include "libdesktop.h"
#include "dt_ipc.h"

#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <sys/shm.h>

#define DT_ABI_VERSION 3

static int _createWindow(
	const char *title,
	int x,
	int y,
	int w,
	int h,
	unsigned int style
) {
    if (dt_ipc_uses_kernel())
    {
        char event_name[DT_IPC_PATH_MAX];
        /* dt_ipc_read creates the client mailbox lazily before O is sent. */
        (void)event_name;

        dt_ipc_read(DT_CHAN_INPUT, getpid(), event_name, 0);
    }
    char buf[256];
    dt_ipc_build_open_cmd(buf, sizeof(buf), getpid(), style, x, y, w, h, title);
    dt_ipc_cmd_append(buf);
    return 0;
}

static int _createPopup(int x, int y, int w, int h)
{
    return _createWindow(
        "",
        x,
        y,
        w,
        h,
        DT_POPUP
    );
}

static void _closeWindow(void)
{
    char buf[32];
    char num[12];
    int p = 0;
    int k;

    buf[p++] = 'C';
    buf[p++] = ' ';

    dt_ipc_itoa((int)getpid(), num);
    for (k = 0; num[k]; k++) buf[p++] = num[k];

    buf[p++] = '\n';
    buf[p] = '\0';

    dt_ipc_cmd_append(buf);
}

static void _setTitle(const char *title)
{
    char buf[256];
    char num[12];
    int p = 0;
    int k;

    buf[p++] = 'T';
    buf[p++] = ' ';

    dt_ipc_itoa((int)getpid(), num);
    for (k = 0; num[k]; k++) buf[p++] = num[k];
    buf[p++] = ' ';

    while (*title) buf[p++] = *title++;
    buf[p++] = '\n';

    buf[p] = '\0';

    dt_ipc_cmd_append(buf);
}

static uint64_t s_shm_id  = 0;
static unsigned int  *s_shm_ptr = NULL;

static int s_shm_w = 0;
static int s_shm_h = 0;
static int s_shm_fd = -1;

static int ensure_shm_fd(void)
{
    if (s_shm_fd < 0) s_shm_fd = open(SHM_DEV, O_RDWR);
    return s_shm_fd;
}

static void send_shm_cmd(uint64_t shm_id, int w, int h)
{
    char buf[128];
    char num[12];
    int p = 0;
    int k;

    buf[p++] = 'S';
    buf[p++] = ' ';

    dt_ipc_itoa((int)getpid(), num);
    for (k = 0; num[k]; k++) buf[p++] = num[k]; buf[p++] = ' ';

    dt_ipc_itoa((int)shm_id, num);
    for (k = 0; num[k]; k++) buf[p++] = num[k]; buf[p++] = ' ';

    dt_ipc_itoa(w, num);
    for (k = 0; num[k]; k++) buf[p++] = num[k]; buf[p++] = ' ';

    dt_ipc_itoa(h, num);
    for (k = 0; num[k]; k++) buf[p++] = num[k]; buf[p++] = '\n';

    buf[p] = '\0';

    dt_ipc_cmd_append(buf);
}

static unsigned int *_allocFramebuffer(int w, int h)
{
    if (w <= 0 || h <= 0) return NULL;
    if (ensure_shm_fd() < 0) return NULL;

    shm_ioctl_args_t args =
    {
    	.id = 0,
     	.size = (uint64_t)w * h * 4,
      	.vaddr = 0
    };

    if (ioctl(s_shm_fd, SHM_IOCTL_ALLOC, &args) < 0) return NULL;

    s_shm_ptr = (unsigned int *)(unsigned long)args.vaddr;
    s_shm_w = w;
    s_shm_h = h;
    s_shm_id = args.id;

    send_shm_cmd(s_shm_id, w, h);
    return s_shm_ptr;
}

static unsigned int *_resizeFramebuffer(int w, int h)
{
    if (w <= 0 || h <= 0 || (w == s_shm_w && h == s_shm_h)) return s_shm_ptr;

    uint64_t old_id  = s_shm_id;
    unsigned int *old_ptr = s_shm_ptr;
    unsigned int *fresh = _allocFramebuffer(w, h);

    if (!fresh) return s_shm_ptr; //old segment

    if (old_id)
    {
        shm_ioctl_args_t args =
        {
            .id = old_id,
            .size = 0,
            .vaddr = (uint64_t)(unsigned long)old_ptr
        };
        ioctl(s_shm_fd, SHM_IOCTL_UNMAP, &args);
    }

    return fresh;
}

static void _presentFrame(void)
{
    if (!s_shm_id) return;

    char one = '1';
    dt_ipc_write(DT_CHAN_DIRTY, getpid(), &one, 1);
}

static int _pollEvents(dt_event_t *buf, int max)
{
    if (!buf || max <= 0) return 0;

    if (dt_ipc_uses_kernel())
    {
        int count = 0;
        while (count < max)
        {
            int n = dt_ipc_read(DT_CHAN_INPUT, getpid(), &buf[count], sizeof(dt_event_t));
            if (n != (int)sizeof(dt_event_t)) break;
            count++;
        }
        return count;
    }

    unsigned char raw[sizeof(int) + sizeof(dt_event_t) * DT_EVENT_QUEUE_MAX];

    int bytes = dt_ipc_read(DT_CHAN_INPUT, getpid(), raw, sizeof(raw));
    if (bytes < (int)sizeof(int)) return 0;

    int count;
    memcpy(&count, raw, sizeof(count));

    if (count <= 0) return 0;
    if (count > DT_EVENT_QUEUE_MAX) count = DT_EVENT_QUEUE_MAX;

    int available = (bytes - (int)sizeof(int)) / (int)sizeof(dt_event_t);
    if (count > available) count = available;

    dt_event_t *evs = (dt_event_t *)(raw + sizeof(int));
    int amount = count < max ? count : max;
    memcpy(buf, evs, (size_t)amount * sizeof(dt_event_t));

    int remaining = count - amount;
    if (remaining > 0) memmove(evs, evs + amount, (size_t)remaining * sizeof(dt_event_t));

    {
        unsigned char out[sizeof(int) + sizeof(dt_event_t) * DT_EVENT_QUEUE_MAX];
        memcpy(out, &remaining, sizeof(remaining));
        if (remaining > 0) memcpy(out + sizeof(int), evs, (size_t)remaining * sizeof(dt_event_t));

        dt_ipc_write(
        	DT_CHAN_INPUT, getpid(), out,
         	(unsigned)(sizeof(int) + (unsigned)remaining * sizeof(dt_event_t))
        );
    }

    return amount;
}

static void _getCurrentMousePos(int *out_x, int *out_y)
{
    if (!out_x || !out_y) return;

    *out_x = 0;
    *out_y = 0;
    char buf[32];
    int n = dt_ipc_read(DT_CHAN_CURSOR, 0, buf, sizeof(buf) - 1);
    if (n <= 0) return;
    buf[n] = '\0';

    const char *p = buf;
    int x = 0;
    int y = 0;
    while (*p >= '0' && *p <= '9') x = x * 10 + (*p++ -'0');
    while (*p == ' ') p++;
    while (*p >= '0' && *p <= '9') y = y * 10 + (*p++ -'0');

    *out_x = x;
    *out_y = y;
}

static void _getWindowSize(int *out_w, int *out_h)
{
    if (!out_w || !out_h) return;

    *out_w = 0;
    *out_h = 0;

    char path[64];
    char buf[32];
    int n = dt_ipc_read(DT_CHAN_WSIZE, getpid(), buf, sizeof(buf) - 1);
    if (n <= 0) return;
    buf[n] = '\0';

    const char *p = buf;
    int w = 0;
    int h = 0;
    while (*p >= '0' && *p <= '9') w = w * 10 + (*p++ - '0');
    while (*p == ' ') p++;
    while (*p >= '0' && *p <= '9') h = h * 10 + (*p++ - '0');

    *out_w = w;
    *out_h = h;
}

// for "desktop."
Desktop desktop =
{
    .abi_version        = DT_ABI_VERSION,
    .createWindow       = _createWindow,
    .createPopup        = _createPopup,
    .closeWindow        = _closeWindow,
    .setTitle           = _setTitle,
    .allocFramebuffer   = _allocFramebuffer,
    .resizeFramebuffer  = _resizeFramebuffer,
    .presentFrame       = _presentFrame,
    .pollEvents         = _pollEvents,
    .getCurrentMousePos = _getCurrentMousePos,
    .getWindowSize      = _getWindowSize,
};
