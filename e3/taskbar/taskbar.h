/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: e3
 * FILE: taskbar.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#pragma once

#include "tb_widget.h"

void taskbar_init(int scr_w, int scr_h);
void taskbar_draw(int mx, int my, int btn_down);
int taskbar_click(int mx, int my);
int taskbar_y(void);
void taskbar_cmd_process(void);
int taskbar_add_widget(const tb_widget_t *w);
