/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 sulfurLabs
 *
 * PROJECT: s4
 * FILE: startmenu.h
 *
 */
#pragma once

void startmenu_init(int screen_width, int screen_height);
void startmenu_toggle(int button_x, int taskbar_y);
void startmenu_close(void);
void startmenu_draw(int mouse_x, int mouse_y, int button_down);
int startmenu_is_open(void);
int startmenu_click(int mouse_x, int mouse_y);
