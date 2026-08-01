/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 sulfurLabs
 *
 * PROJECT: s4
 * FILE: ui16buttons.h
 *
 */

#pragma once

#include "ui16.h"

ui16_node_t *ui16__buttonSimple(const char *button_text);
ui16_node_t *ui16__buttonStyled(ui16_style_t button_style, const char *button_text);

#define ui16_button(...) UI16_ARG2(\
	                               __VA_ARGS__, \
								   ui16__buttonStyled, \
								   ui16__buttonSimple) \
                         (__VA_ARGS__)