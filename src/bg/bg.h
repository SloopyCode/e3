/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: e3
 * FILE: bg.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#pragma once

#include "../config/cfg.h"
#define BG_PATH SYSTEM "background/2.bmp"

void bg_init(int w, int h);
void bg_draw_full(void);
void bg_draw_rect(int x, int y, int w, int h);
