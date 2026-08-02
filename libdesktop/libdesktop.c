/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 sulfurLabs
 *
 * PROJECT: s4
 * FILE: libdesktop.c
 */

#include "libdesktop.h"

#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <sys/shm.h>

#define DT_ABI_VERSION 3

#define DT_CMD         "/tmp/dt/cmd"
#define DT_DIRTY_PFX   "/tmp/dt/dirty_"
#define DT_WSIZE_PFX   "/tmp/dt/wsize_"
#define DT_CURSOR      "/tmp/dt/cursor"
//#define DT_WBUF_PFX    "/tmp/dt/wbuf_"
#define DT_INPUT_PFX   "/tmp/dt/input_"

#define DT_QUEUE_BYTES (sizeof(int) + sizeof(dt_event_t) * DT_EVENT_QUEUE_MAX)

static void _itoa(int v, char *out)
{
    char tmp[16];
    int i = 0;
    int j = 0;
    int neg = (v < 0);

    if (v == 0)
    {
    	out[0] = '0';
     	out[1] = '\0';
      	return;
    }
    if (neg) v =- v;

    while (v)
    {
    	tmp[i++] = '0' + v % 10;
     	v /= 10;
    }
    if (neg) tmp[i++] = '-';

    while (i > 0) out[j++] = tmp[--i];

    out[j]='\0';
}

static int _slen(const char *s)
{
	int n = 0;
	while(s[n]) n++;
	return n;
}

static void _pid_path(const char *pfx, char *out)
{
    int i = 0;
    int j = 0;
    char ps[12];

    while (*pfx) out[i++] = *pfx++;

    _itoa((int) getpid(), ps);

    while (ps[j]) out[i++] = ps[j++];

    out[i] = '\0';
}

static void _app_int(char *buf, int *pos, int v)
{
    char tmp[12];
    int i = 0;
    _itoa(v, tmp);
    while (tmp[i]) buf[(*pos )++] = tmp[i++];
}

static void _app_str(char *buf, int *pos, const char *s)
{
    while (*s) buf[(*pos)++] = *s++;
}

// handles structural commands (OCTM)
static void _cmd_append(const char *line)
{
    static char existing[4096];

    int fd = open(DT_CMD, O_RDONLY);
    int elen = 0;
    if (fd >= 0)
    {
        int r = (int) read(fd, existing, sizeof(existing)-1);
        close(fd);
        if (r > 0)
        {
        	existing[r] = '\0';
         	elen = r;
        }
    }

    fd = open(DT_CMD, O_WRONLY | O_CREAT);
    if (fd < 0) return;
    if (elen > 0) write(fd, existing, (unsigned)elen);

    write(fd, line, (unsigned)_slen(line));
    close(fd);
}

static int _createWindow(
	const char *title,
	int x,
	int y,
	int w,
	int h,
	unsigned int style
) {
    char buf[256];
    int p = 0;
    buf[p++] = 'O';
    buf[p++] = ' ';

    _app_int(buf, &p, (int)getpid());
    buf[p++] = ' ';

    _app_int(buf, &p, (int)style);
    buf[p++] = ' ';

    _app_int(buf, &p, x);
    buf[p++] = ' ';

    _app_int(buf, &p, y);
    buf[p++] = ' ';

    _app_int(buf, &p, w);
    buf[p++] = ' ';

    _app_int(buf, &p, h);
    buf[p++] = ' ';

    _app_str(buf, &p, title);
    buf[p++] = '\n';

    buf[p] = '\0';
    _cmd_append(buf);

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
    int p = 0;

    buf[p++] = 'C';
    buf[p++] = ' ';

    _app_int(buf, &p, (int)getpid());
    buf[p++] = '\n';
    buf[p] = '\0';

    _cmd_append(buf);
}

static void _setTitle(const char *title)
{
    char buf[256];
    int p = 0;

    buf[p++] = 'T';
    buf[p++] = ' ';

    _app_int(buf, &p, (int)getpid());
    buf[p++] = ' ';

    _app_str(buf, &p, title);
    buf[p++] = '\n';

    buf[p] = '\0';
    _cmd_append(buf);
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
    int p = 0;

    buf[p++] = 'S'; buf[p++] = ' ';

    _app_int(buf, &p, (int)getpid());
    buf[p++] = ' ';

    _app_int(buf, &p, (int)shm_id);
    buf[p++] = ' ';

    _app_int(buf, &p, w);
    buf[p++] = ' ';

    _app_int(buf, &p, h);
    buf[p++] = '\n';

    buf[p] = '\0';

    _cmd_append(buf);
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

    char path[64];
    _pid_path("/tmp/dt/dirty_", path);

    int fd = open(path, O_WRONLY | O_CREAT);
    if (fd >= 0)
    {
    	write(fd, "1", 1);
     	close(fd);
    }
}

static int _pollEvents(dt_event_t *buf, int max)
{
    if (!buf || max <= 0) return 0;

    char path[64];
    unsigned char raw[DT_QUEUE_BYTES];

    _pid_path(DT_INPUT_PFX, path);

    int fd = open(path, O_RDONLY);
    if (fd < 0) return 0;

    int bytes = (int)read(fd, raw, sizeof(raw));
    close(fd);

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

    fd = open(path, O_WRONLY | O_CREAT);
    if (fd >= 0)
    {
        write(fd, &remaining, sizeof(remaining));
        if (remaining > 0) write(fd, evs, (size_t)remaining * sizeof(dt_event_t));
        close(fd);
    }

    return amount;
}

static void _getCurrentMousePos(int *out_x, int *out_y)
{
    if (!out_x || !out_y) return;

    *out_x = 0;
    *out_y = 0;
    char buf[32];

    int fd = open(DT_CURSOR, O_RDONLY);
    if (fd < 0) return;

    int n = (int)read(fd, buf, sizeof(buf) -1);
    close(fd);

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
    int n;
    int fd;

    _pid_path(DT_WSIZE_PFX, path);

    fd = open(path, O_RDONLY);
    if (fd < 0) return;
    n = (int)read(fd, buf, sizeof(buf) - 1);
    close(fd);

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