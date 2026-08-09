/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 sulfurLabs
 *
 * PROJECT: s4
 * FILE: fb_backend.c
 *
 */

#include "fb_backend.h"
#include <unistd.h>

int fb_backend_init(fb_backend_t *fb, int fb_fd)
{
    fb_info_t info;
    unsigned long vaddr;

    fb->fd = fb_fd;
    fb->width = 0;
    fb->height = 0;
    fb->stride = 0;
    fb->pixels = 0;

    if (ioctl(fb_fd, FB_IOCTL_GET_INFO, &info) < 0) return 0;

    fb->width = (int)info.width;
    fb->height = (int)info.height;
    fb->stride = info.pitch / 4;

    vaddr = 0;
    if (ioctl(fb_fd, FB_IOCTL_MAP, &vaddr) < 0) return 0;

    fb->pixels = (unsigned int *)vaddr;
    return 1;
}

void fb_backend_flush_rect(
    const fb_backend_t *fb,
    unsigned int x,
    unsigned int y,
    unsigned int w,
    unsigned int h
) {
    fb_rect_t r;
    r.x = x;
    r.y = y;
    r.width = w;
    r.height = h;
    ioctl(fb->fd, FB_IOCTL_FLUSH_RECT, &r);
}

void fb_backend_flush_all(const fb_backend_t *fb)
{
    ioctl(fb->fd, FB_IOCTL_FLUSH, 0);
}
