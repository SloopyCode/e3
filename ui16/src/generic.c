/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 sulfurLabs
 *
 * PROJECT: s4
 * FILE: generic.c
 *
 */

#include "ui16.h"

//TODO:
// read generic style from ~/.config/ui16/index.ui16

ui16_style_t ui16__genericStyle(void)
{
    ui16_style_t generic_style;

    generic_style.width = fill;
    generic_style.height = fill;
    generic_style.min_width  = -1;
    generic_style.max_width  = -1;
    generic_style.min_height = -1;
    generic_style.max_height = -1;
    generic_style.background = rgb(45, 45, 45);
    generic_style.color = 0xFFFFFFFFu;
    generic_style.padding = 0;
    generic_style.margin = 0;
    generic_style.gap = 0;
    generic_style.radius = 0;
    generic_style.border_width = 0;
    generic_style.border_color = 0;
    generic_style.layout = row;
    generic_style.font = fontRegular;
    generic_style.align_items = UI16_ALIGN_START;
    generic_style.justify_content = UI16_JUSTIFY_START;
    generic_style.wrap = 0;
    generic_style.position = UI16_POSITION_STATIC;
    generic_style.left = 0;
    generic_style.top = 0;
    generic_style.layer = 0;

    return generic_style;
}