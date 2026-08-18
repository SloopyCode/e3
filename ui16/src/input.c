/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 sulfurLabs
 *
 * PROJECT: s4
 * FILE: input.c
 *
 */

#include "ui16.h"
#include "ui16_priv.h"

static int g_mouse_x = -1;
static int g_mouse_y = -1;
static int g_mouse_down = 0;
static int g_mouse_down_prev = 0;

void ui16_input(int mouse_x, int mouse_y, int mouse_down)
{
    g_mouse_x = mouse_x;
    g_mouse_y = mouse_y;
    g_mouse_down = mouse_down ? 1 : 0;
}

static int node_contains(const ui16_node_t *node, int x, int y)
{
    if (!node || x < 0 || y < 0) return 0;

    return
        x >= node->box_x && x < node->box_x + node->box_width &&
        y >= node->box_y && y < node->box_y + node->box_height
    ;
}

int ui16_hovered(const ui16_node_t *node)
{
    return node_contains(node, g_mouse_x, g_mouse_y);
}

int ui16_pressed(const ui16_node_t *node)
{
    return ui16_hovered(node) && g_mouse_down;
}

int ui16_clicked(const ui16_node_t *node)
{
    return ui16_hovered(node) && g_mouse_down && !g_mouse_down_prev;
}


int ui16__mouseX(void)
{
    return g_mouse_x;
}

int ui16__mouseY(void)
{
    return g_mouse_y;
}

int ui16__mouseDown(void)
{
    return g_mouse_down;
}


void ui16__inputEndFrame(void)
{
    g_mouse_down_prev = g_mouse_down;
}