/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 sulfurLabs
 *
 * PROJECT: s4
 * FILE: fonts.h
 *
 */

#pragma once

#include <stdint.h>

typedef enum
{
    FONT8X12      = 0,
    FONT8X12_BOLD = 1,
} font_id_t;

//psf stuff
int fonts_deco_load(const char *path);
int fonts_deco_loaded(void);
uint32_t fonts_deco_glyph_row(unsigned int codepoint, int row);

int fonts_deco_w(void);
int fonts_deco_h(void);

// returns the glyph row bits for character c at row [0..11]
uint16_t font_glyph(font_id_t font, unsigned char c, int row);

// font dimensions
int font_w(font_id_t font);
int font_h(font_id_t font);