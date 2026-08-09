/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 sulfurLabs
 *
 * PROJECT: s4
 * FILE: render_target.h
 *
 */

#pragma once

void rt_damage_begin(void);
void rt_damage_mark(int x, int y, int w, int h);
void rt_damage_get(int *x0, int *y0, int *x1, int *y1);
void rt_set(int x, int y, unsigned int color);
void rt_fill(int x, int y, int w, int h, unsigned int color);
void rt_put_row(int x, int y, const unsigned int *row, int len);
void rt_put_pixels(int x, int y, int w, int h, const unsigned int *pixels);
void rt_copy_rect(int src_x, int src_y, int dst_x, int dst_y, int w, int h);
void rt_flush_rect(int x, int y, int w, int h);

int rt_width(void);
int rt_height(void);
int rt_damage_empty(void);

unsigned int rt_get(int x, int y);
