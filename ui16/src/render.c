/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 sulfurLabs
 *
 * PROJECT: s4
 * FILE: render.c
 *
 */

#include "ui16.h"
#include "ui16_priv.h"

static void ui16__renderNode(ui16_node_t *node, const ui16_renderer_t *renderer)
{
    if (
        node->style.background != 0 &&
        renderer->drawRect
    ) renderer->drawRect(
        node->box_x,
        node->box_y,
        node->box_width,
        node->box_height,
        node->style.background,
        node->style.radius
    );

    if (
        (
            node->kind == UI16_NODE_LABEL ||
            node->kind == UI16_NODE_BUTTON
        )&&
        node->text  &&
        renderer->drawText
    ) renderer->drawText(
        node->box_x,
        node->box_y,
        node->text,
        0xFFFFFFFFu,
        ui16__resolveFont(node)
    );

    for (
        ui16_node_t *child_node = node->first_child;
        child_node;
        child_node = child_node->next_sibling
    ) ui16__renderNode(child_node, renderer);
}

void ui16__renderTree(const ui16_renderer_t *renderer)
{
    ui16_node_t *root_node = ui16__rootNode();

    if (!root_node || !renderer) return;

    ui16__renderNode(root_node, renderer);
}