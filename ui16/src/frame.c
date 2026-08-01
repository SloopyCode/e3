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

void ui16_frame(int screen_width, int screen_height, const ui16_renderer_t *renderer)
{
    ui16__computeLayout(screen_width, screen_height, renderer);
    ui16__renderTree(renderer);
}