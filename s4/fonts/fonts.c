/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 sulfurLabs
 *
 * PROJECT: s4
 * FILE: fonts.c
 *
 */

#include "fonts.h"
#include "../../libpsf/psf.h"
#include "data/font8x12.h"
#include "data/font8x12_bold.h"
#include <stdint.h>
#include <stdio.h>

static psf_font_t g_deco_font;
static int g_deco_loaded = 0;

int fonts_deco_load(const char *path)
{
    if (psf_load(path, &g_deco_font) != 0)
    {
        printf(":: fonts: didnt load psf font\n");
        g_deco_loaded = 0;
        return -1;
    }

    printf(":: fonts: loaded psf deco font with success\n");

    g_deco_loaded = 1;
    return 0;
}
int fonts_deco_loaded(void)
{
    return g_deco_loaded;
}

uint32_t fonts_deco_glyph_row(unsigned int codepoint, int row)
{
    if (!g_deco_loaded) return 0;

    int gi = psf_glyph_index(&g_deco_font, codepoint);
    if (gi < 0) return 0;

    return psf_glyph_row_bits(&g_deco_font, (uint32_t)gi, row);
}

int fonts_deco_w(void)
{
    return g_deco_loaded ? psf_width(&g_deco_font) : 0;
}
int fonts_deco_h(void)
{
    return g_deco_loaded ? psf_height(&g_deco_font) : 0;
}

uint16_t font_glyph(font_id_t font, unsigned char c, int row)
{
    if (row < 0 || row >= 12) return 0;
    switch (font)
    {
        default:
        case FONT8X12:
            return font_8x12[c & 0x7F][row];
        case FONT8X12_BOLD:
            return font_8x12_bold[c & 0x7F][row];
    }
}

int font_w(font_id_t font)
{
	(void)font;
	return 8;
}
int font_h(font_id_t font)
{
	(void)font;
	return 12;
}