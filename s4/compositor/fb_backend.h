/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 sulfurLabs
 *
 * PROJECT: s4
 * FILE: fb_backend.h
 *
 */

#pragma once

#include <sys/fb.h>

typedef struct
{
    int fd;
    int width;
    int height;
    unsigned int stride;
    unsigned int *pixels;
} fb_backend_t;

int fb_backend_init(fb_backend_t *fb, int fb_fd);

void fb_backend_flush_rect(const fb_backend_t *fb, unsigned int x, unsigned int y, unsigned int w, unsigned int h);
void fb_backend_flush_all(const fb_backend_t *fb);
