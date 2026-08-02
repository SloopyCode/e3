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
    generic_style.background = rgb(45, 45, 45);
    generic_style.color = 0xFFFFFFFFu;
    generic_style.padding = 0;
    generic_style.gap = 0;
    generic_style.radius = 0;
    generic_style.layout = row;
    generic_style.font = fontRegular;

    return generic_style;
}