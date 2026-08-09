/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 sulfurLabs
 *
 * PROJECT: s4
 * FILE: render_target.c
 *
 */

#include "render_target.h"
#include "../compositor/comp.h"

static int g_dx0;
static int g_dy0;
static int g_dx1;
static int g_dy1;

void rt_damage_begin(void)
{
    g_dx0 = 0x7FFFFFFF;
    g_dy0 = 0x7FFFFFFF;
    g_dx1 = -1;
    g_dy1 = -1;
}

void rt_damage_mark(int x, int y, int w, int h)
{
    if (w <= 0 || h <= 0) return;
    if (x < g_dx0) g_dx0 = x;
    if (y < g_dy0) g_dy0 = y;
    if (x + w > g_dx1) g_dx1 = x + w;
    if (y + h > g_dy1) g_dy1 = y + h;
}

void rt_damage_get(int *x0, int *y0, int *x1, int *y1)
{
    *x0 = g_dx0;
    *y0 = g_dy0;
    *x1 = g_dx1;
    *y1 = g_dy1;
}

int rt_damage_empty(void)
{
    return g_dx1 <= g_dx0 || g_dy1 <= g_dy0;
}

unsigned int rt_get(int x, int y)
{
    return comp_get(x, y);
}

void rt_set(int x, int y, unsigned int color)
{
    comp_set(x, y, color);
}

void rt_fill(int x, int y, int w, int h, unsigned int color)
{
    comp_fill(x, y, w, h, color);
}

void rt_put_row(int x, int y, const unsigned int *row, int len)
{
    comp_put_row(x, y, row, len);
}

void rt_put_pixels(int x, int y, int w, int h, const unsigned int *pixels)
{
    comp_put_pixels(x, y, w, h, pixels);
}

void rt_copy_rect(int src_x, int src_y, int dst_x, int dst_y, int w, int h)
{
    comp_copy_rect(src_x, src_y, dst_x, dst_y, w, h);
}

int rt_width(void)
{
    return comp_w();
}

int rt_height(void)
{
    return comp_h();
}

void rt_flush_rect(int x, int y, int w, int h)
{
    comp_flush_rect(x, y, w, h);
}
