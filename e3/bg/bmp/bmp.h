/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: e3
 * FILE: bmp.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#pragma once

/*
 * 24bpp and 32bpp , uncompressed BMP
 *
 */

#include "../../compositor/comp.h"

typedef struct {
    int width;
    int height;
    unsigned int *pixels; /* ARGB 0xAARRGGBB; top-down */
} bmp_image_t;


int bmp_load(const char *path, bmp_image_t *img);

void bmp_free(bmp_image_t *img);

void bmp_draw(const bmp_image_t *img, int x, int y);

void bmp_draw_scaled(
	const bmp_image_t *img,
    int x, int y, int w, int h
);

void bmp_draw_ex(
	const bmp_image_t *img,
    int x, int y, int w, int h,
    int sat, int bright, int alpha
);