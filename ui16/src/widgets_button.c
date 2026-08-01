/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 sulfurLabs
 *
 * PROJECT: s4
 * FILE: widgets_button.c
 *
 */

#include "ui16.h"
#include "ui16buttons.h"
#include "ui16_priv.h"

ui16_node_t *ui16__buttonSimple(const char *button_text)
{
    return ui16__buttonStyled(ui16__defaultStyle(), button_text);
}

ui16_node_t *ui16__buttonStyled(ui16_style_t button_style, const char *button_text)
{
    return ui16__attachNode(UI16_NODE_BUTTON, button_style, button_text);
}