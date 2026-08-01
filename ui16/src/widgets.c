/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 sulfurLabs
 *
 * PROJECT: s4
 * FILE: widgets.c
 *
 */

#include "ui16.h"
#include "ui16_priv.h"

ui16_node_t *ui16__containerBegin(ui16_style_t container_style)
{
    ui16_node_t *container_node = ui16__attachNode(UI16_NODE_CONTAINER, container_style, 0);
    ui16__setCurrentParent(container_node);

    return container_node;
}

void ui16__containerEnd(void)
{
    ui16_node_t *parent_node = ui16__currentParent();
    if (parent_node && parent_node->parent) ui16__setCurrentParent(parent_node->parent);
}

ui16_node_t *ui16__labelSimple(const char *label_text)
{
    return ui16__labelStyled(ui16__defaultStyle(), label_text);
}

ui16_node_t *ui16__labelStyled(ui16_style_t label_style, const char *label_text)
{
    return ui16__attachNode(UI16_NODE_LABEL, label_style, label_text);
}