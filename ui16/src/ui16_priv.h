/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 sulfurLabs
 *
 * PROJECT: s4
 * FILE: ui16_priv.h
 *
 */

#pragma once

#include "ui16.h"

void *ui16__alloc(unsigned long byte_count);
ui16_node_t *ui16__attachNode(ui16_node_kind_t node_kind, ui16_style_t node_style, const char *node_text);
ui16_node_t *ui16__currentParent(void);
void ui16__setCurrentParent(ui16_node_t *parent_node);
ui16_node_t *ui16__rootNode(void);

void ui16__computeLayout(int screen_width ,int screen_height, const ui16_renderer_t *renderer);
void ui16__renderTree(const ui16_renderer_t *renderer);