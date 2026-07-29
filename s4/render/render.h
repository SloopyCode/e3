/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 sulfurLabs
 *
 * PROJECT: s4
 * FILE: render.h
 *
 */

#pragma once

#include "../win/win.h"

void render_win(dt_win_t *w);
void render_all(void);
void render_all_in_rect(int x, int y, int w, int h);
