/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 sulfurLabs
 *
 * PROJECT: s4
 * FILE: surface.h
 *
 */

#pragma once

typedef struct
{
    unsigned int *pixels;
    int width;
    int height;
    unsigned int stride;
} surface_t;

int surface_alloc(surface_t *s, int w, int h);
void surface_free(surface_t *s);
void surface_alias(surface_t *s, unsigned int *pixels, int w, int h, unsigned int stride);

unsigned int surface_get(const surface_t *s, int x, int y);

void surface_set(surface_t *s, int x, int y, unsigned int c);
void surface_fill(surface_t *s, int x, int y, int w, int h, unsigned int color);
void surface_put_row(surface_t *s, int x, int y, const unsigned int *row, int len);
void surface_put_pixels(surface_t *s, int x, int y, int w, int h, const unsigned int *pixels);
void surface_copy_rect(surface_t *s, int src_x, int src_y, int dst_x, int dst_y, int w, int h);
