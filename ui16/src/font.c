/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 sulfurLabs
 *
 * PROJECT: s4
 * FILE: font.c
 *
 */

#include "ui16_priv.h"
#include "data/font8x12.h"
#include "data/font8x12_bold.h"

unsigned short ui16__glyphRow(ui16_font_kind_t font, unsigned char character, int row)
{
    if (row < 0 || row >= UI16_GLYPH_HEIGHT) return 0;

    unsigned char glyph_index = character & 0x7F;

    if (font == UI16_FONT_BOLD) return font_8x12_bold[glyph_index][row];

    return font_8x12[glyph_index][row];
}