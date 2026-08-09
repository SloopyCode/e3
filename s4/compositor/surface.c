/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 sulfurLabs
 *
 * PROJECT: s4
 * FILE: surface.c
 *
 */

#include "surface.h"
#include <stdlib.h>
#include <string.h>

int surface_alloc(surface_t *s, int w, int h)
{
    s->pixels = (unsigned int *)malloc((unsigned)(w * h) * sizeof(unsigned int));
    if (!s->pixels)
    {
        s->width = 0;
        s->height = 0;
        s->stride = 0;
        return 0;
    }

    memset(s->pixels, 0, (unsigned)(w * h) * sizeof(unsigned int));
    s->width = w;
    s->height = h;
    s->stride = (unsigned int)w;
    return 1;
}

void surface_free(surface_t *s)
{
    if (s->pixels) free(s->pixels);
    s->pixels = 0;
    s->width = 0;
    s->height = 0;
    s->stride = 0;
}

void surface_alias(surface_t *s, unsigned int *pixels, int w, int h, unsigned int stride)
{
    s->pixels = pixels;
    s->width  = w;
    s->height = h;
    s->stride = stride;
}

unsigned int surface_get(const surface_t *s, int x, int y)
{
    if (
        !s->pixels ||
        x < 0 ||
        x >= s->width ||
        y < 0 ||
        y >= s->height
    ) return 0;

    return s->pixels[(unsigned)y * s->stride + (unsigned)x];
}

void surface_set(surface_t *s, int x, int y, unsigned int c)
{
    if (
        !s->pixels ||
        x < 0 ||
        x >= s->width ||
        y < 0 ||
        y >= s->height
    ) return;

    s->pixels[(unsigned)y * s->stride + (unsigned)x] = c;
}

void surface_fill(surface_t *s, int x, int y, int w, int h, unsigned int color)
{
    if (!s->pixels) return;

    for (int dy = 0; dy < h; dy++)
    {
        int py = y + dy;
        if (py < 0 || py >= s->height) continue;
        unsigned int *row = s->pixels + (unsigned)py * s->stride;

        int x0 = x < 0 ? 0 : x;
        int x1 = (x + w) > s->width ? s->width : (x + w);

        for (int px = x0; px < x1; px++) row[px] = color;
    }
}

void surface_put_row(surface_t *s, int x, int y, const unsigned int *row, int len)
{
    if (!s->pixels || y < 0 || y >= s->height) return;
    if (!row || len <= 0) return;

    int src_offset = 0;
    if (x < 0)
    {
        src_offset = -x;
        len += x;
        x = 0;
    }
    if (x >= s->width || len <= 0) return;
    if (len > s->width - x) len = s->width - x;

    memcpy(
        s->pixels + (unsigned)y * s->stride + (unsigned)x,
        row + src_offset,
        (size_t)len * sizeof(*row)
    );
}

void surface_put_pixels(surface_t *s, int x, int y, int w, int h, const unsigned int *pixels)
{
    if (!s->pixels || !pixels) return;
    if (w <= 0 || h <= 0) return;

    int src_stride = w;
    int src_x = 0;
    int src_y = 0;
    if (x < 0) { src_x = -x; w += x; x = 0; }
    if (y < 0) { src_y = -y; h += y; y = 0; }
    if (x >= s->width || y >= s->height || w <= 0 || h <= 0) return;
    if (w > s->width - x) w = s->width - x;
    if (h > s->height - y) h = s->height - y;

    for (int row = 0; row < h; row++)
    {
        unsigned int *dst = s->pixels + (unsigned)(y + row) * s->stride + (unsigned)x;
        const unsigned int *src = pixels + (size_t)(src_y + row) * src_stride + src_x;
        memcpy(dst, src, (size_t)w * sizeof(*pixels));
    }
}

void surface_copy_rect(
    surface_t *s,
    int src_x,
    int src_y,
    int dst_x,
    int dst_y,
    int w,
    int h
){
    if (!s->pixels || w <= 0 || h <= 0) return;

    int ystep = (dst_y <= src_y) ? 1 : -1;
    int r = 0;
    int r0 = (ystep == 1) ? 0 : h - 1;
    int r1 = (ystep == 1) ? h : -1;

    for (r = r0; r != r1; r += ystep)
    {
        int sy = src_y + r;
        int dy = dst_y + r;
        if (sy < 0 || sy >= s->height || dy < 0 || dy >= s->height) continue;

        unsigned int *srow = s->pixels + (unsigned)sy * s->stride;
        unsigned int *drow = s->pixels + (unsigned)dy * s->stride;

        int xstep = (dst_x <= src_x) ? 1 : -1;
        int c = 0;
        int c0 = (xstep == 1) ? 0 : w - 1;
        int c1 = (xstep == 1) ? w : -1;

        for (c = c0; c != c1; c += xstep)
        {
            int spx = src_x + c;
            int dpx = dst_x + c;
            if (spx >= 0 && spx < s->width && dpx >= 0 && dpx < s->width) drow[dpx] = srow[spx];
        }
    }
}
