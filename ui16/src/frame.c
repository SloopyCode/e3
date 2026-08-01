/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 sulfurLabs
 *
 * PROJECT: s4
 * FILE: frame.c
 *
 */

#include "ui16.h"
#include "ui16_priv.h"

void ui16_frame(void)
{
    const ui16_renderer_t *renderer = ui16__softwareRenderer();

    ui16__computeLayout(ui16__targetBufferWidth(), ui16__targetBufferHeight(), renderer);
    ui16__renderTree(renderer);
}